#pragma once
#include <cyh/graphic/graphic_.hpp>
namespace cyh::graphic {

	struct image_data
	{
		void* data;
		int width;
		int height;
		int channels;
	};
	struct binary_data
	{
		void* data;
		int current;
		int length;
	};

	struct stb_callback
	{
		static bool (*read_image_info)(image_info* image_info_ptr, binary_data* data);
		static bool (*load_image)(image_data* output, binary_data* data, int req_comp);
		static bool (*load_image_file)(image_data* output, const char* path, int req_comp);
		static void (*free_binary_data)(binary_data* data);
		static bool (*write_image)(binary_data* output, image_data* data, int format, int additional);
		static bool (*write_image_file)(image_data* data, const char* path, int encode, int additional);
		static bool (*write_image_to_stream)(obstream& _obstream, image_data* data, int format, int additional);
	};
};
