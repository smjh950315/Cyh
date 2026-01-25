#pragma once
#include <cyh/reference.hpp>
#include "reflection_.hpp"
#include "partial_glz.hpp"
#include <map>
#include <any>
using namespace partial_glz;
namespace cyh::reflection {

	template<class T>
	constexpr size_t get_member_count() { return partial_glz::count_members<T>; }

	template<class T>
	std::vector<std::string> get_member_names() {
		static auto member_name_array = partial_glz::member_names<T>;
		static std::vector<std::string> names;
		if (names.size() != 0) { return names; }
		names.reserve(member_name_array.size());
		for (auto& member_name : member_name_array) {
			names.push_back(std::string(member_name));
		}
		return names;
	}

	template<class T>
	void try_set_members(T& valref, const std::unordered_map<std::string, std::any>& src, bool skip_error = false);

	template<class T, class V>
	void try_set_member(T& valref, const std::string_view& name, const V& value);

	template<class T, class Src>
	bool try_get_member(T& output, const Src& src, const std::string_view& name);

	template<class T>
	std::unordered_map<std::string_view, ref<member_visitor>> get_member_visitors(); 

#ifdef INCLUDE_NLOHMANN_JSON_HPP_
	template<class T>
	void try_deserialize(T& valref, const nlohmann::json& src, bool skip_error = true);
	template<class T>
	void try_serialize(nlohmann::json& j, const T& valref, bool skip_error, bool bypass_null = false);
#endif

	namespace details {
		using sv = std::string_view;
		template<class Tuple, size_t _Index>
		using index_value_type = std::decay_t<decltype(std::get<_Index>(std::declval<Tuple>()))>;
		template<class Type>
		using member_ref_tuple_type = std::decay_t<decltype(partial_glz::to_tuple(std::declval<Type>()))>;
		template<class T>
		inline constexpr bool is_builtin_v = cyh::type::is_any_of_v<T,
			bool,
			char, wchar_t, ch8, ch16, ch32,
			uchar,
			short, ushort,
			int, uint,
			long, ulong,
			long long, unsigned long long,
			float, double, long double,
			std::string, std::string_view>;
		template<class T, size_t Idx>
		struct member_visitor_ final : public member_visitor {
			using object_tuple = member_ref_tuple_type<T>;
			using member_type = index_value_type<object_tuple, Idx>;
			~member_visitor_() = default;
			member_visitor_(std::string_view _property_name) {
				this->property_name = std::string(_property_name);
				this->parent_type_info_ptr = get_ptr(typeid(T));
				this->type_info_ptr = get_ptr(typeid(member_type));
				this->is_builtin_type = is_builtin_v<member_type>;
			}
			std::any get(void* _object) const override {
				if (_object == nullptr)
					return std::any();
				T* obj = (T*)_object;
				auto tuple = partial_glz::to_tuple(*obj);
				return std::any(std::get<Idx>(tuple));
			}
			bool set(void* _object, const std::any& _value) const override {
				if (_object == nullptr)
					return false;
				T* obj = (T*)_object;				
				auto tuple = partial_glz::to_tuple(*obj);
				auto pvalue = std::any_cast<member_type>(get_ptr(_value));
				if (pvalue) {
					std::get<Idx>(tuple) = *pvalue;
					return true;
				} else {
					//throw cyh::exception::invalid_type_exception(__POSITION__);
					return false;
				}
			}
		};
		template<class T, class Src>
		static bool set_value_t(T& dst, Src& src) requires(!cyh::type::is_assignable_tolvalref_v<T, Src>) { return false; }
		template<class T, class Src>
		static bool set_value_t(T& dst, Src& src) requires(cyh::type::is_assignable_tolvalref_v<T, Src>) {
			dst = src;
			return true;
		}

