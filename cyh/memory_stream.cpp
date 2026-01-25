#include "memory_stream.hpp"
namespace cyh::details {
	static uint64 get_abs_pos(uint64 length, int64 pos, cyh::details::std_ios_flag_type flag) {
		int64 s_pos{};
		int64 slength = static_cast<int64>(length);
		if (flag == std::ios::end) {
			s_pos = slength + pos;
		} else {
			s_pos = pos;
		}

		if (s_pos < 0)
			return 0;

		uint64 u_pos = static_cast<uint64>(s_pos);

		if (u_pos > length)
			return length;
		return u_pos;
	}
	void* mstream_impl::get_buffer_ensure_length(size_t length) {
		auto remaining = this->m_buffer.size() - this->m_position;
		if (remaining < length) {
			this->m_buffer.resize(this->m_position + length);
		}
		return (void*)(this->m_buffer.data() + this->m_position);
	}
	void* mstream_impl::get_buffer_max_remaining(size_t length, size_t* plen) {
		auto remaining = this->m_buffer.size() - this->m_position;
		if (remaining < length)
			*plen = remaining;
		else
			*plen = length;
		return (void*)(this->m_buffer.data() + this->m_position);
	}
	void mstream_impl::open() {
		this->m_is_open = true;
	}
	void mstream_impl::close() {
		this->m_is_open = false;
	}
	uint64 mstream_impl::tellg() const {
		return this->m_position;
	}
	uint64 mstream_impl::tellp() const {
		return this->m_position;
	}
	bool mstream_impl::is_open() const {
		return this->m_is_open;
	}
	void mstream_impl::seekg(int64 pos, cyh::details::std_ios_flag_type flag) {
		if (this->is_open())
			this->m_position = get_abs_pos(this->m_buffer.size(), pos, flag);
	}
	void mstream_impl::seekp(int64 pos, cyh::details::std_ios_flag_type flag) {
		if (this->is_open())
			this->m_position = get_abs_pos(this->m_buffer.size(), pos, flag);
	}
	int64 mstream_impl::gcount() const {
		return this->m_position - this->m_previous_pos;
	}
	void mstream_impl::read(void* _buf, size_t count) {
		this->m_previous_pos = this->m_position;
		size_t blen{};
		MemoryHelper::Copy<char>(_buf, get_buffer_max_remaining(count, &blen), count);
		this->m_position += blen;
	}
	void mstream_impl::write(const void* _data, size_t count) {
		this->m_previous_pos = this->m_position;
		MemoryHelper::Copy<char>(get_buffer_ensure_length(count), (void*)_data, count);
		this->m_position += count;
	}
	void mstream_impl::flush() {}
	std::vector<uint8> mstream_impl::get_bytes() const {
		return this->m_buffer;
	}
	mstream_impl::~mstream_impl() {}
};
namespace cyh {
	memory_stream::memory_stream() { this->open_stream(); }
	void memory_stream::open() {
		if (!this->is_open())
			this->open_stream();
	}
	std::vector<uint8> memory_stream::get_bytes() const {
		if(this->m_stream)
			return this->m_stream->get_bytes();
		return std::vector<uint8>{};
	}
}
