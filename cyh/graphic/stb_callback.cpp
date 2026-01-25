#include "stb_callback.hpp"
#include <cyh/graphic/image.hpp>
#include <cyh/memory_helper.hpp>
using IMFORMAT = cyh::graphic::image::FORMAT;
namespace cyh::graphic {
	struct stb_override
	{
		static constexpr void* Override_PrimitiveMalloc(size_t sz) { return MemoryHelper::AllocateArray<byte>(sz); }
		static constexpr void* Override_PrimitiveRealloc(void* old, size_t sz) { return MemoryHelper::ReallocateArray<byte>(old, sz, 0); }
		static constexpr void Override_PrimitiveFree(void* old) { MemoryHelper::Free(old); }
	};
	struct stb_io
	{
		static int read_binary(void* _src, char* data, int size)
		{
			binary_data* src = (binary_data*)_src;
			int remaining = static_cast<int>(src->length) - static_cast<int>(src->current);
			if (remaining <= 0) return 0;
			int max_read = remaining < size ? remaining : size;
			MemoryHelper::Copy<byte>(data, (byte*)src->data + src->current, max_read);
			return max_read;
		}
		static void skip_binary(void* _src, int n) {
			binary_data* src = (binary_data*)_src;
			if (n < 0) {
				src->current += (-n);
			} else {
				src->current += n;
			}
		}
		static int check_binary_eof(void* _src) {
			binary_data* src = (binary_data*)_src;
			return src->length == src->current ? 1 : 0;
		}
		static void write_binary_data(void* _dst, void* data, int size) {
			binary_data* dst = (binary_data*)_dst;
			int totlen = dst->current + size;
			if (totlen > dst->length)
			{
				auto newPtr = MemoryHelper::ReallocateArray<byte>(dst->data, totlen, dst->length);
				if (newPtr == nullptr) return;
				dst->data = newPtr;
				dst->length = totlen;
			}
			MemoryHelper::Copy<byte>((byte*)dst->data + dst->current, data, size);
			dst->current += size;
		}
		static void free_stb_io_allocated_data(void* _src)
		{
			MemoryHelper::Free(_src);
		}

		static void write_stream_data(void* _dst, void* data, int size) 
		{
			obstream* dst = (obstream*)_dst;
			dst->write((byte*)data, size);
		}
	};
};
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MALLOC(sz) cyh::graphic::stb_override::Override_PrimitiveMalloc(sz);
#define STBI_REALLOC(p,newsz) cyh::graphic::stb_override::Override_PrimitiveRealloc(p,newsz);
#define STBI_REALLOC_SIZED(p,oldsz,newsz) cyh::MemoryHelper::ReallocateArray<cyh::byte>(p,newsz,oldsz);
#define STBI_FREE(p) cyh::graphic::stb_override::Override_PrimitiveFree(p);
#include <stb_image.h>
#include <stb_image_write.h>
#define __CONSTEXPR_CSTR(x) static constexpr const char* x = #x
namespace cyh::graphic {
	struct image_format_cstring {
		__CONSTEXPR_CSTR(jpeg);
		__CONSTEXPR_CSTR(png);
		__CONSTEXPR_CSTR(gif);
		__CONSTEXPR_CSTR(bmp);
		__CONSTEXPR_CSTR(psd);
		__CONSTEXPR_CSTR(pic);
		__CONSTEXPR_CSTR(pnm);
		__CONSTEXPR_CSTR(hdr);
		__CONSTEXPR_CSTR(tga);
	};
};
#define TEST_AND_WRITE_FMTBUFFER(fmt, fmtstr_addr_ptr)                          \
	if (stbi__##fmt##_info(stb_ctx_ptr, px, py, pc)) {                          \
		*fmtstr_addr_ptr = (char *)(cyh::graphic::image_format_cstring::fmt); \
		return true;                                                            \
	}
namespace cyh::graphic {
	static constexpr bool hasData(image_data* data)
	{
		return data != nullptr && data->data != nullptr && data->width > 0 && data->height > 0 && data->channels > 0;
	}
	static constexpr bool hasData(binary_data* data)
	{
		return data != nullptr && data->data != nullptr && data->length > 0;
	}

	using stb_write_to_fnuc = int(*)(stbi_write_func* func, void* context, int x, int y, int comp, const void* data, int additional);
	using stb_write_to_file = int(*)(const char* filename, int x, int y, int comp, const void* data, int quality);

	static int my_stbi_write_bmp_to_func(stbi_write_func* func, void* context, int x, int y, int comp, const void* data, int additional)
	{
		return stbi_write_bmp_to_func(func, context, x, y, comp, data);
	}
	static int my_stbi_write_bmp_to_file(const char* filename, int x, int y, int comp, const void* data, int quality)
	{
		return stbi_write_bmp(filename, x, y, comp, data);
	}

	static constexpr stb_write_to_fnuc stb_write_funcs[] =
	{
		my_stbi_write_bmp_to_func,
		stbi_write_jpg_to_func,
		stbi_write_png_to_func,
	};

	static constexpr stb_write_to_file stb_write_files[] =
	{
		my_stbi_write_bmp_to_file,
		stbi_write_jpg,
		stbi_write_png,
	};