		template<size_t I, size_t M, class Tuple>
		void set_member_t(Tuple& valref, std::array<sv, M>& names, std::unordered_map<std::string, std::any>& src, bool skip_error) requires(I >= M) { }
		template<size_t I, size_t M, class Tuple>
		void set_member_t(Tuple& valref, std::array<sv, M>& names, std::unordered_map<std::string, std::any>& src, bool skip_error) requires(I < M) {
			auto iter = src.find(std::string(names[I]));
			if (iter != src.end()) {
				auto pvalue = std::any_cast<index_value_type<Tuple, I>>(&(iter->second));
				if (!pvalue && !skip_error) {
					throw cyh::exception::invalid_type_exception(__POSITION__);
				}
				if (pvalue) {
					std::get<I>(valref) = *pvalue;
				}
			}
			set_member_t<I + 1, M, Tuple>(valref, names, src, skip_error);
		}

#ifdef INCLUDE_NLOHMANN_JSON_HPP_
		template<class T>
		struct json_convertable {
			template <class U>
			static auto test(U u) -> decltype(std::declval<nlohmann::json>().get<U>(), std::true_type{});
			static std::false_type test(...);
			static constexpr bool value = !std::is_same_v<decltype(test(std::declval<T>())), std::false_type>;
		};

		template<class T, class R>
		void try_read_json(R& val, nlohmann::json& j, bool skip_error) requires(json_convertable<T>::value) { val = j.get<T>(); }
		template<class T, class R>
		void try_read_json(R& val, nlohmann::json& j, bool skip_error) requires(!json_convertable<T>::value) {
			try_deserialize(val, j, skip_error);
		}

		template<size_t I, size_t M, class Tuple>
		void set_member_from_json(Tuple& valref, std::array<sv, M>& names, nlohmann::json& src, bool skip_error) requires(I >= M) {}
		template<size_t I, size_t M, class Tuple>
		void set_member_from_json(Tuple& valref, std::array<sv, M>& names, nlohmann::json& src, bool skip_error) requires(I < M) {
			using member_type = index_value_type<Tuple, I>;
			auto iter = src.find(std::string(names[I]));
			if (iter != src.end()) {
				if (iter->is_null()) {
					// do nothing
				} else {
					if (!iter->is_object()) {
						try {
							try_read_json<member_type>(std::get<I>(valref), *iter, skip_error);
						} catch (...) {
							if (!skip_error) {
								throw cyh::exception::invalid_type_exception(__POSITION__);
							}
						}
					} else {		
						auto& member = std::get<I>(valref);
						try_deserialize(member, *iter, skip_error);
					}
				}
			}
			set_member_from_json<I + 1, M, Tuple>(valref, names, src, skip_error);
		}

		template<class T>
		void try_deserialize_impl_v(T& valref, nlohmann::json& src, bool skip_error) requires (std::is_aggregate_v<std::remove_cvref_t<T>>)
		{
			auto tuple = partial_glz::to_tuple(valref);
			static auto names = partial_glz::member_names<T>;
			auto& refsrc = *get_ptr(src);
			details::set_member_from_json<0>(tuple, names, refsrc, skip_error);
		}
		template<class T>
		void try_deserialize_impl_v(T& valref, nlohmann::json& src, bool skip_error) requires (!std::is_aggregate_v<std::remove_cvref_t<T>>)
		{
			try {
				try_read_json<T>(valref, src, skip_error);
			} catch (...) {
				if (!skip_error) {
					throw cyh::exception::invalid_type_exception(__POSITION__);
				}
			}
		}
		template<class T>
		void try_deserialize_impl(T& valref, const nlohmann::json& src, bool skip_error)
		{
			auto& refsrc = *get_ptr(src);
			if (src.is_null()) {
				// do nothing
			} else {
				try_deserialize_impl_v<T>(valref, refsrc, skip_error);
			}
		}
		template<class T>
		void try_deserialize_impl(ref<T>& valref, const nlohmann::json& src, bool skip_error)
		{
			auto& refsrc = *get_ptr(src);
			if (src.is_null()) {
				valref.release();
			} else {
				valref.new_if_empty();
				try_deserialize_impl_v<T>(*valref, refsrc, skip_error);
			}
		}

