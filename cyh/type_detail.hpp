#pragma once
#include "typedef.hpp"
namespace cyh {
	struct type_detail {
		size_t hash_code{};
		size_t type_size{};
		std::string_view fullname{};
		std::string_view value_type{};
		std::string_view type_name{};
		std::string_view type_namespace{};
		virtual ~type_detail() {}
	};
	template<class T>
	type_detail get_type_detail() {
		auto detail_ = cyh::type_detail{};
		auto pInfo = cyh::get_ptr(typeid(T));
		auto detail_ptr = &detail_;
		std::string_view fullname_with_type = pInfo->name();
		detail_ptr->hash_code = pInfo->hash_code();
		size_t space_index = fullname_with_type.find(' ');
		if (space_index == std::string::npos) { return detail_; }
		detail_ptr->value_type = fullname_with_type.substr(0, space_index);
		detail_ptr->fullname = fullname_with_type.substr(space_index + 1);
		size_t ns_index = fullname_with_type.find_last_of("::");
		if (ns_index != std::string::npos) {
			detail_ptr->type_name = fullname_with_type.substr(ns_index + 1);
			detail_ptr->type_namespace = fullname_with_type.substr(space_index + 1, ns_index - space_index - 2);
		} else {
			detail_ptr->type_name = detail_ptr->fullname;
		}
		detail_ptr->hash_code = pInfo->hash_code();
		detail_ptr->type_size = sizeof(T);
		return detail_;
	}
};
