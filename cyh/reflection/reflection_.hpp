#pragma once
#include <cyh/typedef.hpp>
#include <cyh/exceptions.hpp>
#include <cyh/reference.hpp>
#include <any>
namespace cyh::reflection {
	class member_visitor {
	public:
		std::string property_name;
		std::type_info* parent_type_info_ptr{ nullptr };
		std::type_info* type_info_ptr{ nullptr };
		bool is_builtin_type{ false };
		virtual ~member_visitor() = default;
		virtual std::any get(void* _object) const = 0;
		virtual bool set(void* _object, const std::any& _value) const = 0;
	};
};
