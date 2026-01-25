#pragma once
#include <cyh/typedef.hpp>
#include <cyh/reference.hpp>
#include <iostream>
namespace cyh {
	namespace details {
		class invalid_stream {
		public:
			void seekp(size_t pos) {}
			void seekg(size_t pos) {}
			void flush() {}
			void close() {}
			bool is_open() const { return false; }
			size_t tellp() const { return 0; }
			size_t tellg() const { return 0; }
		};

		template<class T>
		struct is_istream {
			template<class X>
			static auto test(X x) -> decltype(std::declval<X>().tellg(), std::declval<X>().seekg(0, std::ios::beg), std::true_type{});
			static auto test(...) -> std::false_type;
			static constexpr bool value = decltype(test(std::declval<T>()))::value;
		};
		template<class T>
		struct is_ostream {
			template<class X>
			static auto test(X x) -> decltype(std::declval<X>().tellp(), std::declval<X>().seekp(0, std::ios::beg), std::true_type{});
			static auto test(...) -> std::false_type;
			static constexpr bool value = decltype(test(std::declval<T>()))::value;
		};

		template<class T>
		using make_istream_t = typename std::conditional_t<is_istream<T>::value, T, invalid_stream>;
		template<class T>
		using make_ostream_t = typename std::conditional_t<is_ostream<T>::value, T, invalid_stream>;

		template<class T>
		constexpr bool is_istream_v = !std::is_same_v<cyh::details::make_istream_t<T>, cyh::details::invalid_stream>;
		template<class T>
		constexpr bool is_ostream_v = !std::is_same_v<cyh::details::make_ostream_t<T>, cyh::details::invalid_stream>;
		using std_ios_flag_type = decltype(std::ios::beg);
		class stream_operator {
		public:
			virtual bool is_open(void* ptr) const { return false; }
			virtual void close(void* ptr) {}

			virtual size_t tell(void* ptr) const { return 0; }
			virtual void seek(void* ptr, int64 pos, std_ios_flag_type _from) {}

			virtual bool can_read(void* ptr) const { return false; }
			virtual uint64 read(void* ptr, void* _buf, uint64 count) const { return 0; }
			virtual bool can_write(void* ptr) const { return false; }
			virtual uint64 write(void* ptr, const void* _data, uint64 count) { return 0; }

			virtual void flush(void* ptr) {}
			virtual ~stream_operator() {}
		};
		template<class T>
		class stream_operator_ : public stream_operator {
			using actual_type = make_istream_t<T>;
		public:
			bool is_open(void* ptr) const override {
				if (!ptr) return false;
				return ((actual_type*)ptr)->is_open();
			}
			void close(void* ptr) override {
				if (!this->is_open(ptr)) return;
				((actual_type*)ptr)->close();
			}
		};
		template<class T>
		class istream_operator : public stream_operator_<T> {
			using actual_type = make_istream_t<T>;
		public:
			istream_operator() {
				static_assert(!std::is_same_v<actual_type, invalid_stream>);
			}

			size_t tell(void* ptr) const override {
				if (!this->is_open(ptr)) return 0;
				return static_cast<size_t>(((actual_type*)ptr)->tellg());
			}
			void seek(void* ptr, int64 pos, std_ios_flag_type _from) override {
				if (!this->is_open(ptr)) return;
				((actual_type*)ptr)->seekg(pos, _from);
			}

			bool can_read(void* ptr) const override {
				return this->is_open(ptr);
			}
			uint64 read(void* ptr, void* _buf, uint64 count) const override {
				if (!this->is_open(ptr)) return 0;
				actual_type* inst = (actual_type*)ptr;
				inst->read((char*)_buf, count);
				return inst->gcount();
			}

			virtual ~istream_operator() {}
		};
		template<class T>
		class ostream_operator : public stream_operator_<T> {
			using actual_type = make_istream_t<T>;
		public:
			ostream_operator() {
				static_assert(!std::is_same_v<actual_type, invalid_stream>);
			}

			size_t tell(void* ptr) const override {
				if (!this->is_open(ptr)) return 0;
				return static_cast<size_t>(((actual_type*)ptr)->tellp());
			}
			void seek(void* ptr, int64 pos, std_ios_flag_type _from) override {
				if (!this->is_open(ptr)) return;
				((actual_type*)ptr)->seekp(pos, _from);
			}

