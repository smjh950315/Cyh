#pragma once
#include "reference.hpp"
#include <functional>
#include <thread>
#include <future>
#include <cyh/exceptions.hpp>
namespace cyh {

	class itask {
	public:
		virtual ~itask() {}
		virtual bool is_finished() const = 0;
		virtual void wait_for_millis(int) = 0;
		virtual void wait() = 0;
		virtual bool wait_and_finish() = 0;
		virtual bool try_get_result_v(void* pres, std::string* inner_msg = nullptr) = 0;
		virtual void start() = 0;
		virtual void cancel(bool force_kill = false) = 0;
	};

	template<class T>
	class task_;
	namespace details {
		bool terminate_thread(std::thread::native_handle_type native_handle, int opt);
		struct THREAD_STATE_MASK {
			static constexpr int MASK_IS_INITED = 0b01;
			static constexpr int MASK_IS_INVOKED = 0b10;
			static constexpr int MASK_IS_FINISHED = 0b100;
			static constexpr int MASK_HAS_ERROR = 0b1000;
		};
		struct task_result_base {
			virtual const std::type_info* result_type() const = 0;
			virtual void get_result(const ref<reference>& receiver) = 0;
			virtual bool has_result() const = 0;
			virtual void reset() = 0;
			virtual ~task_result_base() {}
		};
		template<class T>
		struct task_result final : task_result_base {
			ref<T> result_ptr{};
			const std::type_info* result_type() const override { return &typeid(T); }
			void get_result(const ref<reference>& receiver) override;
			bool has_result() const override;
			void reset() override { this->result_ptr.release(); }
			~task_result() = default;
		};
		template<>
		struct task_result<void> final : task_result_base {
			const std::type_info* result_type() const override { return &typeid(void); };
			void get_result(const ref<reference>& receiver) override {}
			bool has_result() const override { return false; }
			void reset() override {}
			~task_result() = default;
		};
		class thread_handler final {
			ref<std::thread> m_thread{};
			ref<std::atomic<int>> m_state{};
			ref<bool> m_should_start{};
			ref<bool> m_should_cancel{};
			std::exception_ptr m_inner_exception{};
		public:
			thread_handler();
			thread_handler(const thread_handler&) = delete;
			thread_handler(thread_handler&& other) noexcept;
			thread_handler& operator=(const thread_handler&) = delete;
			thread_handler& operator=(thread_handler&& other) noexcept;
			~thread_handler();

			// setup the delegate
			bool setup(const std::function<void(const ref<bool>&)>& fn, const ref<bool>& _start_flag, const ref<bool>& _cancel_flag);
			// indicates that the thread is initialized(which the delegate is set)
			bool is_initialized() const;
			// indicates that the thread is invoked
			bool is_invoked() const;
			// indicates that the thread is running
			bool is_running() const;
			// finished with or without error
			bool is_finished() const;
			// finished without error
			bool is_completed() const;
			// run the delegate (by set m_should_start to true)
			bool invoke();
			// cancel the delegate (by set m_should_cancel to true)
			bool cancel();
			// indicates that the task is in the invoke sequence
			bool is_invoking() const;
			// unsafe
			[[nodiscard]] int terminate();
			// get inner exception, null if no exception
			ref<std::exception> get_inner_exception() const;
		};
		class task_impl final {
			cyh::details::thread_handler m_thread_handler{};
			ref<cyh::details::task_result_base> m_result{};
			ref<bool> m_start{};
			ref<bool> m_cancel{};
			bool is_fetched{};
			template<class T>
			friend class task_;
			template<class T>
			ref<task_result<T>> init_get_result_holder();
		public:
			task_impl();
			~task_impl();
			task_impl(const task_impl&) = delete;
			task_impl(task_impl&& other) noexcept;
			task_impl& operator=(const task_impl&) = delete;
			task_impl& operator=(task_impl&& other) noexcept;
			bool wait_micros(uint micro_sec) const;
			void setup(const std::function<void(const ref<bool>&)>& pfunc, const ref<bool>& start_flag = null, const ref<bool>& cancel_flag = null);
			void start();
			void cancel(bool force_kill = false);
			bool is_finished() const;
			bool get_result(const ref<reference>& ref);
			// get inner exception, null if no exception
			ref<std::exception> get_inner_exception() const;
		};
	};
	template<class T>
	class task_ final : public virtual itask {
		ref<cyh::details::task_impl> m_impl;
		std::function<void(const ref<bool>&)> make_delegate(const std::function<T(const ref<bool>&)>& pfunc) requires(!std::is_same_v<T, void>);
		std::function<void(const ref<bool>&)> make_delegate(const std::function<T(const ref<bool>&)>& pfunc) requires(std::is_same_v<T, void>);
		bool wait_micros(uint micro_sec) const;
	public:
		~task_() {}
		task_(const task_&) = default;
		task_& operator=(const task_&) = default;
		task_(task_&& other) noexcept = default;
		task_& operator=(task_&& other) noexcept = default;
		task_(const std::function<T(const ref<bool>&)>& pfunc, const ref<bool>& start_flag = null, const ref<bool>& cancel_flag = null);
		bool get_result(T* pres = nullptr, std::string* inner_msg = nullptr) requires(!std::is_same_v<T, void>);
		bool get_result(T* pres = nullptr, std::string* inner_msg = nullptr) requires(std::is_same_v<T, void>);
		bool try_get_result(T* pres = nullptr, std::string* inner_msg = nullptr);
		bool try_get_result_v(void* pres, std::string* inner_msg = nullptr) override;

