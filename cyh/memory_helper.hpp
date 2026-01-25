#pragma once
#include "typedef.hpp"
#include "exceptions.hpp"

namespace cyh {
	class MemoryHelper {
	private:
		using This = MemoryHelper;
		using m = MemoryHelper;
		static pvoid(*ms_unchecked_alloc)(size_t);
		static pvoid(*ms_unchecked_realloc)(pvoid, size_t);
		static void(*ms_unchecked_free)(pvoid);
		static void(*ms_unchecked_copy)(pvoid, pvoid, size_t);
		static void(*ms_unchecked_zero)(pvoid, size_t);
		template<class T>
		struct mapped_addr { byte bytes[sizeof(T)]; };
		template<class Val>
		static constexpr byte& get_lsb_byte_ref(Val& val, size_t index) requires(cyh::Environment::is_big_endian) {
			mapped_addr<Val>* addrMap = ((mapped_addr<Val>*)(&val));
			return addrMap->bytes[sizeof(Val) - index];
		}
		template<class Val>
		static constexpr byte& get_lsb_byte_ref(Val& val, size_t index) requires(cyh::Environment::is_little_endian) {
			mapped_addr<Val>* addrMap = ((mapped_addr<Val>*)(&val));
			return addrMap->bytes[index];
		}

		static inline size_t min_value(const size_t lhs, const size_t rhs) {
			return lhs < rhs ? lhs : rhs;
		}

		template<class T>
		static inline void copy_t(void* dst, void* src, size_t count) requires(std::is_class_v<std::decay_t<T>>) {
			if (count == 1) {
				*((T*)dst) = *((T*)src);
			} else {
				T* d = static_cast<T*>(dst);
				T* s = static_cast<T*>(src);
				for (size_t i = 0; i < count; ++i) {
					d[i] = s[i];
				}
			}
		}
		template<class T>
		static inline void copy_t(void* dst, void* src, size_t count) requires(!std::is_class_v<std::decay_t<T>>) {
			m::ms_unchecked_copy(dst, src, sizeof(T) * count);
		}
		template<class T>
		static inline void move_t(void* dst, void* src, size_t count) requires(std::is_class_v<std::decay_t<T>>) {
			if (count == 1) {
				*((T*)dst) = std::move(*((T*)src));
			} else {
				T* d = static_cast<T*>(dst);
				T* s = static_cast<T*>(src);
				for (size_t i = 0; i < count; ++i) {
					d[i] = std::move(s[i]);
				}
			}
		}
		template<class T>
		static inline void move_t(void* dst, void* src, size_t count) requires(!std::is_class_v<std::decay_t<T>>) {
			m::ms_unchecked_copy(dst, src, sizeof(T) * count);
			m::ms_unchecked_zero(src, sizeof(T) * count);
		}
		template <class T, class... Args>
		static inline void initialize_t(void* addr, Args &&...args) requires(std::is_class_v<std::decay_t<T>>) {
			new (addr) T{ std::forward<Args>(args)... };
		}
		template <class T, class... Args>
		static inline void initialize_t(void* addr, Args &&...args) requires(!std::is_class_v<std::decay_t<T>>) {
			*((T*)addr) = T{ std::forward<Args>(args)... };
			//memset(addr, 0, sizeof(T));
		}
		template <class T, class... Args>
		static inline void batch_initialize_t(void* addr, size_t count, Args &&...args) {
			T* ptr = (T*)addr;
			for (size_t i = 0; i < count; ++i) {
				initialize_t<T>(ptr + i, std::forward<Args>(args)...);
			}
		}
		template<class T>
		static inline void finalize_t(void* addr, size_t count) requires(std::is_class_v<std::decay_t<T>>) {
			if (count == 1) {
				((T*)addr)->~T();
				return;
			}
			T* p = static_cast<T*>(addr);
			for (size_t i = 0; i < count; ++i) {
				p[i].~T();
			}
		}
		template<class T>
		static inline void finalize_t(void* addr, size_t count) requires(!std::is_class_v<std::decay_t<T>>) {
			m::ms_unchecked_zero(addr, count);
		}
		template<class T>
		static inline void finalize_and_free_t(void* addr, size_t count) requires(std::is_class_v<std::decay_t<T>>) {
			This::finalize_t<T>(addr, count);
			This::Free(addr);
		}
		template<class T>
		static inline void finalize_and_free_t(void* addr, size_t count) requires(!std::is_class_v<std::decay_t<T>>) {
			This::Free(addr);
		}
	public:
		template<class Val>
		static constexpr byte& GetByteRefIndexedByLSB(Val& val, size_t index) {
			return get_lsb_byte_ref(val, index);
		}

