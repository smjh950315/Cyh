#pragma once
#include <any>
#include <type_traits>
#include <typeinfo>
#include <cyh/text.hpp>
namespace cyh {
	class cloneable {
		template<class T>
		static bool try_to_str(std::string& _res, std::any* _p) {
			T* _ptr = std::any_cast<T>(_p);
			if (!_ptr)
				return false;
			const T& val = *_ptr;
			_res = cyh::to_string(val);
			return true;
		}
		std::any m_data{};
		std::type_info* m_type_info{};
		bool(*fn_to_string)(std::string&, std::any*);
	public:
		template<class T>
		bool is_type() const {
			return &typeid(T) == this->m_type_info;
		}
		std::string to_string(const std::string& _val_on_failure = "") const {
			if (!this->fn_to_string)
				return _val_on_failure;
			std::string _res;
			if (this->fn_to_string(_res, get_ptr(this->m_data)))
				return _res;
			return _val_on_failure;
		}
		template<class T>
		cloneable(T&& _any) {
			this->m_data = std::forward<T>(_any);
			this->m_type_info = get_ptr(typeid(T));
			this->fn_to_string = cloneable::try_to_str<std::decay_t<T>>;
		}
	};
};