#pragma once
#include "typedef.hpp"
#include <vector>
namespace cyh::exception {
	std::string get_inner_message(std::exception_ptr ptr);
	class exception_base : public std::exception {
	protected:
		virtual const char* m_exception_type() const;
		std::vector<std::string> m_stack_trace;
		std::string m_msg;
		std::string m_src;
		std::string m_what;
	public:
		using std_ex_msg = decltype(std::declval<std::exception>().what());
		virtual ~exception_base() = default;
		exception_base() {}
		exception_base(const std::string_view& msg);
		exception_base(const std::exception& ex, const std::string_view& msg);
#ifdef _MSVC_LANG
		virtual std_ex_msg what() const override;
#else
		virtual std_ex_msg what() const noexcept override;
#endif
	};
};
namespace cyh::exception {
	class null_pointer_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "null pointer exception"; }
	public:
		null_pointer_exception() {}
		null_pointer_exception(const std::string_view& position) : exception_base(position) {}
		null_pointer_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class unknow_type_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "unknow input type"; }
	public:
		unknow_type_exception() {}
		unknow_type_exception(const std::string_view& position) : exception_base(position) {}
		unknow_type_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class invalid_type_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "invalid input type"; }
	public:
		invalid_type_exception() {}
		invalid_type_exception(const std::string_view& position) : exception_base(position) {}
		invalid_type_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class invalid_argument_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "invalid input value"; }
	public:
		invalid_argument_exception() {}
		invalid_argument_exception(const std::string_view& position) : exception_base(position) {}
		invalid_argument_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class out_of_range_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "invalid input index"; }
	public:
		out_of_range_exception() {}
		out_of_range_exception(const std::string_view& position) : exception_base(position) {}
		out_of_range_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class invalid_operation_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "invalid operation"; }
	public:
		invalid_operation_exception() {}
		invalid_operation_exception(const std::string_view& position) : exception_base(position) {}
		invalid_operation_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class logical_error_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "logical error in library"; }
	public:
		logical_error_exception() {}
		logical_error_exception(const std::string_view& position) : exception_base(position) {}
		logical_error_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};

	class platform_not_supported_exception final : public exception_base {
	protected:
		const char* m_exception_type() const override { return "library not support current platform"; }
	public:
		platform_not_supported_exception() {}
		platform_not_supported_exception(const std::string_view& position) : exception_base(position) {}
		platform_not_supported_exception& src(const std::string& pos) {
			this->m_src = pos;
			return *this;
		}
	};
};