			bool can_write(void* ptr) const override {
				return this->is_open(ptr);
			}
			uint64 write(void* ptr, const void* _data, uint64 count) override {
				if (!this->is_open(ptr)) return 0;
				actual_type* inst = (actual_type*)ptr;
				auto current_pos = this->tell(ptr);
				inst->write((const char*)_data, count);
				return this->tell(ptr) - current_pos;
			}

			void flush(void* ptr) override {
				if (!this->is_open(ptr)) return;
				((actual_type*)ptr)->flush();
			}
			virtual ~ostream_operator() {}
		};
		template<class T>
		class iostream_operator : public ostream_operator<T> {
			istream_operator<T> m_istr_op{};
		public:
			iostream_operator() {
				static_assert(!std::is_same_v<make_istream_t<T>, invalid_stream>);
			}
			bool can_read(void* ptr) const override {
				return this->is_open(ptr);
			}
			uint64 read(void* ptr, void* _buf, uint64 count) const override {
				return this->m_istr_op.read(ptr, _buf, count);
			}
			virtual ~iostream_operator() {}
		};
		class bstream_impl {
		protected:
			template<class T>
			static ref<stream_operator> create_istream_op() requires (is_istream_v<T>) {
				return ref<istream_operator<T>> {std::move(istream_operator<T>{})};
			}
			template<class T>
			static ref<stream_operator> create_istream_op() requires (!is_istream_v<T>) {
				return stream_operator{};
			}
			template<class T>
			static ref<stream_operator> create_ostream_op() requires (is_ostream_v<T>) {
				return ref<ostream_operator<T>> {std::move(ostream_operator<T>{})};
			}
			template<class T>
			static ref<stream_operator> create_ostream_op() requires (!is_ostream_v<T>) {
				return stream_operator{};
			}
			template<class T>
			static ref<stream_operator> create_iostream_op() requires (is_istream_v<T>&& is_ostream_v<T>) {
				return ref<iostream_operator<T>> {std::move(iostream_operator<T>{})};
			}
			template<class T>
			static ref<stream_operator> create_iostream_op() requires (!(is_istream_v<T>&& is_ostream_v<T>)) {
				return stream_operator{};
			}
			template<class T>
			static ref<stream_operator> create_operator() {
				if constexpr (is_istream_v<T> && is_ostream_v<T>) {
					return create_iostream_op<T>();
				} else if constexpr (is_ostream_v<T>) {
					return create_ostream_op<T>();
				} else if constexpr (is_istream_v<T>) {
					return create_istream_op<T>();
				} else {
					return stream_operator{};
				}
			}
		};
		template<class T>
		class bstream_impl_ : public bstream_impl {
		protected:
			ref<T> m_stream;
			ref<stream_operator> m_operator;
			template<class... Args>
			void open_stream(Args... args) {
				this->m_stream.new_if_empty();
				this->m_stream->open(std::forward<Args>(args)...);
			}
		public:
			bstream_impl_() {
				this->m_operator = create_operator<T>();
			}
			virtual ~bstream_impl_() {}
		};

		template<class T>
		constexpr bool is_byte_collection_v = cyh::type::is_container_v<T> && (std::is_same_v<cyh::trait::elem_type<T>, char> || std::is_same_v<cyh::trait::elem_type<T>, uint8>);
	};

	class bstream {
	public:
		static size_t length(bstream& str) {
			if (!str.is_open()) return 0;
			auto curr_idx = str.tell();
			str.seek(0, std::ios::end);
			auto len = str.tell() - curr_idx;
			str.seek(curr_idx, std::ios::beg);
			return len;
		}

		virtual bool can_read() const { return false; }
		virtual bool can_write() const { return false; }
		virtual bool is_open() const { return false; }
		virtual void close() {}
		virtual size_t tell() const { return 0; }
		virtual void seek(int64 pos, cyh::details::std_ios_flag_type _from = std::ios::beg) {}
		virtual ~bstream() {}
	};

