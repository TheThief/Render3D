#include "globaldefs.h"
#include "renderer/renderer.h"
#include "array.h"

#include <functional>
#include <thread>
#include <type_traits>
#include <atomic>

template<size_t size, size_t alignment>
struct Renderer_Task
{
private:
	aligned_storage<size, alignment> store;
	void(*fn)(void*);         // used to invoke stored functor
	void(*destructor)(void*); // used to destruct stored functor

public:
	template<typename T>
	Renderer_Task(T&& thing)
	{
		static_assert(sizeof(T) <= size, "Increase size of Renderer_Task");
		static_assert(__alignof(T) <= alignment, "Increase alignment of Renderer_Task");
		new(store.mData) T(std::forward<T>(thing));
		fn = &invoke<T>;
		destructor = &destroy<T>;
	}

	Renderer_Task(Renderer_Task&) = delete;
	Renderer_Task(Renderer_Task&& other)
		: store(std::move(other.store)), fn(std::move(other.fn)), destructor(std::move(other.destructor))
	{
		other.fn = nullptr;
		other.destructor = nullptr;
	}

	~Renderer_Task()
	{
		if (destructor)
		{
			(*destructor)((void*)store.mData);
		}
	}

	void operator()() const
	{
		assert(fn);
		(*fn)((void*)store.mData);
	}

private:
	template<typename T>
	static void invoke(void* thing)
	{
		(*(const T*)thing)();
	}
	template<typename T>
	static void destroy(void* thing)
	{
		((T*)thing)->~T();
	}
};

template<typename T, int size>
struct Renderer_TaskQueue
{
	std::atomic<size_t> ReadIndex;
	std::atomic<size_t> WriteIndex;
	__declspec(align(64)) byte Data[size * sizeof(T)];

	Renderer_TaskQueue();
	Renderer_TaskQueue(Renderer_TaskQueue&) = delete;
	Renderer_TaskQueue(Renderer_TaskQueue&&) = delete;

	// Reading thread, owns ReadIndex so can do non-synchronised read
	// Does an aquire on WriteIndex to make contents of Data visible for reading
	T* Peek()
	{
		size_t CachedReadIndex = ReadIndex.load(std::memory_order::memory_order_relaxed);
		size_t CachedWriteIndex = WriteIndex.load(std::memory_order::memory_order_acquire);

		size_t RemainingData = CachedWriteIndex - CachedReadIndex;
		if (CachedReadIndex > CachedWriteIndex)
		{
			RemainingData += size;
		}

		if (RemainingData <= 0)
		{
			return nullptr;
		}

		return &((T*)Data)[CachedReadIndex];
	}

	// Reading thread, owns ReadIndex so can do non-synchronised read
	// Does a non-synchronised read on WriteIndex because we don't care about Data in Pop(void)
	// Writing to ReadIndex is done relaxed because there is no dependent data
	void Pop()
	{
		size_t CachedReadIndex = ReadIndex.load(std::memory_order::memory_order_relaxed);

#ifndef NDEBUG
		size_t CachedWriteIndex = WriteIndex.load(std::memory_order::memory_order_relaxed);
		assert(CachedReadIndex != CachedWriteIndex);
#endif // !NDEBUG

		ReadIndex.store((CachedReadIndex + 1) % size, std::memory_order::memory_order_relaxed);
	}

	// Reading thread, owns ReadIndex so can do non-synchronised read
	// Does an aquire on WriteIndex to make contents of Data visible for reading
	// Writing to ReadIndex is done relaxed because there is no dependent data
	bool Pop(T& out)
	{
		size_t CachedReadIndex = ReadIndex.load(std::memory_order::memory_order_relaxed);
		size_t CachedWriteIndex = WriteIndex.load(std::memory_order::memory_order_acquire);

		size_t RemainingData = CachedWriteIndex - CachedReadIndex;
		if (CachedReadIndex > CachedWriteIndex)
		{
			RemainingData += size;
		}

		if (RemainingData <= 0)
		{
			return false;
		}

		T* src = &((T*)Data)[CachedReadIndex];
		out = std::move(*src);
		src->~T();

		ReadIndex.store((CachedReadIndex + 1) % size, std::memory_order::memory_order_relaxed);

		return true;
	}

