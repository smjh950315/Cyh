#pragma once
#include <thread>
#include <future>
#include <cyh/atomic_t.hpp>
#include <queue>
namespace cyh {

	template<size_t _MaxThread, typename Iterator, typename Func>
	void parallel_for(Iterator begin, Iterator end, Func func) {

		struct _iter_handle {
			Iterator m_beg, m_end;
			bool try_pop(Iterator& value) {
				if (m_beg == m_end)
					return false;
				value = this->m_beg;
				++this->m_beg;
				return true;
			}
			_iter_handle(Iterator _beg, Iterator _end) : m_beg(_beg), m_end(_end) {}
			_iter_handle(_iter_handle&&) = default;
			_iter_handle& operator=(_iter_handle&&) = default;
			_iter_handle(const _iter_handle&) = delete;
			_iter_handle& operator=(const _iter_handle&) = delete;
		};

		cyh::atomic_<_iter_handle> atomic_stack = std::move(_iter_handle{ begin, end });

		std::vector<std::future<void>> futures;

		for (size_t i = 0; i < _MaxThread; ++i) {
			futures.emplace_back(std::async(std::launch::async, [&]() {
				bool has_more = true;
				while (has_more) {
					Iterator it{};
					atomic_stack.invoke([&](auto& stack) {
						has_more = stack.try_pop(it);
					});
					if (has_more) {
						func(*it);
					}
				}
			}));
		}

		for (auto& f : futures) f.get();
	}

	template<typename Iterator, typename Func>
	std::vector<trait::func_return_type<Func, decltype(*std::declval<Iterator>())>>
		select(Iterator begin, Iterator end, Func func, std::function<bool(decltype(*std::declval<Iterator>()))> filter = {})
		requires(type::is_callableby_v<Func, decltype(*std::declval<Iterator>())>) {
		using element_type = trait::func_return_type<Func, decltype(*std::declval<Iterator>())>;
		std::vector<element_type> results;
		auto count = std::distance(begin, end);
		results.reserve(count);
		while (begin != end) {
			if (filter) {
				if (filter(*begin))
					results.push_back(func(*begin));
			} else {
				results.push_back(func(*begin));
			}			
			begin++;
		}
		return results;
	}

	template<typename Iterator, typename Func>
	ref<trait::func_return_type<Func, decltype(*std::declval<Iterator>())>>
		select_first_or_null(Iterator begin, Iterator end, Func func, std::function<bool(decltype(*std::declval<Iterator>()))> filter = {})
		requires(type::is_callableby_v<Func, decltype(*std::declval<Iterator>())>) {
		using element_type = trait::func_return_type<Func, decltype(*std::declval<Iterator>())>;
		while (begin != end) {
			if (filter) {
				if (filter(*begin))
					return std::move(func(*begin));
			} else {
				return std::move(func(*begin));
			}
			begin++;
		}
		return {};
	}
}