	class ibstream : public bstream {
	public:
		virtual uint64 read(void* _buf, uint64 count) const { return 0; }
		template<class _Insertable>
		ibstream& operator >> (_Insertable& _target) requires (cyh::details::is_byte_collection_v<_Insertable>)
		{
			using element_t = cyh::trait::elem_type<_Insertable>;
			if (!this->can_read()) 
				return *this;
			char buffer[4096]{};
			size_t readed = 0;
			do {
				readed = this->read(buffer, 4096);
				_target.insert(_target.end(), (element_t*)buffer, (element_t*)(buffer + readed));
			} while (readed);
			return *this;
		}
		virtual ~ibstream() {}
	};

	class obstream : public bstream {
	public:
		virtual uint64 write(const void* _data, uint64 count) { return 0; }
		virtual void flush() {}
		template<class _Collection>
		obstream& operator << (const _Collection& _src) {
			if (!this->can_write()) return *this;
			auto size = std::distance(_src.begin(), _src.end());
			if (size != 0) {
				const auto* ptr = (const void*)&(*_src.begin());
				this->write(ptr, size);
			}
			return *this;
		}
		virtual ~obstream() {}
	};

	class iobstream : public ibstream, public obstream {};

	template<class T>
	class bstream_ : public virtual cyh::details::bstream_impl_<T>, public bstream {
	public:
		// bstream
		virtual bool can_read() const override {
			return this->m_operator->can_read(this->m_stream);
		}
		virtual bool can_write() const override {
			return this->m_operator->can_write(this->m_stream);
		}
		bool is_open() const override {
			return this->m_operator->is_open(this->m_stream);
		}
		size_t tell() const override {
			return this->m_operator->tell(this->m_stream);
		}
		void seek(int64 pos, cyh::details::std_ios_flag_type _from = std::ios::beg) override {
			this->m_operator->seek(this->m_stream, pos, _from);
		}
		void close() override {
			this->m_operator->close(this->m_stream);
		}

		virtual ~bstream_() {}
	};
	template<class T>
	class ibstream_ : public bstream_<T>, public ibstream {
		using base = cyh::bstream_<T>;
	public:
		// bstream
		virtual bool can_read() const override { return base::can_read(); }
		virtual bool can_write() const override { return false; }
		bool is_open() const override { return base::is_open(); }
		size_t tell() const override { return base::tell(); }
		void seek(int64 pos, cyh::details::std_ios_flag_type _from = std::ios::beg) override { base::seek(pos, _from); }
		void close() override { base::close(); }

		// ibstream
		uint64 read(void* _buf, uint64 count) const override {
			return this->m_operator->read(this->m_stream, _buf, count);
		}

		virtual ~ibstream_() {}
	};
	template<class T>
	class obstream_ : public bstream_<T>, public obstream {
		using base = cyh::bstream_<T>;
	public:
		// bstream
		virtual bool can_read() const override { return false; }
		virtual bool can_write() const override { return base::can_write(); }
		bool is_open() const override { return base::is_open(); }
		size_t tell() const override { return base::tell(); }
		void seek(int64 pos, cyh::details::std_ios_flag_type _from = std::ios::beg) override { base::seek(pos, _from); }
		void close() override { base::close(); }

		// obstream
		uint64 write(const void* _data, uint64 count) override {
			return this->m_operator->write(this->m_stream, _data, count);
		}
		void flush() override {
			this->m_operator->flush(this->m_stream);
		}
		virtual ~obstream_() {}
	};
	template<class T>
	class iobstream_ : public bstream_<T>, public virtual iobstream {
		using base = cyh::bstream_<T>;
	public:
		bool can_read() const override { return base::can_read(); }
		bool can_write() const override { return base::can_write(); }
		bool is_open() const override { return base::is_open(); }
		size_t tell() const override { return base::tell(); }
		void seek(int64 pos, cyh::details::std_ios_flag_type _from = std::ios::beg) override { base::seek(pos, _from); }
		void close() override { base::close(); }

		uint64 read(void* _buf, uint64 count) const override {
			return this->m_operator->read(this->m_stream, _buf, count);
		}
		uint64 write(const void* _data, uint64 count) override {
			return this->m_operator->write(this->m_stream, _data, count);
		}
		void flush() override {
			this->m_operator->flush(this->m_stream);
		}

		virtual ~iobstream_() {}
	};
};