		static constexpr void* Allocate(size_t byteSize) {
			if (byteSize == 0)
				return nullptr;
			try {
				return operator new(byteSize);
			} catch (...) {
				return nullptr;
			}
		}

		static constexpr void* Reallocate(void* ptr, size_t byteSize) {
			This::Free(ptr);
			if (byteSize == 0) {
				return nullptr;
			} else {
				return This::Allocate(byteSize);
			}
		}

		static constexpr void Free(void* ptr) {
			if (ptr != nullptr)
				operator delete(ptr);
		}

		template <class T>
		static constexpr void* AllocateArray(size_t count) {
			return This::Allocate(sizeof(T) * count);
		}

		template<class T>
		static constexpr void* ReallocateArray(void* ptr, size_t new_count, size_t old_count) {
			if (ptr == nullptr) {
				return This::AllocateArray<T>(new_count);
			}

			if (new_count == 0) {
				This::finalize_and_free_t<T>(ptr, old_count);
				return nullptr;
			}

			T* newPtr = static_cast<T*>(This::AllocateArray<T>(new_count));
			T* oldPtr = static_cast<T*>(ptr);
			size_t move_count = min_value(old_count, new_count);

			if constexpr (std::is_class_v<T>) {
				for (size_t i = 0; i < move_count; ++i) {
					This::initialize_t<T>(newPtr + i, std::move(*(oldPtr + i)));
				}
				This::finalize_t<T>(oldPtr, old_count);
			} else {
				m::ms_unchecked_copy((void*)newPtr, ptr, sizeof(T) * move_count);
			}

			This::Free(oldPtr);
			return newPtr;
		}

		template <class T, class... Args>
		static constexpr void Initialize(void* ptr, Args &&...args) requires(!std::is_abstract_v<T>) {
			if (ptr == nullptr) return;
			This::initialize_t<T>(ptr, std::forward<Args>(args)...);
		}

		template<class T>
		static constexpr void Finalize(void* ptr) {
			if (ptr == nullptr) return;
			This::finalize_t<T>(ptr, 1);
		}

		template<class T, class...Args>
		static constexpr void BatchInitialize(void* ptr, size_t count, Args&&...args)  requires(!std::is_abstract_v<T>) {
			if (ptr == nullptr || count == 0) return;
			This::batch_initialize_t<T>(ptr, count, std::forward<Args>(args)...);
		}

		template<class T>
		static constexpr void BatchFinalize(void* ptr, size_t count) {
			if (ptr == nullptr || count == 0) return;
			This::finalize_t<T>(ptr, count);
		}

		template <class T>
		static constexpr void Move(void* dst, void* src, size_t count) requires(std::is_move_assignable_v<T>) {
			if (!dst || !src || !count || dst == src)
				return;
			This::move_t<T>(dst, src, count);
		}

		template <class T>
		static constexpr void Copy(void* dst, void* src, size_t count) requires(std::is_copy_assignable_v<T>) {
			if (!dst || !src || !count || dst == src)
				return;
			This::copy_t<T>(dst, src, count);
		}

		template<class T, class... Args>
		static constexpr void* CreateObject(Args&&...args) {
			void* retPtr = Allocate(sizeof(T));
			if (!retPtr) {
				return nullptr;
			} else {
				try {
					This::initialize_t<T>(retPtr, std::forward<Args>(args)...);
					return retPtr;
				} catch (...) {
					This::Free(retPtr);
					return nullptr;
				}
			}
		}

		template <class T, class U, class... Args>
		static constexpr void* CreateAbstractBaseObject(void** ori_ptr, Args &&...args) requires(std::is_base_of_v<T, U>) {
			if (!ori_ptr) {
				return nullptr;
			} else {
				void* new_ptr = This::Allocate(sizeof(U));
				if (!new_ptr) {
					return nullptr;
				} else {
					This::initialize_t<U>(new_ptr, std::forward<Args>(args)...);
					T* ret_ptr_t = dynamic_cast<T*>(static_cast<U*>(new_ptr));
					if (!ret_ptr_t) {
						This::finalize_t<U>(new_ptr, 1);
						This::Free(new_ptr);
						return nullptr;
					}
					if (*ori_ptr) {
						This::finalize_t<T>(*ori_ptr, 1);
					}
					*ori_ptr = new_ptr;
					return (void*)ret_ptr_t;
				}
			}
		}

		template<class T>
		static constexpr void ReleaseAbstract(void* objPtr, void* originalPtr = nullptr) {
			if (!objPtr) return;
			This::finalize_t<T>(objPtr, 1);
			if (originalPtr) {
				This::Free(originalPtr);
			} else {
				This::Free(objPtr);
			}
		}
	};
};
