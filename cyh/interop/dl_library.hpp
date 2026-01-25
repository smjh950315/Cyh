#pragma once
#include "interop_.hpp"
namespace cyh::interop
{
	class dl_library
	{
	public:
		static void* load_library(const char* path);
		static void free_library(void* hLib);
		static void* get_library_function(void* hLib, const char* func_name);
		template<class TFunc>
		static bool set_callback(TFunc& _pfunc, void* hLib, const char* func_name)
		{
			void* pfunc = get_library_function(hLib, func_name);
			if (pfunc == nullptr) return false;
			_pfunc = (TFunc)pfunc;
			return true;
		}
	};
};
