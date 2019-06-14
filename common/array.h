#pragma once

#include <new>
#include <malloc.h>
#include <memory.h>

#include "globaldefs.h"

#if __GNUC__
template<size_t size, size_t alignment>
struct alignstruct(alignment) aligned_storage
{
	byte mData[size];
};
#else
template<size_t size, size_t alignment>
struct aligned_storage
{
};
template<size_t size>
struct alignstruct(1) aligned_storage<size, 1>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(2) aligned_storage<size, 2>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(4) aligned_storage<size, 4>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(8) aligned_storage<size, 8>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(16) aligned_storage<size, 16>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(32) aligned_storage<size, 32>
{
	byte mData[size];
};
template<size_t size>
struct alignstruct(64) aligned_storage<size, 64>
{
	byte mData[size];
};
#endif

template<typename type>
class ArrayAllocator_Heap
{
protected:
	type* __restrict mData;
	unsigned int mAllocated;

public:
	ArrayAllocator_Heap() :
		mData(nullptr),
		mAllocated(0)
	{
	}

	~ArrayAllocator_Heap()
	{
		ReAlloc(0, 0);
	}

	void ReAlloc(unsigned int count, unsigned int allocate)
	{
		check(allocate >= count);

		type* __restrict oldData = mData;
		if (allocate > 0)
		{
			mData = (type*)_aligned_malloc(sizeof(type) * allocate, __alignof(type));
			check(mData);
		}
		else
		{
			__assume(count == 0);
			mData = nullptr;
		}
		if (count > 0)
		{
			memcpy(mData, oldData, count * sizeof(type));
		}
		if (oldData)
		{
			_aligned_free(oldData);
		}
		mAllocated = allocate;
	}

	type* GetDataPointer() const
	{
		return mData;
	}

	type* GetDataPointer(unsigned int index) const
	{
		assert(index < mAllocated);
		return &mData[index];
	}

	unsigned int GetAllocatedSize() const
	{
		return mAllocated;
	}

	bool IsContiguousData() const
	{
		return true;
	}
};

template<typename type, unsigned int FixedDataSize>
class ArrayAllocator_Inline
{
protected:
	aligned_storage<FixedDataSize*sizeof(type), __alignof(type)> mAlignedData;

public:
	ArrayAllocator_Inline()
	{
	}

	~ArrayAllocator_Inline()
	{
	}

	void ReAlloc(unsigned int count, unsigned int allocate)
	{
		assert(allocate <= FixedDataSize);
	}

	type* GetDataPointer() const
	{
		return (type*)mAlignedData.mData;
	}

	type* GetDataPointer(unsigned int index) const
	{
		assert(index < FixedDataSize);
		return &((type*)mAlignedData.mData)[index];
	}

	unsigned int GetAllocatedSize() const
	{
		return FixedDataSize;
	}

	bool IsContiguousData() const
	{
		return true;
	}
};

template<typename type, unsigned int FixedDataSize>
class ArrayAllocator_Heap_OldPartialFixed
{
protected:
	aligned_storage<FixedDataSize*sizeof(type), __alignof(type)> mAlignedData;

	type* mData;
	unsigned int mAllocated;

public:
	ArrayAllocator_Heap_OldPartialFixed() :
		mAllocated(FixedDataSize)
	{
	}

	~ArrayAllocator_Heap_OldPartialFixed()
	{
		ReAlloc(0, 0);
	}

	void ReAlloc(unsigned int count, unsigned int allocate)
	{
		assert(allocate >= count);

		bool oldUseExternalData = mAllocated > FixedDataSize;
		bool newUseExternalData = allocate > FixedDataSize;
		if (oldUseExternalData || newUseExternalData)
		{
			type* oldData = oldUseExternalData ? mData : (type*)mAlignedData.mData;
			type* newData = newUseExternalData ? (type*)_aligned_malloc(sizeof(type) * allocate, __alignof(type)) : (type*)mAlignedData.mData;
			if (count)
			{
				memcpy(newData, oldData, count * sizeof(type));
			}
			if (oldUseExternalData)
			{
				_aligned_free(oldData);
			}
			mAllocated = newUseExternalData ? allocate : FixedDataSize;
			if (newUseExternalData)
			{
				mData = newData;
			}
		}
		// else do nothing, both before and after use the internal buffer
	}

	type* GetDataPointer() const
	{
		if (mAllocated > FixedDataSize)
		{
			return mData;
		}
		else
		{
			return (type*)mAlignedData.mData;
		}
	}

	type* GetDataPointer(unsigned int index) const
	{
		assert(index < mAllocated);
		return &GetDataPointer()[index];
	}

	unsigned int GetAllocatedSize() const
	{
		return mAllocated;
	}

	bool IsContiguousData() const
	{
		return true;
	}
};

template<typename type, unsigned int FixedDataSize>
class ArrayAllocator_Heap_PartialFixed
{
protected:
	aligned_storage<FixedDataSize*sizeof(type), __alignof(type)> mAlignedData;