	// Writing thread, owns WriteIndex so can do non-synchronised read
	// Does a non-synchronised read on ReadIndex because there's no dependent data
	// Writing to WriteIndex is done with release to make Data visible to read thread
	bool Push(T&& in)
	{
		size_t CachedReadIndex = ReadIndex.load(std::memory_order::memory_order_relaxed);
		size_t CachedWriteIndex = WriteIndex.load(std::memory_order::memory_order_relaxed);

		size_t RemainingData = CachedReadIndex - CachedWriteIndex - 1;
		if (CachedReadIndex <= CachedWriteIndex)
		{
			RemainingData += size;
		}

		if (RemainingData <= 0)
		{
			return false;
		}

		void* dest = (void*)(&((T*)Data)[CachedWriteIndex]);
		new(dest) T(std::move(in));

		WriteIndex.store((CachedWriteIndex + 1) % size, std::memory_order::memory_order_release);

		return true;
	}
};

class RendererThread
{
private:
	typedef Renderer_Task<272, 16> Task;

	std::thread ThreadHandle;
	Renderer_TaskQueue<Task, 128> TaskQueue; // 128 entry task queue
	HANDLE TaskQueueWaitEvent;
	bool bTerminate; // owned by task thread, not for modification by other threads

public:
	BoundsRect ClipBounds;

public:
	RendererThread(int ThreadIndex, BoundsRect ClipBounds);
	~RendererThread();

	RendererThread(RendererThread&) = delete;
	RendererThread(RendererThread&&) = delete;

	void PushTask(Task&& task);

private:
	void Run();
};

template<typename Depth, typename... Pixels>
class Renderer
{
private:
	Array2d<Depth>& DepthBuffer;
	std::tuple<Array2d<Pixels>&...> Buffers;

	std::vector<std::unique_ptr<RendererThread>> WorkerThreads;

	bool needs_finalize = false;

public:
	//Renderer(unsigned int NumThreads, Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers);
	//Renderer(Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers);

	Renderer(unsigned int NumThreads, Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
		: DepthBuffer(DepthBuffer), Buffers(Buffers...), WorkerThreads()
	{
		WorkerThreads.reserve(NumThreads);

		for (unsigned int i : NumericRange<unsigned int>(0, NumThreads))
		{
			WorkerThreads.push_back(std::make_unique<RendererThread>(i, BoundsRect(0, DepthBuffer.GetWidth(), (DepthBuffer.GetHeight() * i) / NumThreads, (DepthBuffer.GetHeight() * (i + 1)) / NumThreads)));
		}
	}

	Renderer(Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
		: Renderer(std::max(2u, std::thread::hardware_concurrency()) - 1, DepthBuffer, Buffers...)
	{
	}

	~Renderer();

	void Finalize();

	// Depth-only
	template<typename DepthComparator, typename Globals, typename Vertex>
	void DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2);
	template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array>
	void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices);
	template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array>
	void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices);

	// Multi-buffer
	template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex>
	void DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2);
	template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array>
	void DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices);
	template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array>
	void DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices);

	// Fills
	void Fill(const Depth& depth, const Pixels&... pixels);

protected:
	template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex>
	static void RawDrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
	{
		return ::DrawTriangle<DepthComparator, PixelFunctor>(g, v0, v1, v2, ClipBounds, DepthBuffer, Buffers...);
	}
};

//////////////////////////////////////////////////////////////////////////

template<typename T, int size>
Renderer_TaskQueue<T, size>::Renderer_TaskQueue()
	: ReadIndex(0), WriteIndex(0)
{
}

//////////////////////////////////////////////////////////////////////////

