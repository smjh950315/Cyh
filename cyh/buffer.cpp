#include "buffer.hpp" 
#include <cyh/numeric.hpp>
namespace cyh {
	namespace details {
		static constexpr char hex_chars[] = "0123456789ABCDEFabcdef";
		template<class T>
		int to_hex_string_(void* strPtr, void* data, size_t length) {
			if (!strPtr || !data) {
				return -1;
			}
			if (length == 0) {
				return 0;
			}
			auto str = (std::basic_string<T>*)strPtr;
			auto strLen = str->length();

			size_t requiredLength = length * 2;
			if (strLen < requiredLength) {
				str->resize(requiredLength);
			}
			char* byteData = (char*)data;
			T* strData = str->data();
			for (size_t i = 0; i < length; ++i) {
				uint8 byte = static_cast<uint8>(byteData[i]);
				auto highNibble = hex_chars[(byte >> 4) & 0x0F];
				auto lowNibble = hex_chars[byte & 0x0F];
				strData[i * 2] = static_cast<T>(highNibble);
				strData[i * 2 + 1] = static_cast<T>(lowNibble);
			}
			return 0;
		}
		int to_hex_string(size_t charSize, void* outStrPtr, void* data, size_t length)
		{
			if (charSize == sizeof(char)) {
				return to_hex_string_<char>(outStrPtr, data, length);
			} else if (charSize == 2) {
				return to_hex_string_<char16_t>(outStrPtr, data, length);
			} else if (charSize == 4) {
				return to_hex_string_<char32_t>(outStrPtr, data, length);
			} else {
				return -1;
			}
		}
	};

	buffer::buffer(const buffer& other) {
		layout()->object_size = other.layout()->object_size;
		layout()->callbacks = other.layout()->callbacks;
		this->reallocate(other.size());
		callbacks()->objects_copy(this->vdata_begin(), other.vdata_begin(), other.size());
		layout()->count = other.size();
	}
	buffer::buffer(buffer&& other) noexcept {
		auto otherCallbacks = other.layout()->callbacks;
		MemoryHelper::Move<uintptr>(this->block, other.block, 5);
		other.layout()->callbacks = otherCallbacks;
	}
	buffer& buffer::operator=(const buffer& other) {
		// check if is not self-assignment
		if (this == &other) {
			return *this;
		} else {
			this->release();
			layout()->object_size = other.layout()->object_size;
			layout()->callbacks = other.layout()->callbacks;
			this->reallocate(other.size());
			callbacks()->objects_copy(this->vdata_begin(), other.vdata_begin(), other.size());
			layout()->count = other.size();
			return *this;
		}
	}
	buffer& buffer::operator=(buffer&& other) noexcept {
		if (this != &other) {
			auto otherCallbacks = other.layout()->callbacks;
			MemoryHelper::Move<uintptr>(this->block, other.block, 5);
			other.layout()->callbacks = otherCallbacks;
		}
		return *this;
	}
	buffer::~buffer() {
		this->release();
	}
	void* buffer::vdata_begin() const { return (void*)(this->block[0]); }
	void* buffer::vdata_end() const { return this->vdata_unchecked(this->size()); }
	void* buffer::vdata_unchecked(size_t index) const {
		byte* _data = (byte*)(this->vdata_begin());
		return _data + this->object_size() * index;
	}
	void* buffer::vdata(size_t index) const {
		byte* _data = (byte*)(this->vdata_begin());
		return _data + this->object_size() * index;
	}
	size_t buffer::object_size() const { return this->layout()->object_size; }
	size_t buffer::size() const { return this->layout()->count; }
	size_t buffer::capacity() const { return this->layout()->capacity; }
	size_t buffer::remain() const { return this->capacity() - this->size(); }
	void buffer::expand_capacity(size_t _expand) {
		size_t new_min_size = this->size() + _expand;
		if (this->capacity() >= new_min_size) { return; }
		this->reallocate(static_cast<size_t>(1.5 * static_cast<double>(new_min_size)));
	}
	void buffer::reallocate(size_t sz) {
		callbacks()->realloc(layout(), sz);
	}
	void buffer::resize(size_t sz) {
		auto thisSize = this->size();
		if (sz >= thisSize) {
			auto expand_sz = sz - thisSize;
			this->expand_capacity(expand_sz);
			callbacks()->objects_construct(vdata(thisSize), expand_sz);
		} else {
			callbacks()->objects_destruct((void*)(vdata(sz)), thisSize - sz);
		}
		layout()->count = sz;
	}
	void buffer::release() { this->reallocate(0); }
	void buffer::clear() {
		if (this->size() != 0) {
			callbacks()->objects_destruct(vdata_begin(), this->size());
			layout()->count = 0;
		}
	}

	__UNCHECKED__ buffer::block_data buffer::get_internal_buffer_t() {
		void* ptr = (void*)this->layout()->data;
		size_t len = this->layout()->count;
		size_t cap = this->layout()->capacity;
		this->layout()->data = 0;
		this->layout()->count = 0;
		this->layout()->capacity = 0;
		return block_data(ptr, len, cap);
	}
	__UNCHECKED__ void buffer::swap_internal_buffer(block_data* outter_block) {
		if (!outter_block) { return; }
		auto outter_ptr = outter_block->pointer;
		auto outter_len = outter_block->length;
		auto outter_cap = outter_block->capacity;
		outter_block->pointer = (void*)this->layout()->data;
		outter_block->length = this->layout()->count;
		outter_block->capacity = this->layout()->capacity;
		this->layout()->data = (uintptr)outter_ptr;
		this->layout()->count = outter_len;
		this->layout()->capacity = outter_cap;
	}
	std::vector<byte> buffer::bytes_from_hex_string(const char* hexstr)
	{
		if (!hexstr) {
			return {};
		}
		std::string_view hexview(hexstr);
		if (hexview.empty() || hexview.length() % 2 != 0) {
			return {};
		}
		std::vector<byte> result{};
		size_t byte_length = hexview.length() / 2;
		result.reserve(byte_length);
		for (size_t i = 0; i < byte_length; ++i) {
			std::string_view byte_str = hexview.substr(i * 2, 2);
			int64 byte_value = cyh::numeric::from_hex_string(byte_str);
			if (byte_value == -1) {
				return {};
			}
			result.push_back(static_cast<char>(byte_value & 0xFF));
		}
		return result;
	}
};