	type* mData;
	unsigned int mAllocated;

public:
	ArrayAllocator_Heap_PartialFixed() :
		mData(nullptr),
		mAllocated(FixedDataSize)
	{
	}

	~ArrayAllocator_Heap_PartialFixed()
	{
		ReAlloc(0, 0);
	}

	void ReAlloc(unsigned int count, unsigned int allocate)
	{
		assert(allocate >= count);

		bool oldUseExternalData = mAllocated > FixedDataSize;
		bool newUseExternalData = allocate > FixedDataSize;
		if (oldUseExternalData || newUseExternalData)
		{
			type* oldData = mData;
			if (newUseExternalData)
			{
				mData = (type*)_aligned_malloc(sizeof(type) * (allocate - FixedDataSize), __alignof(type));
			}
			else
			{
				mData = nullptr;
			}
			if (count > FixedDataSize)
			{
				memcpy(mData, oldData, (count - FixedDataSize) * sizeof(type));
			}
			if (oldUseExternalData)
			{
				_aligned_free(oldData);
			}
			if (newUseExternalData)
			{
				mAllocated = allocate;
			}
			else
			{
				mAllocated = FixedDataSize;
			}
		}
		// else do nothing, both before and after use only the internal buffer
	}

	type* GetDataPointer() const
	{
		assert(IsContiguousData());
		return (type*)mAlignedData.mData;
	}

	type* GetDataPointer(unsigned int index) const
	{
		assert(index < mAllocated);
		if (index < FixedDataSize)
		{
			return &((type*)mAlignedData.mData)[index];
		}
		else
		{
			return &mData[index - FixedDataSize];
		}
	}

	unsigned int GetAllocatedSize() const
	{
		return mAllocated;
	}

	bool IsContiguousData() const
	{
		return mAllocated <= FixedDataSize;
	}
};

template<typename type>
class ArrayAllocator_PreAllocated
{
protected:
	type* mData;
	unsigned int mAllocated;

public:
	ArrayAllocator_PreAllocated() :
		mData(nullptr),
		mAllocated(0)
	{
	}

	ArrayAllocator_PreAllocated(type* pData, unsigned int Size) :
		mData(pData),
		mAllocated(Size)
	{
	}

	~ArrayAllocator_PreAllocated()
	{
	}

	void ReAlloc(unsigned int count, unsigned int allocate)
	{
		assert(allocate <= mAllocated);
	}

	type* GetDataPointer() const
	{
		return mData;
	}

	type* GetDataPointer(unsigned int index) const
	{
		assert(index < mAllocated);
		return &mData[index];
	}

	unsigned int GetAllocatedSize() const
	{
		return mAllocated;
	}

	bool IsContiguousData() const
	{
		return true;
	}
};

template<typename type, int growstep = 32, typename allocatortype = ArrayAllocator_Heap<type>>
class Array
{
	allocatortype mAllocator;
	unsigned int mSize;

public:
	Array(const allocatortype& allocator = allocatortype()) :
		mAllocator(allocator),
		mSize(0)
	{
	}

	Array(unsigned int size, unsigned int allocate = 0, const allocatortype& allocator = allocatortype()) :
		mAllocator(allocator),
		mSize(0)
	{
		if (allocate < size)
		{
			allocate = size;
		}
		mAllocator.ReAlloc(0, allocate);
		mSize = size;
		if (!std::is_trivially_default_constructible<type>::value)
		{
			for (unsigned int i = 0; i < size; i++)
			{
				new(GetDataPointer(i)) type;
			}
		}
	}
	
	// copy constructor
	Array(const Array<type, growstep, allocatortype>& rhs) :
		mAllocator(),
		mSize(0)
	{
		CopyFrom(rhs);
	}

	// "array of same type" constructor
	template<int rhsgrowstep, typename rhsallocatortype>
	Array(const Array<type, rhsgrowstep, rhsallocatortype>& rhs) :
		mAllocator(),
		mSize(0)
	{
		CopyFrom(rhs);
	}

	// copy assignment
	const Array<type, growstep, allocatortype>& operator =(const Array<type, growstep, allocatortype>& rhs)
	{
		CopyFrom(rhs);
		return *this;
	}

	// "array of same type" assignment
	template<int rhsgrowstep, typename rhsallocatortype>
	const Array<type, growstep, allocatortype>& operator =(const Array<type, rhsgrowstep, rhsallocatortype>& rhs)
	{
		CopyFrom(rhs);
		return *this;
	}

	~Array()
	{
		Empty();
		mAllocator.ReAlloc(0, 0);
	}

	// placeholder ranged-for functions, should return iterator types
	type* begin() const
	{
		return GetDataPointer();
	}
	type* end() const
	{
		return GetDataPointer() + mSize;
	}

	type& operator[] (unsigned int x) const
	{
		return *GetDataPointer(x);
	}

	type& operator() (unsigned int x) const
	{
		return *GetDataPointer(x);
	}