		template<class R>
		void try_set_json_v(nlohmann::json& j, const R& val, bool skip_error, bool bypass_null) requires(json_convertable<R>::value) {
			j = val;
		}
		template<class R>
		void try_set_json_v(nlohmann::json& j, const R& val, bool skip_error, bool bypass_null) requires(!json_convertable<R>::value) {
			try_serialize<R>(j, val, skip_error, bypass_null);
		}
		template<class R>
		void try_set_json(nlohmann::json& j, const R& val, bool skip_error, bool bypass_null) {
			try_set_json_v<R>(j, val, skip_error, bypass_null);
		}
		template<class R>
		void try_set_json(nlohmann::json& j, const ref<R>& val, bool skip_error, bool bypass_null) {
			if (val.empty()) {
				j = NAN;
			} else {
				try_set_json_v<R>(j, *val, skip_error, bypass_null);
			}
		}

		template<class T>
		bool is_nullref(T& val) { return false; }
		template<class T>
		bool is_nullref(ref<T>& val) { return val.empty(); }
		template<size_t I, size_t M, class Tuple>
		void read_member_to_json(nlohmann::json& j, std::array<sv, M>& names, Tuple& srcref, bool skip_error, bool bypass_null) requires(I >= M) {}
		template<size_t I, size_t M, class Tuple>
		void read_member_to_json(nlohmann::json& j, std::array<sv, M>& names, Tuple& srcref, bool skip_error, bool bypass_null) requires(I < M) {
			using member_type = index_value_type<Tuple, I>;			
			auto& srcrefv = std::get<I>(srcref);
			if (is_nullref(srcrefv) && bypass_null) {
				// do nothing
			} else {
				j.emplace(names[I], nlohmann::json{});
				auto& j2 = j[names[I]];
				try_set_json(j2, srcrefv, skip_error, bypass_null);
			}
			read_member_to_json<I + 1, M, Tuple>(j, names, srcref, skip_error, bypass_null);
		}
		template<class T>
		void try_serialize_impl_v(nlohmann::json& j, const T& valref, bool skip_error, bool bypass_null) requires (std::is_aggregate_v<std::remove_cvref_t<T>>)
		{
			auto& refsrc = *get_ptr(valref);
			auto tuple = partial_glz::to_tuple(refsrc);
			static auto names = partial_glz::member_names<T>;
			read_member_to_json<0>(j, names, tuple, skip_error, bypass_null);
		}
		template<class T>
		void try_serialize_impl_v(nlohmann::json& j, const T& valref, bool skip_error, bool bypass_null) requires (!std::is_aggregate_v<std::remove_cvref_t<T>>)
		{
			try {
				try_set_json_v<T>(j, valref, skip_error, bypass_null);
			} catch (...) {
				if (!skip_error) {
					throw cyh::exception::invalid_type_exception(__POSITION__);
				}
			}
		}
		template<class T>
		void try_serialize_impl(nlohmann::json& j, const T& valref, bool skip_error, bool bypass_null)
		{
			try_serialize_impl_v<T>(j, valref, skip_error, bypass_null);
		}
		template<class T>
		void try_serialize_impl(nlohmann::json& j, const ref<T>& valref, bool skip_error, bool bypass_null)
		{
			if (valref.empty()) {
				j = NAN;
			} else {
				try_serialize_impl_v<T>(j, *valref, skip_error, bypass_null);
			}
		}
#endif

		template<size_t I, size_t M, class ValueType, class Tuple>
		void set_member_by_index(Tuple& valref, std::array<sv, M>& names, size_t target_index, ValueType& val) requires(I >= M) { }
		template<size_t I, size_t M, class ValueType, class Tuple>
		void set_member_by_index(Tuple& valref, std::array<sv, M>& names, size_t target_index, ValueType& val) requires(I < M) {
			if (target_index > M) {
				return;
			} else if (target_index < I) {
				set_member_by_index<I + 1, M, ValueType, Tuple>(valref, names, target_index, val);
			} else {
				using value_type = index_value_type<Tuple, I>;
				set_value_t<value_type, ValueType>(std::get<I>(valref), *cyh::get_ptr(val));
			}
		}