RendererThread::RendererThread(int ThreadIndex, BoundsRect ClipBounds)
	: TaskQueue(), bTerminate(false), ClipBounds(ClipBounds)
{
	TaskQueueWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ThreadHandle = std::thread(&RendererThread::Run, this);
	SetThreadPriority(ThreadHandle.native_handle(), THREAD_PRIORITY_LOWEST);
	SetThreadIdealProcessor(ThreadHandle.native_handle(), ThreadIndex + 1);
}

RendererThread::~RendererThread()
{
	PushTask([this](){ bTerminate = true; });

	ThreadHandle.join();
	CloseHandle(TaskQueueWaitEvent);
}

void RendererThread::PushTask(Task&& task)
{
	while (!TaskQueue.Push(std::move(task)))
	{
		_mm_pause();
	}

	SetEvent(TaskQueueWaitEvent);
}

void RendererThread::Run()
{
	while (!bTerminate)
	{
		WaitForSingleObject(TaskQueueWaitEvent, INFINITE);

		// Pop and execute tasks
		while (const Task* task = TaskQueue.Peek())
		{
			(*task)();
			TaskQueue.Pop();
		}
	}
}

//////////////////////////////////////////////////////////////////////////

//template<typename Depth, typename... Pixels>
//Renderer<Depth, Pixels...>::Renderer(unsigned int NumThreads, Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
//	: WorkerThreads(), DepthBuffer(DepthBuffer), Buffers(Buffers)...
//{
//	WorkerThreads.Reserve(NumThreads);
//
//	for (unsigned int i : NumericRange<unsigned int>(0, NumThreads))
//	{
//		WorkerThreads.EmplaceItem(i, BoundsRect(0, DepthBuffer.GetWidth(), (DepthBuffer.GetHeight() * i) / NumThreads, (DepthBuffer.GetHeight() * (i + 1)) / NumThreads));
//	}
//}
//
//template<typename Depth, typename... Pixels>
//Renderer<Depth, Pixels...>::Renderer(Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
//	: Renderer(max(2, std::thread::hardware_concurrency()) - 1, DepthBuffer, Buffers...)
//{
//}

template<typename Depth, typename... Pixels>
Renderer<Depth, Pixels...>::~Renderer()
{
	// Worker threads are automatically destroyed
}

template<typename Depth, typename... Pixels>
void Renderer<Depth, Pixels...>::Finalize()
{
	if (needs_finalize)
	{
		HANDLE TaskQueueFinalizeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		for (auto& WorkerThread : WorkerThreads)
		{
			WorkerThread->PushTask([TaskQueueFinalizeEvent]() { SetEvent(TaskQueueFinalizeEvent); });
			WaitForSingleObject(TaskQueueFinalizeEvent, INFINITE);
		}

		CloseHandle(TaskQueueFinalizeEvent);
	}
}


// Depth-only

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename Globals, typename Vertex>
void Renderer<Depth, Pixels...>::DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	needs_finalize = true;

	for (int i : NumericRange<int>(0, (int)WorkerThreads.size()))
	{
		auto& WorkerThread = *WorkerThreads[i];
		BoundsRect ClipBounds = WorkerThread.ClipBounds;
		//WorkerThread.PushTask(std::bind(&::DrawTriangle<DepthComparator, Globals, Vertex, Depth>, g, v0, v1, v2, std::ref(DepthBuffer)));
		WorkerThread.PushTask([this, g, v0, v1, v2, ClipBounds](){ ::DrawTriangle<DepthComparator, Globals, Vertex, Depth>(g, v0, v1, v2, ClipBounds, DepthBuffer); });
	}
}

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array>
void Renderer<Depth, Pixels...>::DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices)
{
	for (unsigned int x = 0; x + 2 < indices.size(); x += 3)
	{
		DrawTriangle<DepthComparator>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]]);
	}
}

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename Globals, typename vertex_array, typename index_array>
void Renderer<Depth, Pixels...>::DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices)
{
	for (unsigned int x = 0; x + 2 < indices.size(); x++)
	{
		DrawTriangle<DepthComparator>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]]);
	}
}