	unsigned int GetSize() const
	{
		return mSize;
	}

	type* GetDataPointer() const
	{
		assert(mAllocator.IsContiguousData());
		return mAllocator.GetDataPointer();
	}

	type* GetDataPointer(unsigned int x) const
	{
		assert(x < mSize);
		return mAllocator.GetDataPointer(x);
	}

	void Reserve(unsigned int count)
	{
		if (count > mAllocator.GetAllocatedSize())
		{
			mAllocator.ReAlloc(mSize, count);
		}
	}

	unsigned int Add(unsigned int count)
	{
		if (mSize + count > mAllocator.GetAllocatedSize())
		{
			unsigned int newAllocated = Max(mAllocator.GetAllocatedSize() + growstep, mSize + count);
			mAllocator.ReAlloc(mSize, newAllocated);
		}
		unsigned int newIndex = mSize;
		mSize += count;
		for (unsigned int i = newIndex; i < mSize; i++)
		{
			new(GetDataPointer(i)) type;
		}
		return newIndex;
	}

	void Remove(unsigned int x)
	{
		(*this)(x).~type();
		if (x < mSize-1)
		{
			assert(mAllocator.IsContiguousData());
			memmove(GetDataPointer(x), GetDataPointer(x+1), (mSize - x - 1) * sizeof(type));
		}
		mSize--;
	}

	template<typename rhstype>
	void RemoveItem(const rhstype& item)
	{
		unsigned int itemIndex = FindItemIndex(item);
		Remove(itemIndex);
	}

	unsigned int AddItem(const type& element)
	{
		if (mSize >= mAllocator.GetAllocatedSize())
		{
			unsigned int newAllocated = mSize + growstep;
			mAllocator.ReAlloc(mSize, newAllocated);
		}
		unsigned int newIndex = mSize;
		mSize++;
		new(GetDataPointer(newIndex)) type(element);
		return newIndex;
	}

	unsigned int AddItem(const type&& element)
	{
		if (mSize >= mAllocator.GetAllocatedSize())
		{
			unsigned int newAllocated = mSize + growstep;
			mAllocator.ReAlloc(mSize, newAllocated);
		}
		unsigned int newIndex = mSize;
		mSize++;
		new(GetDataPointer(newIndex)) type(std::move(element));
		return newIndex;
	}

	template<typename... Ts>
	unsigned int EmplaceItem(Ts&&... params)
	{
		if (mSize >= mAllocator.GetAllocatedSize())
		{
			unsigned int newAllocated = mSize + growstep;
			mAllocator.ReAlloc(mSize, newAllocated);
		}
		unsigned int newIndex = mSize;
		mSize++;
		new(GetDataPointer(newIndex)) type(std::forward<Ts>(params)...);
		return newIndex;
	}

	template<typename rhstype>
	unsigned int FindItemIndex(const rhstype& item)
	{
		for (unsigned int i = 0; i < mSize; i++)
		{
			if (*GetDataPointer(i) == item)
			{
				return i;
			}
		}
		return -1;
	}

	void Empty()
	{
		auto size = mSize;
		if (!std::is_trivially_destructible<type>::value)
		{
			for (unsigned int i = 0; i < size; i++)
			{
				GetDataPointer(i)->~type();
			}
		}
		mSize = 0;
	}

	template<int rhsgrowstep, typename rhsallocatortype>
	const Array<type, growstep, allocatortype>& operator +=(const Array<type, rhsgrowstep, rhsallocatortype>& rhs)
	{
		unsigned int newSize = mSize + rhs.GetSize();
		if (mAllocator.GetAllocatedSize() < newSize)
		{
			mAllocator.ReAlloc(mSize, newSize);
		}

		unsigned int newIndex = mSize;
		mSize = newSize;
		for (unsigned int i = newIndex, j = 0; i < newSize; i++, j++)
		{
			new(GetDataPointer(i)) type(rhs(j));
		}

		return *this;
	}

protected:
	template<int rhsgrowstep, typename rhsallocatortype>
	void CopyFrom(const Array<type, rhsgrowstep, rhsallocatortype>& rhs)
	{
		// destruct any extra
		if (mSize > rhs.mSize)
		{
			auto size = mSize;
			if (!std::is_trivially_destructible<type>::value)
			{
				for (unsigned int i = rhs.mSize; i < size; i++)
				{
					GetDataPointer(i)->~type();
				}
			}
			mSize = rhs.mSize;
		}
		// realloc larger if needed
		if (mAllocator.GetAllocatedSize() < rhs.mSize)
		{
			mAllocator.ReAlloc(mSize, rhs.mSize);
		}

		unsigned int i = 0;
		auto size = mSize;
		for (/*i*/; i < size; i++)
		{
			*GetDataPointer(i) = rhs(i);
		}
		mSize = rhs.mSize;
		size = rhs.mSize;
		for (/*i*/; i < size; i++)
		{
			new(GetDataPointer(i)) type(rhs(i));
		}
	}
};
