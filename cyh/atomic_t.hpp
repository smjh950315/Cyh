#pragma once
#include <cyh/typedef.hpp>
#include <cyh/reference.hpp>
#include <cyh/exceptions.hpp>
#include <mutex>
#include <functional>
namespace cyh {
	template<class T>
	class atomic_ {
		ref<T> m_instance;
		ref<std::mutex> m_mutex;
		template<class Ret, class ... _Args, class ... Args>
		bool _exec(Ret* res, Ret(T::* pfunc)(_Args...), Args&&... args) requires(!std::is_same_v<Ret, void>) {
			if (res) {
				*res = std::move((this->m_instance->*pfunc)(std::forward<Args>(args)...));
			} else {
				(this->m_instance->*pfunc)(std::forward<Args>(args)...);
			}
		}
		template<class Ret, class ... _Args, class ... Args>
		bool _exec(Ret* res, Ret(T::* pfunc)(_Args...), Args&&... args) requires(std::is_same_v<Ret, void>) {
			(this->m_instance->*pfunc)(std::forward<Args>(args)...);
		}
	public:
		atomic_() requires(!std::is_default_constructible_v<T>) {}
		atomic_() requires(std::is_default_constructible_v<T>) {
			this->m_mutex.create_new();
			this->m_instance.create_new();
		}
		atomic_(T&& data) {
			this->m_mutex.create_new();
			this->m_instance = std::move(data);
		}
		atomic_(const atomic_<T>&) = delete;
		atomic_(atomic_<T>&& other) noexcept {
			this->m_instance = std::move(other.m_instance);
			this->m_mutex = std::move(other.m_mutex);
		}
		atomic_(T* _pdata, void(*_deleter)(void*)) {
			this->m_mutex.create_new();
			this->m_instance = ref<T>(_pdata, _deleter);
		}
		atomic_<T>& operator=(const atomic_<T>&) = delete;
		atomic_<T>& operator=(atomic_<T>&& other) noexcept {
			this->m_instance = std::move(other.m_instance);
			this->m_mutex = std::move(other.m_mutex);
			return *this;
		}
		~atomic_() {
			if (!this->m_mutex.empty()) {
				if (this->m_mutex->try_lock())
					this->m_mutex->unlock();
			}
		}
		bool is_initialized() const {
			return this->m_instance && this->m_mutex;
		}
		bool initialize_if_empty() requires(std::is_default_constructible_v<T>) {
			if (!this->is_initialized()) {
				this->initialize(T{});
			}
			return this->is_initialized();
		}
		bool invoke(const std::function<void(T&)>& func) {
			std::lock_guard<std::mutex> guard{ *this->m_mutex };
			func(*this->m_instance);
			return true;
		}
		void initialize(T&& data) {
			if (this->m_mutex) {
				if (this->m_mutex->try_lock()) {
					this->m_mutex->unlock();
				}
			} else {
				this->m_mutex.create_new();
			}
			this->m_instance = ref<T>(std::move(data));
		}
		void initialize(T* pdata, void(*deleter)(void*)) {
			if (this->m_mutex) {
				if (this->m_mutex->try_lock()) {
					this->m_mutex->unlock();
				}
			} else {
				this->m_mutex.create_new();
			}
			this->m_instance = ref<T>(pdata, deleter);
		}
		template<class V>
		bool try_get_value(V& output, std::function<V(T&)> getter) {
			return this->invoke([&](auto& inst) { output = getter(inst); });
		}
		template<class V>
		ref<V> try_get_value(std::function<V(T&)> getter) {
			V ret{};
			if (this->invoke([&](auto& inst)
				{
					ret = getter(inst);
				}))
			{
				return ref<V>(std::move(ret));
			}
			return ref<V>{};
		}
		bool is_initialized() {
			return !this->m_instance.empty();
		}
	};
	template<class T>
	using atomic_ref = ref<atomic_<T>>;
};