// Multi-buffer
template<typename T, typename... U, typename... V, std::size_t... S>
auto Call_impl(T&& f, std::integer_sequence<size_t, S...>, const std::tuple<V...>& vs, U&&... us) -> decltype(f(std::forward<U>(us)..., std::get<S>(vs)...))
{
	return f(std::forward<U>(us)..., std::get<S>(vs)...);
}

template<typename T, typename... U, typename... V>
auto Call(T&& f, const std::tuple<V...>& vs, U&&... us) -> decltype(Call_impl(std::forward<T>(f), std::make_index_sequence<sizeof...(V)>(), vs, std::forward<U>(us)...))
{
	return Call_impl(std::forward<T>(f), std::make_index_sequence<sizeof...(V)>(), vs, std::forward<U>(us)...);
}

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex>
void Renderer<Depth, Pixels...>::DrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	needs_finalize = true;

	for (int i : NumericRange<int>(0, (int)WorkerThreads.size()))
	{
		auto& WorkerThread = *WorkerThreads[i];
		const BoundsRect& ClipBounds = WorkerThread.ClipBounds;
		//WorkerThread.PushTask(std::bind(&::DrawTriangle<DepthComparator, PixelFunctor, Globals, Vertex, Depth, Pixel>, g, v0, v1, v2, std::ref(DepthBuffer), std::ref(Buffer)));
		WorkerThread.PushTask([= /*, this*/](){ Call(&RawDrawTriangle<DepthComparator, PixelFunctor, Globals, Vertex>, Buffers, g, v0, v1, v2, ClipBounds, DepthBuffer); });
	}
}

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array>
void Renderer<Depth, Pixels...>::DrawIndexedTriList(const Globals& g, const vertex_array& vertices, const index_array& indices)
{
	for (int x = 0; x + 2 < indices.size(); x += 3)
	{
		DrawTriangle<DepthComparator, PixelFunctor>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]]);
	}
}

template<typename Depth, typename... Pixels>
template<typename DepthComparator, typename PixelFunctor, typename Globals, typename vertex_array, typename index_array>
void Renderer<Depth, Pixels...>::DrawIndexedTriStrip(const Globals& g, const vertex_array& vertices, const index_array& indices)
{
	for (int x = 0; x + 2 < indices.size(); x++)
	{
		DrawTriangle<DepthComparator, PixelFunctor>(g, vertices[indices[x + 0]], vertices[indices[x + 1]], vertices[indices[x + 2]]);
	}
}


// Fills

template<typename Depth, typename... Pixels>
void Renderer<Depth, Pixels...>::Fill(const Depth& depth, const Pixels&... pixels)
{
	needs_finalize = true;

	static_assert(sizeof...(Pixels) == 1, "Fill not yet variadic, only supports one pixel buffer");
	for (int i : NumericRange<int>(0, (int)WorkerThreads.size()))
	{
		auto& WorkerThread = *WorkerThreads[i];
		const BoundsRect& ClipBounds = WorkerThread.ClipBounds;
		const std::tuple<Pixels...> pixels_tuple(pixels...);
		WorkerThread.PushTask([= /*, this*/]() { Fill2d(DepthBuffer, depth, ClipBounds); Fill2d(std::get<0>(Buffers), std::get<0>(pixels_tuple), ClipBounds); });
	}
}

// protected:

//template<typename Depth, typename... Pixels>
//template<typename DepthComparator, typename PixelFunctor, typename Globals, typename Vertex>
//void Renderer<Depth, Pixels...>::RawDrawTriangle(const Globals& g, const Vertex& v0, const Vertex& v1, const Vertex& v2, const BoundsRect& ClipBounds, Array2d<Depth>& DepthBuffer, Array2d<Pixels>&... Buffers)
//{
//	return ::DrawTriangle(g, v0, v1, v2, ClipBounds, DepthBuffer, Buffers...);
//}
