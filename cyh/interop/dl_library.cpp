#include "dl_library.hpp"
#ifdef __WINDOWS__
#include <windows.h>
#else
#include <dlfcn.h>
#endif // __WINDOWS__

namespace cyh::interop
{
	void* dl_library::load_library(const char* path)
	{
#ifdef __WINDOWS__
		return LoadLibraryA(path);
#else
		return dlopen(path, RTLD_LAZY);
#endif // __WINDOWS__
	}
	void dl_library::free_library(void* hLib)
	{
#ifdef __WINDOWS__
		FreeLibrary((HMODULE)hLib);
#else
		dlclose(hLib);
#endif // __WINDOWS__
	}
	void* dl_library::get_library_function(void* hLib, const char* func_name)
	{
		if (!hLib || xstrlen(func_name) == 0) return nullptr;
#ifdef __WINDOWS__
		return GetProcAddress((HMODULE)hLib, func_name);
#else
		return dlsym(hLib, func_name);
#endif // __WINDOWS__
	}
};

