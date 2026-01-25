#include "task.hpp"
#ifdef __WINDOWS__
#include <Windows.h>
#else
#include <pthread.h>
#endif
namespace cyh::details {

	inline void _wait_millis(int millis) {
		std::this_thread::sleep_for(std::chrono::milliseconds(millis));
	}
	inline void _wait_micros(int micros) {
		std::this_thread::sleep_for(std::chrono::microseconds(micros));
	}
	inline void _wait_nanos(int micros) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(micros));
	}
	bool terminate_thread(std::thread::native_handle_type native_handle, int opt) {
#ifdef __WINDOWS__
		return TerminateThread(native_handle, opt);
#else
		return pthread_cancel(native_handle);
#endif
	}

	thread_handler::thread_handler()
	{
		this->m_state.create_new();
	}

	thread_handler::thread_handler(thread_handler&& other) noexcept
	{
		this->m_state = std::move(other.m_state);
		other.m_state.create_new();
		this->m_thread = std::move(other.m_thread);
		this->m_should_start = std::move(other.m_should_start);
		this->m_should_cancel = std::move(other.m_should_cancel);
		std::exchange(m_inner_exception, other.m_inner_exception);
	}

	thread_handler& thread_handler::operator=(thread_handler&& other) noexcept
	{
		this->m_state = std::move(other.m_state);
		other.m_state.create_new();
		this->m_thread = std::move(other.m_thread);
		this->m_should_start = std::move(other.m_should_start);
		this->m_should_cancel = std::move(other.m_should_cancel);
		std::exchange(m_inner_exception, other.m_inner_exception);
		return *this;
	}

	thread_handler::~thread_handler()
	{
		if (!this->is_initialized() || this->m_thread.empty())
			return;
		set_value_if_not_null(this->m_should_cancel, true);
		if (this->m_thread->joinable())
			this->m_thread->join();
	}

	bool thread_handler::setup(const std::function<void(const ref<bool>&)>& fn, const ref<bool>& _start_flag, const ref<bool>& _cancel_flag)
	{
		// if the thread is already inited, return false
		if (this->is_initialized()) return false;

		if (_start_flag.empty() || _cancel_flag.empty()) {
			return false;
		}

		this->m_should_start = _start_flag;
		this->m_should_cancel = _cancel_flag;

		if (this->m_thread.empty()) {

			if (this->m_state.empty())
				this->m_state.create_new();
			// update the state to inited
			this->m_state->fetch_or(THREAD_STATE_MASK::MASK_IS_INITED);
			if (this->m_should_start.empty())
				this->m_should_start.create_new();
			if (this->m_should_cancel.empty())
				this->m_should_cancel.create_new();

			this->m_thread = std::move(std::thread([=, this]() {
				// wait until the start or cancel flag is set
				while (!*this->m_should_start && !*this->m_should_cancel) {
					_wait_nanos(300);
				}
				// now the flag is set, check if the cancel flag is set				
				// if the cancel flag is not set, set the invoked flag and call the function
				if (!*this->m_should_cancel) {
					// set the invoked flag
					this->m_state->fetch_or(THREAD_STATE_MASK::MASK_IS_INVOKED);
					try {
						fn(this->m_should_cancel);
					} catch (...) {
						this->m_inner_exception = std::current_exception();
						this->m_state->fetch_or(THREAD_STATE_MASK::MASK_HAS_ERROR);
						// do nothing
					}
				}
				// set the finished flag
				this->m_state->fetch_or(THREAD_STATE_MASK::MASK_IS_FINISHED);
			}));
			return true;
		}
		return false;
	}

	bool thread_handler::is_initialized() const
	{
		return this->m_state->load() & THREAD_STATE_MASK::MASK_IS_INITED;
	}

	bool thread_handler::is_invoked() const
	{
		return this->m_state->load() & THREAD_STATE_MASK::MASK_IS_INVOKED;
	}

	bool thread_handler::is_running() const
	{
		auto stat = this->m_state->load();
		// this->is_invoked() && !this->is_finished();
		return (stat & THREAD_STATE_MASK::MASK_IS_INVOKED) && !(stat & THREAD_STATE_MASK::MASK_IS_FINISHED);
	}

	bool thread_handler::is_finished() const
	{
		return this->m_state->load() & THREAD_STATE_MASK::MASK_IS_FINISHED;
	}

	bool thread_handler::is_completed() const
	{
		auto stat = this->m_state->load();
		// this->is_finished() && (this->m_state->load() & THREAD_STATE_MASK::MASK_HAS_ERROR) == 0;
		return (stat & THREAD_STATE_MASK::MASK_IS_FINISHED) && ((stat & THREAD_STATE_MASK::MASK_HAS_ERROR) == 0);
	}

	bool thread_handler::invoke()
	{
		// if (this->is_initialized() && !this->is_invoked())
		auto stat = this->m_state->load();
		if ((stat & THREAD_STATE_MASK::MASK_IS_INITED) && !(stat & THREAD_STATE_MASK::MASK_IS_INVOKED))
		{
			(*this->m_should_start) = true;
			return true;
		}
		return false;
	}

	bool thread_handler::cancel()
	{
		// if (this->is_initialized() && !this->is_invoked())
		auto stat = this->m_state->load();
		if ((stat & THREAD_STATE_MASK::MASK_IS_INITED) && !(stat & THREAD_STATE_MASK::MASK_IS_INVOKED))
		{
			(*this->m_should_cancel) = true;
			return true;
		}
		return false;
	}

	bool thread_handler::is_invoking() const
	{
		if (this->m_should_start.empty())
			return false;
		return *this->m_should_start && !this->is_invoked();
	}

	int thread_handler::terminate()
	{
		try {
			if (this->is_running())
				return terminate_thread(this->m_thread->native_handle(), 0);
			return 0;
		} catch (...) {
			this->m_inner_exception = std::current_exception();
			return -1;
		}
	}

	ref<std::exception> thread_handler::get_inner_exception() const
	{
		if (m_inner_exception) {
			try {
				std::rethrow_exception(m_inner_exception);
			} catch (const std::exception& ex) {
				return make_ref(ex);
			}
		}
		return {};
	}

	task_impl::task_impl()
	{
	}

	task_impl::~task_impl()
	{
	}

	task_impl::task_impl(task_impl&& other) noexcept
	{
		this->m_start = std::move(other.m_start);
		this->m_cancel = std::move(other.m_cancel);
		this->m_result = std::move(other.m_result);
		this->is_fetched = other.is_fetched;
		other.is_fetched = false;
		this->m_thread_handler = std::move(other.m_thread_handler);
	}

	task_impl& task_impl::operator=(task_impl&& other) noexcept
	{
		this->m_start = std::move(other.m_start);
		this->m_cancel = std::move(other.m_cancel);
		this->m_result = std::move(other.m_result);
		this->is_fetched = other.is_fetched;
		other.is_fetched = false;
		this->m_thread_handler = std::move(other.m_thread_handler);
		// TODO: 於此處插入 return 陳述式
		return *this;
	}

	bool task_impl::wait_micros(uint micro_sec) const
	{
		if (!this->m_thread_handler.is_initialized()) return false;
		uint is_waited_microsec = 0;
		while (this->m_thread_handler.is_invoking() && is_waited_microsec < micro_sec) {
			_wait_micros(1);
			++is_waited_microsec;
		}
		while (!this->m_thread_handler.is_finished() && is_waited_microsec < micro_sec) {
			std::this_thread::sleep_for(std::chrono::microseconds(1));
			++is_waited_microsec;
		}
		return is_waited_microsec < micro_sec;
	}


	void task_impl::setup(const std::function<void(const ref<bool>&)>& pfunc, const ref<bool>& start_flag, const ref<bool>& cancel_flag)
	{
		if (!start_flag.empty())
			this->m_start = start_flag;
		else
			this->m_start.create_new();

		if (!cancel_flag.empty())
			this->m_cancel = cancel_flag;
		else
			this->m_cancel.create_new();

		this->m_thread_handler.setup(pfunc, this->m_start, this->m_cancel);
	}

	void task_impl::start()
	{
		this->m_thread_handler.invoke();
	}

	void task_impl::cancel(bool force_kill)
	{
		this->m_thread_handler.cancel();
	}

	bool task_impl::is_finished() const
	{
		return this->m_thread_handler.is_finished();
	}

	bool task_impl::get_result(const ref<reference>& recv)
	{
		if (this->is_fetched) return false;
		this->wait_micros(2147483647u);
		if (!this->m_thread_handler.is_completed()) {
			printf("task is failed!");
			return false;
		}
		this->is_fetched = true;
		if (this->m_result.empty()) return false;
		if (!this->m_result->has_result()) return true;
		if (recv.empty()) {
			this->m_result->reset();
		} else {
			this->m_result->get_result(recv);
		}
		return true;
	}

	ref<std::exception> task_impl::get_inner_exception() const
	{
		return this->m_thread_handler.get_inner_exception();
	}
}