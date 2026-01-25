#include "memory_helper.hpp"
#include <cstring>
namespace cyh {
	pvoid __impl__alloc(size_t _byteSize) { return malloc(_byteSize); }
	pvoid __impl__realloc(pvoid _ptr, size_t _byteSize) { return realloc(_ptr, _byteSize); }
	void __impl__free(pvoid _ptr) { free(_ptr); }
	void __impl__copy(pvoid _dst, pvoid _src, size_t _byteSize) { memcpy(_dst, _src, _byteSize); }
	void __impl__zero(pvoid _dst, size_t _byteSize) { memset(_dst, 0, _byteSize); }
	SET_CALLBACK_API(cyh::MemoryHelper::ms_unchecked_alloc, __impl__alloc);
	SET_CALLBACK_API(cyh::MemoryHelper::ms_unchecked_realloc, __impl__realloc);
	SET_CALLBACK_API(cyh::MemoryHelper::ms_unchecked_free, __impl__free);
	SET_CALLBACK_API(cyh::MemoryHelper::ms_unchecked_copy, __impl__copy);
	SET_CALLBACK_API(cyh::MemoryHelper::ms_unchecked_zero, __impl__zero);
}
