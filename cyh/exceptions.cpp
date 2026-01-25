#include "exceptions.hpp"
namespace cyh::exception {
	std::string get_inner_message(std::exception_ptr ptr) {
		try {
			if (ptr) {
				std::rethrow_exception(ptr);
			} else {
				return "No error";
			}
		} catch (const std::exception& ex) {
			return ex.what();
		}
	}
	const char* exception_base::m_exception_type() const { return "unknow"; }
	exception_base::exception_base(const std::string_view& msg) {
		this->m_msg = msg;
	}
	exception_base::exception_base(const std::exception& ex, const std::string_view& msg) {
		this->m_stack_trace.push_back(ex.what());
		this->m_msg = msg;
	}

#ifdef _MSVC_LANG
	exception_base::std_ex_msg exception_base::what() const {
		if (this->m_msg.length()) {
			return this->m_msg.c_str();
		} else {
			return this->m_exception_type();
		}
	}
#else
	exception_base::std_ex_msg exception_base::what() const noexcept {
		if (this->m_msg.length()) {
			return this->m_msg.c_str();
		} else {
			return this->m_exception_type();
		}
	}
#endif
};
