#pragma once
#include "memory_helper.hpp"
#include "reference.hpp"
#include <cyh/console.hpp>
namespace cyh {

	template<size_t _MaxLength>
	struct cstr_
	{
		char data[_MaxLength + 1]{};
		cstr_() {}
		cstr_(const char* _cstr) 
		{
			write_char_array(data, _cstr);
		}
		cstr_(const std::string& _str) : cstr_<_MaxLength>(_str.c_str()) {}
		template<size_t _L2>
		cstr_(const cstr_<_L2>& _other)
		{
			MemoryHelper::Copy<char>(data, _other.data, std::min(_MaxLength, _L2));
		}
		template<size_t _L2>
		cstr_(cstr_<_L2>&& _other) noexcept
		{
			MemoryHelper::Copy<char>(data, _other.data, std::min(_MaxLength, _L2));
		}
		template<size_t _L2>
		cstr_<_MaxLength>& operator=(const cstr_<_L2>& _other)
		{
			MemoryHelper::Copy<char>(data, _other.data, std::min(_MaxLength, _L2));
			return *this;
		}
		template<size_t _L2>
		cstr_<_MaxLength>& operator=(cstr_<_L2>&& _other) noexcept
		{
			MemoryHelper::Copy<char>(data, _other.data, std::min(_MaxLength, _L2));
			return *this;
		}
		operator std::string() const
		{
			return std::string((const char*)data, this->length());
		}
		size_t length() const { return xstrlen((const char*)data, _MaxLength); }
		size_t size() const { return this->length(); }
	};

	namespace details {
		struct buffer_layout;
		struct buffer_callbacks;
		struct buffer_layout {
			union {
				struct {
					uintptr data;
					uintptr count;
					uintptr capacity;
					uintptr object_size;
					buffer_callbacks* callbacks;
				};
				uintptr raw[5];
			};
		};
		struct buffer_callbacks {
			bool (*realloc)(buffer_layout*, size_t);
			void (*objects_move)(void* _from, void* _to, size_t count);
			void (*objects_copy)(void* _from, void* _to, size_t count);
			void (*objects_destruct)(void* ptr, size_t count);
			void (*objects_construct)(void* ptr, size_t count);
			void (*print_object)(void* ptr);
			template<class T>
			static buffer_callbacks get_callbacks() {
				auto cb = buffer_callbacks{};
				cb.realloc = [](buffer_layout* b, size_t s)
					{
						auto count = b->count;
						auto capacity = b->capacity;
						void* temp = (void*)b->data;
						temp = MemoryHelper::ReallocateArray<T>(temp, s, count);
						if (!temp) {
							b->data = 0;
							b->count = 0;
							b->capacity = 0;
							return s == 0;
						} else {
							b->data = (uintptr)temp;
							b->count = count < s ? count : s;
							b->capacity = s;
							return true;
						}
					};
				cb.objects_construct = [](void* dst, size_t count)
					{
						MemoryHelper::BatchInitialize<T>(dst, count);
					};
				cb.objects_copy = [](void* dst, void* src, size_t count)
					{
						MemoryHelper::Copy<T>(dst, src, count);
					};
				cb.objects_move = [](void* dst, void* src, size_t count)
					{
						MemoryHelper::Move<T>(dst, src, count);
					};
				cb.objects_destruct = [](void* ptr, size_t count)
					{
						MemoryHelper::BatchFinalize<T>(ptr, count);
					};
				cb.print_object = [](void* ptr)
					{
						T& obj = *(T*)ptr;
						cyh::console::print(obj);
					};
				return cb;
			}
			template<class T>
			struct static_callbacks { static buffer_callbacks callbacks; };
		};
		int to_hex_string(size_t charSize, void* outStrPtr, void* data, size_t length);
	};

	struct buffer {
		struct block_data {
			void* pointer{};
			size_t length{};
			size_t capacity{};
			constexpr block_data() {}
			constexpr block_data(void* ptr, size_t len, size_t cap) :pointer(ptr), length(len), capacity(cap) {}
		};
		using buffer_layout = cyh::details::buffer_layout;
		using buffer_callbacks = cyh::details::buffer_callbacks;

