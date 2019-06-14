#pragma once

#include <iterator>

template<typename Func, typename Buffer1, typename... BufferNs>
void Process1d(Func&& func, Buffer1&& buffer1, BufferNs&&... bufferNs)
{
	const auto size = std::size(buffer1);
	for (unsigned int x = 0; x < size; x++)
	{
		func(buffer1[x], bufferNs[x]...);
	}
}

// Created Output buffer
template<typename Output, typename Func, typename Buffer1, typename... BufferNs>
Output ProcessToNew1d(Func&& func, Buffer1&& buffer1, BufferNs&&... bufferNs)
{
	// todo - uninitialized constructor + placement new + exception safety
	const auto size = std::size(buffer1);
	Output output(size);
	for (unsigned int x = 0; x < size; x++)
	{
		output[x] = func(buffer1[x], bufferNs[x]...);
	}
	return output;
}

//////////////////////////////////////////////////////////////////////////


template<typename Buffer, typename Pixel>
void Fill1d(Buffer& buffer, const Pixel& p)
{
	const auto size = std::size(buffer);
	for (unsigned int x = 0; x < size; x++)
	{
		buffer[x] = p;
	}
}
