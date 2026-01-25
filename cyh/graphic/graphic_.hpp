#pragma once
#include <cyh/typedef.hpp>
#include <cyh/container/matrix.hpp>
#include <cyh/bstream.hpp>
namespace cyh::graphic 
{
	struct image_info
	{
		int width;
		int height;
		int channels;
		// for 8 byte alignment only
		int fill{};
		// static cstring, do not free!
		char* format;
	};
};