		uintptr block[5]{};
		buffer_layout* layout() const {
			return (buffer_layout*)(this->block);
		}
		buffer_callbacks* callbacks() const {
			return layout()->callbacks;
		}
		template<class T>
		bool type_check() const {
			return this->callbacks() == &buffer_callbacks::static_callbacks<T>::callbacks;
		}

		template<class T>
		buffer(T* p = nullptr) {
			layout()->callbacks = &buffer_callbacks::static_callbacks<T>::callbacks;
			layout()->object_size = sizeof(T);
		}
		buffer(const buffer& other);
		buffer(buffer&& other) noexcept;
		buffer& operator=(const buffer& other);
		buffer& operator=(buffer&& other) noexcept;
		virtual ~buffer();
		void* vdata_begin() const;
		void* vdata_end() const;
		void* vdata_unchecked(size_t index) const;
		void* vdata(size_t index) const;
		size_t object_size() const;
		size_t size() const;
		size_t capacity() const;
		size_t remain() const;
		void expand_capacity(size_t _expand);
		void reallocate(size_t sz);
		void resize(size_t sz);
		void release();
		void clear();
		template<class T>
		constexpr T* data_t(size_t index) const {
			if (!this->type_check<T>()) {
				throw std::invalid_argument("Type mismatch in buffer::data_t");
			}
			return (T*)vdata(index);
		}
		template<class T>
		constexpr void push_back_t(const std::decay_t<T>& _data) {
			if (!this->type_check<T>()) {
				throw std::invalid_argument("Type mismatch in buffer::push_back_t");
			}
			if (!this->remain()) { this->expand_capacity(1); }
			void* vptr_end = this->vdata_end();
			MemoryHelper::Initialize<T>(vptr_end, _data);
			layout()->count += 1;
		}
		template<class T>
		constexpr void push_back_t(std::decay_t<T>&& _data) {
			if (!this->type_check<T>()) {
				throw std::invalid_argument("Type mismatch in buffer::push_back_t");
			}
			if (!this->remain()) { this->expand_capacity(1); }
			void* vptr_end = this->vdata_end();
			MemoryHelper::Initialize<T>(vptr_end, std::move(_data));
			layout()->count += 1;
		}
		template<class T>
		constexpr void push_array_back_t(const std::decay_t<T>* _data, size_t length) requires(!std::is_class_v<T>) {
			if (!this->type_check<T>()) {
				throw std::invalid_argument("Type mismatch in buffer::push_array_back_t");
			}
			if (!this->remain()) { this->expand_capacity(length); }
			void* vptr_end = this->vdata_end();
			MemoryHelper::Copy<T>(vptr_end, (void*)_data, length);
			layout()->count += length;
		}
		template<class T>
		constexpr ref<T> pop_back_t() {
			if (!this->type_check<T>()) {
				return {};
			}
			size_t obj_count = this->size();
			if (!obj_count) { return ref<T>(); }
			T* vptr_last = (T*)this->vdata_unchecked(obj_count - 1);
			ref<T> _ref{ std::move(*vptr_last) };
			MemoryHelper::Finalize<T>(vptr_last);
			layout()->count -= 1;
			return _ref;
		}
		template<class T>
		constexpr void just_pop_back_t() {
			if (!this->type_check<T>()) {
				return;
			}
			size_t obj_count = this->size();
			if (!obj_count) { return; }
			void* vptr_last = this->vdata_unchecked(obj_count - 1);
			MemoryHelper::Finalize<T>(vptr_last);
			layout()->count -= 1;
		}
		template<class T>
		__UNCHECKED__ constexpr void replace_internal_buffer_t(T* new_buffer, size_t new_length, size_t new_capacity) {
			this->release();
			this->layout()->data = (uintptr)new_buffer;
			this->layout()->count = new_length;
			this->layout()->capacity = new_capacity;
		}
		__UNCHECKED__ block_data get_internal_buffer_t();
		__UNCHECKED__ void swap_internal_buffer(block_data* outter_block);

		static std::vector<byte> bytes_from_hex_string(const char* hexstr);
		template<class T = char>
		static std::basic_string<T> hex_string_from_bytes(void* data, size_t length) {
			std::basic_string<T> result;
			details::to_hex_string(sizeof(T), (void*)&result, data, length);
			return result;
		}
	};

	namespace details {
		template<class T>
		buffer_callbacks buffer_callbacks::static_callbacks<T>::callbacks = buffer_callbacks::get_callbacks<T>();
	};
};