	static bool _impl_read_image_info(image_info* image_info_ptr, binary_data* data)
	{
		if (image_info_ptr == nullptr || !hasData(data)) return false;

		int* px = &image_info_ptr->width;
		int* py = &image_info_ptr->height;
		int* pc = &image_info_ptr->channels;
		char** pformat = &image_info_ptr->format;

		stbi__context s;
		stbi__start_mem(&s, (stbi_uc*)data->data, data->length);
		stbi__context* stb_ctx_ptr = &s;
		TEST_AND_WRITE_FMTBUFFER(jpeg, pformat);
		TEST_AND_WRITE_FMTBUFFER(png, pformat);
		TEST_AND_WRITE_FMTBUFFER(gif, pformat);
		TEST_AND_WRITE_FMTBUFFER(bmp, pformat);
		TEST_AND_WRITE_FMTBUFFER(psd, pformat);
		TEST_AND_WRITE_FMTBUFFER(pic, pformat);
		TEST_AND_WRITE_FMTBUFFER(pnm, pformat);
		TEST_AND_WRITE_FMTBUFFER(hdr, pformat);
		TEST_AND_WRITE_FMTBUFFER(tga, pformat);
		return false;
	}

	static bool _impl_stb_callback_load_image(image_data* output, binary_data* data, int req_comp)
	{
		if (!hasData(data)) return false;
		output->data = stbi_load_from_memory((const stbi_uc*)data->data, data->length, &output->width, &output->height, &output->channels, req_comp);
		if (output->channels != 0 && output->channels != req_comp && (req_comp == 3 || req_comp == 4)) {
			output->channels = req_comp;
		}
		return hasData(output);
	}
	static bool _impl_stb_callback_load_image_file(image_data* output, const char* path, int req_comp)
	{
		output->data = stbi_load(path, &output->width, &output->height, &output->channels, req_comp);
		if (output->channels != 0 && output->channels != req_comp && (req_comp == 3 || req_comp == 4)) {
			output->channels = req_comp;
		}
		return hasData(output);
	}
	static void _impl_free_binary_data(binary_data* data)
	{
		if (data == nullptr) { return; }
		if (data->data != nullptr)
		{
			stb_io::free_stb_io_allocated_data(data->data);
			data->data = nullptr;
		}
		data->current = 0;
		data->length = 0;
	}
	static bool _impl_stb_callback_write_image(binary_data* output, image_data* data, int encode, int additional)
	{
		if (!hasData(data)) return false;
		IMFORMAT enc = (IMFORMAT)encode;
		switch (enc)
		{
			case IMFORMAT::BMP:
				return stbi_write_bmp_to_func(stb_io::write_binary_data, output, data->width, data->height, data->channels, data->data);
			case IMFORMAT::JPG:
				return stbi_write_jpg_to_func(stb_io::write_binary_data, output, data->width, data->height, data->channels, data->data, additional);
			case IMFORMAT::PNG:
				return stbi_write_png_to_func(stb_io::write_binary_data, output, data->width, data->height, data->channels, data->data, data->width * data->channels);
			default:
				return false;
		}
	}
	static bool _impl_stb_callback_write_image_file(image_data* data, const char* path, int encode, int additional)
	{
		if (!hasData(data)) return false;
		IMFORMAT enc = (IMFORMAT)encode;
		switch (enc)
		{
			case IMFORMAT::BMP:
				return stbi_write_bmp(path, data->width, data->height, data->channels, data->data);
			case IMFORMAT::JPG:
				return stbi_write_jpg(path, data->width, data->height, data->channels, data->data, additional);
			case IMFORMAT::PNG:
				return stbi_write_png(path, data->width, data->height, data->channels, data->data, data->width * data->channels);
			default:
				return false;
		}
	}

	static bool _impl_stb_callback_write_image_to_stream(obstream& _obstream, image_data* data, int encode, int additional)
	{
		if (!hasData(data)) return false;
		IMFORMAT enc = (IMFORMAT)encode;
		switch (enc)
		{
			case IMFORMAT::BMP:
				return stbi_write_bmp_to_func(stb_io::write_stream_data, &_obstream, data->width, data->height, data->channels, data->data);
			case IMFORMAT::JPG:
				return stbi_write_jpg_to_func(stb_io::write_stream_data, &_obstream, data->width, data->height, data->channels, data->data, additional);
			case IMFORMAT::PNG:
				return stbi_write_png_to_func(stb_io::write_stream_data, &_obstream, data->width, data->height, data->channels, data->data, data->width * data->channels);
			default:
				return false;
		}
	}

	SET_CALLBACK_API(stb_callback::read_image_info, _impl_read_image_info);
	SET_CALLBACK_API(stb_callback::load_image, _impl_stb_callback_load_image);
	SET_CALLBACK_API(stb_callback::load_image_file, _impl_stb_callback_load_image_file);
	SET_CALLBACK_API(stb_callback::free_binary_data, _impl_free_binary_data);
	SET_CALLBACK_API(stb_callback::write_image, _impl_stb_callback_write_image);
	SET_CALLBACK_API(stb_callback::write_image_file, _impl_stb_callback_write_image_file);
	SET_CALLBACK_API(stb_callback::write_image_to_stream, _impl_stb_callback_write_image_to_stream);
};