		bool is_finished() const override;
		bool wait_and_finish() override;
		void wait_for_millis(int millis) override;
		void wait() override;
		void start() override;
		void cancel(bool force_kill = false) override;
		// get inner exception, null if no exception
		ref<std::exception> get_inner_exception() const;

		static ref<task_<T>> run(const std::function<T(const ref<bool>&)>& pfunc, const ref<bool>& start_flag = null, const ref<bool>& termainate_flag = null);
	};

	using task = task_<void>;
};

namespace cyh::details {
	template<class T>
	void task_result<T>::get_result(const ref<reference>& receiver) {
		if (receiver.empty() || this->result_ptr.empty()) return;
		auto recv_inst = receiver.as<ref<T>>();
		if (!recv_inst.empty()) {
			*recv_inst = this->result_ptr;
		}
	}
	template<class T>
	bool task_result<T>::has_result() const {
		return !this->result_ptr.empty();
	}
	template<class T>
	ref<task_result<T>> task_impl::init_get_result_holder() {
		ref<task_result<T>> holder{};
		holder.create_new();
		this->m_result = holder;
		return holder;
	}
}
namespace cyh {
	template<class T>
	std::function<void(const ref<bool>&)> task_<T>::make_delegate(const std::function<T(const ref<bool>&)>& pfunc) requires(!std::is_same_v<T, void>) {
		this->m_impl.new_if_empty();
		auto holder = this->m_impl->init_get_result_holder<T>();
		return [=](const ref<bool>& cancelled) {
			if (!holder.empty()) {
				holder->result_ptr = std::move(pfunc(cancelled));
			} else {
				pfunc(cancelled);
			}
			};
	}
	template<class T>
	std::function<void(const ref<bool>&)> task_<T>::make_delegate(const std::function<T(const ref<bool>&)>& pfunc) requires(std::is_same_v<T, void>) {
		return [=](const ref<bool>& cancelled) { pfunc(cancelled); };
	}
	template<class T>
	bool task_<T>::wait_micros(uint micro_sec) const { return this->m_impl.empty() ? false : this->m_impl->wait_micros(micro_sec); }
	template<class T>
	task_<T>::task_(const std::function<T(const ref<bool>&)>& pfunc, const ref<bool>& start_flag, const ref<bool>& cancel_flag) {
		this->m_impl.new_if_empty();
		this->m_impl->setup(this->make_delegate(pfunc), start_flag, cancel_flag);
	}
	template<class T>
	bool task_<T>::get_result(T* pres, std::string* inner_msg) requires(!std::is_same_v<T, void>) {
		if (this->m_impl.empty())
			return false;
		ref<ref<T>> recver{};
		if (pres != nullptr)
			recver.create_new();
		if (this->m_impl->get_result(recver)) {
			if (!recver->empty() && pres != nullptr) {
				*pres = **recver;
			}
			return true;
		}
		return false;
	}
	template<class T>
	bool task_<T>::get_result(T* pres, std::string* inner_msg) requires(std::is_same_v<T, void>) {
		return this->m_impl.empty() ? false : this->m_impl->get_result({});
	}
	template<class T>
	bool task_<T>::try_get_result(T* pres, std::string* inner_msg) {
		if (!this->is_finished()) return false;
		return this->get_result(pres, inner_msg);
	}
	template<class T>
	bool task_<T>::try_get_result_v(void* pres, std::string* inner_msg) {
		return this->try_get_result((T*)pres, inner_msg);
	}
	template<class T>
	bool task_<T>::is_finished() const { return this->m_impl.empty() ? true : this->m_impl->is_finished(); }
	template<class T>
	bool task_<T>::wait_and_finish() { return this->get_result((T*)0); }
	template<class T>
	void task_<T>::wait_for_millis(int millis) { this->wait_micros(millis * 1000); }
	template<class T>
	void task_<T>::wait() { this->wait_micros(~0); }
	template<class T>
	void task_<T>::start() { if (!this->m_impl.empty()) this->m_impl->start(); }
	template<class T>
	void task_<T>::cancel(bool force_kill) { if (!this->m_impl.empty()) this->m_impl->cancel(force_kill); }
	template<class T>
	ref<std::exception> task_<T>::get_inner_exception() const { return this->m_impl.empty() ? ref<std::exception>{} : this->m_impl->get_inner_exception(); }
	template<class T>
	ref<task_<T>> task_<T>::run(const std::function<T(const ref<bool>&)>& pfunc, const ref<bool>& start_flag, const ref<bool>& termainate_flag) {
		task_<T> t{ pfunc, start_flag, termainate_flag };
		t.start();
		return ref<task_<T>>{ std::move(t) };
	}
}