		template<size_t N, size_t M, class T, class Tuple>
		static bool get_value_by_index(T& dst, Tuple& tuple, size_t index) requires(N >= M) { return false; }
		template<size_t N, size_t M, class T, class Tuple>
		static bool get_value_by_index(T& dst, Tuple& tuple, size_t index) requires(N < M) {
			if (N > index) {
				return false;
			} else if (N < index) {
				return get_value_by_index<N + 1, M, T, Tuple>(dst, tuple, index);
			} else { // N == index					
				using value_type = index_value_type<Tuple, N>;
				return set_value_t<T, value_type>(dst, std::get<N>(tuple));
			}
		}
		template<size_t M, class T, class Tuple>
		static bool get_value_by_index_begin(T& dst, Tuple& tuple, size_t index, std::array<std::string_view, M>& memberNames) {
			return get_value_by_index<0, M, T, Tuple>(dst, tuple, index);
		}

		template<size_t N, size_t M, class T, class Tuple>
		static bool insert_visitor(std::unordered_map<std::string_view, ref<member_visitor>>& visitors, std::array<std::string_view, M>& memberNames) requires(N >= M) { return false; }
		template<size_t N, size_t M, class T, class Tuple>
		static bool insert_visitor(std::unordered_map<std::string_view, ref<member_visitor>>& visitors, std::array<std::string_view, M>& memberNames) requires(N < M) {
			using member_type = index_value_type<Tuple, N>;		
			if constexpr (N >= M) {
				return false;
			} else {
				ref<member_visitor_<T, N>> visitor;
				visitor.create_new(memberNames[N]);
				visitors.emplace(memberNames[N], visitor.template as<member_visitor>());
				return insert_visitor<N + 1, M, T, Tuple>(visitors, memberNames);
			} 
		}
	};

	template<class T>
	void try_set_members(T& valref, const std::unordered_map<std::string, std::any>& src, bool skip_error) {
		auto tuple = partial_glz::to_tuple(valref);
		static auto names = partial_glz::member_names<T>;
		auto& refsrc = *get_ptr(src);
		details::set_member_t<0>(tuple, names, refsrc, skip_error);
	}
	template<class T, class V>
	void try_set_member(T& valref, const std::string_view& name, const V& value) {
		auto tuple = partial_glz::to_tuple(valref);
		static auto names = partial_glz::member_names<T>;
		size_t index = 0;
		for (auto& sv : names) {
			if (name == sv) { break; }
			++index;
		}
		if (index == names.size()) { return; }
		auto& refsrc = *cyh::get_ptr(value);
		details::set_member_by_index<0>(tuple, names, 1, refsrc);
	}
	template<class T, class Src>
	bool try_get_member(T& output, const Src& src, const std::string_view& name) {
		static auto memberNames = partial_glz::member_names<Src>;
		auto tuple = partial_glz::to_tuple(*cyh::get_ptr(src));
		size_t memberIndex = ~size_t{};
		for (size_t i = 0; i < memberNames.size(); ++i) {
			if (name == memberNames[i]) {
				memberIndex = i;
				break;
			}
		}
		if (memberIndex != ~size_t{}) {
			return details::get_value_by_index_begin(output, tuple, memberIndex, memberNames);
		} else {
			return false;
		}
	}
	template<class T>
	std::unordered_map<std::string_view, ref<member_visitor>> get_member_visitors() {
		std::unordered_map<std::string_view, ref<member_visitor>> result;
		static auto memberNames = partial_glz::member_names<T>;
		constexpr size_t memberCount = partial_glz::count_members<T>;
		details::insert_visitor<0, memberCount, T, details::member_ref_tuple_type<T>>(result, memberNames);
		return result;
	}

#ifdef INCLUDE_NLOHMANN_JSON_HPP_
	template<class T>
	void try_deserialize(T& valref, const nlohmann::json& src, bool skip_error) {
		details::try_deserialize_impl(valref, src, skip_error);
	}
	template<class T>
	void try_serialize(nlohmann::json& j, const T& valref, bool skip_error, bool bypass_null) {
		details::try_serialize_impl(j, valref, skip_error, bypass_null);
	}
#endif
};
