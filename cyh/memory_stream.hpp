#pragma once
#include "typedef.hpp"
#include "exceptions.hpp"
#include "bstream.hpp"
#include <vector>
namespace cyh {
	namespace details {
		class mstream_impl final {
			std::vector<uint8> m_buffer;
			uint64 m_position{};
			uint64 m_previous_pos{};
			bool m_is_open{};			
			void* get_buffer_ensure_length(size_t length);
			void* get_buffer_max_remaining(size_t length, size_t* plen);
		public:
			void open();
			void close();
			uint64 tellg() const;
			uint64 tellp() const;
			bool is_open() const;
			void seekg(int64 pos, cyh::details::std_ios_flag_type flag);
			void seekp(int64 pos, cyh::details::std_ios_flag_type flag);
			int64 gcount() const;
			void read(void* _buf, size_t count);
			void write(const void* _data, size_t count);
			void flush();
			std::vector<uint8> get_bytes() const;
			~mstream_impl();
		};
	};
	class memory_stream : public cyh::iobstream_<cyh::details::mstream_impl> {
	public:
		memory_stream();
		void open();
		std::vector<uint8> get_bytes() const;
	};
};

