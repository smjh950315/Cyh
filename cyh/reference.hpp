#pragma once
#include "memory_helper.hpp"
#include "type_detail.hpp"
#include <atomic>
namespace cyh {

	namespace details {
		struct unsafe_opt;
	};

	template<class T>
	class reference_;

	class reference {
		template<class U>
		friend class reference_;
		friend cyh::details::unsafe_opt;
	protected:
		using deallocator_type = void(*)(void*);
		using counter_type = std::atomic<size_t>;
	private:
		struct control_block {
			deallocator_type m_deallocator;
			counter_type m_ref_counter{ 1 };
			type_info* m_type_info;
			void* m_address{};
			void* m_instance{};
			const char* m_typename{};
			control_block() = delete;
			control_block(const control_block&) = delete;
			control_block(control_block&&) = delete;
			control_block& operator=(const control_block&) = delete;
			control_block& operator=(control_block&&) = delete;
			template<class T>
			control_block(T* inst, void* addr) :
				m_instance(inst),
				m_address(addr), 
				m_deallocator(MemoryHelper::Finalize<T>),
				m_type_info(cyh::get_ptr(typeid(T))),
				m_typename(typeid(T).name()) { }
			template<class T>
			control_block(T* inst) : 
				m_instance(inst), 
				m_address(inst),
				m_deallocator(MemoryHelper::Finalize<T>),
				m_type_info(cyh::get_ptr(typeid(T))),
				m_typename(typeid(T).name()) { }
			template<class T>
			control_block(T* inst, deallocator_type dealloc) : 
				m_instance(inst),
				m_address(inst),
				m_deallocator(dealloc),
				m_type_info(cyh::get_ptr(typeid(T))),
				m_typename(typeid(T).name()) {}
			~control_block() {
				if (this->m_instance != nullptr) {
					this->m_deallocator(this->m_instance);
				}
				if (this->m_address != nullptr) {
					MemoryHelper::Free(this->m_address);
				}
			}
		};
		control_block* m_block;
	protected:
		template<class T>
		reference(T* p_inst) : m_block(p_inst != nullptr ? (control_block*)MemoryHelper::CreateObject<control_block>(p_inst) : nullptr) {}
		template<class T>
		reference(T* p_inst, void* _addr) : m_block(p_inst != nullptr ? (control_block*)MemoryHelper::CreateObject<control_block>(p_inst, _addr) : nullptr) {}
		template<class T>
		reference(T* p_inst, void(*cust_dealloc)(void*)) : m_block(p_inst != nullptr ? (control_block*)MemoryHelper::CreateObject<control_block>(p_inst, cust_dealloc) : nullptr) {}
		template<class T>
		void set_instance(T* inst) {
			if (this->m_block != nullptr) {
				this->m_block->m_instance = inst;
				this->m_block->m_address = inst;
			} else {
				this->m_block = (control_block*)MemoryHelper::CreateObject<control_block>(inst);
			}
		}
		template<class T>
		void set_instance(T* inst, void* addr) {
			if (this->m_block != nullptr) {
				this->m_block->m_instance = inst;
				this->m_block->m_address = addr;
			} else {
				this->m_block = (control_block*)MemoryHelper::CreateObject<control_block>(inst, addr);
			}
		}
	public:
		void release() {
			if (this->m_block != nullptr) {
				if (this->m_block->m_ref_counter.fetch_sub(1) == 1) {
					MemoryHelper::Finalize<control_block>(this->m_block);
					MemoryHelper::Free(this->m_block);
				}
				this->m_block = nullptr;
			}
		}
		reference(const reference& other) : m_block(nullptr) {
			if (!other.empty()) {
				this->m_block = other.m_block;
				this->m_block->m_ref_counter.fetch_add(1);
			}
		}
		reference(reference&& other) noexcept : m_block(nullptr) {
			if (!other.empty()) {
				this->m_block = other.m_block;
				other.m_block = nullptr;
			}
		}
		reference& operator=(const reference& other) {
			this->release();
			if (!other.empty()) {
				this->m_block = other.m_block;
				this->m_block->m_ref_counter.fetch_add(1);
			}
			return *this;
		}
		reference& operator=(reference&& other) noexcept {
			this->release();
			if (!other.empty()) {
				this->m_block = other.m_block;
				other.m_block = nullptr;
			}
			return *this;
		}
		virtual ~reference() {
			this->release();
		}
		bool is_uniqe() const {
			if (this->m_block == nullptr) {
				return true;
			} else {
				return this->m_block->m_ref_counter.load() == 1;
			}
		}
		bool empty() const {
			return this->m_block == nullptr;
		}
		void* get_address() const {
			if (this->m_block != nullptr)
				return this->m_block->m_address;
			return nullptr;
		}
		void* get_instance_ptr() const {
			if (this->m_block != nullptr)
				return this->m_block->m_instance;
			return nullptr;
		}
		type_info* get_type_info() const {
			if (this->m_block != nullptr)
				return this->m_block->m_type_info;
			return nullptr;
		}
		reference() : m_block(nullptr) {}
		reference(const cyh::details::null_reference& _null) : m_block(nullptr) {}
		reference& operator=(const cyh::details::null_reference& _null) {
			this->release();
			return *this;
		}
		template<class T>
		reference_<T> as_ref() const;
		bool operator == (const cyh::details::null_reference&) const {
			return this->empty();
		}
	};

	template<class T>
	class reference_ final : public reference {
		template<class U>
		friend class reference_;
		friend cyh::details::unsafe_opt;
		void* m_instance{};
		void release_t() {
			this->release();
			this->m_instance = nullptr;
		}
		reference_(T* pinst, void* addr) : reference(pinst), m_instance(this->get_instance_ptr()) {}
	public:
		~reference_() { this->release_t(); }
		reference_() : reference((T*)nullptr), m_instance(this->get_instance_ptr()) {}
		reference_(T&& instance) : reference((T*)MemoryHelper::CreateObject<T>(std::move(instance))), m_instance(this->get_instance_ptr()) { }
		reference_(T* ptr, void(*cust_dealloc)(void*)) : reference(ptr, cust_dealloc), m_instance(this->get_instance_ptr()) {}
		reference_(const reference_<T>& other) : reference((T*)nullptr) {
			this->m_instance = other.m_instance;
			reference& rthis = *this;
			const reference& rthat = *get_ptr(other);
			rthis = rthat;
		}
		reference_(reference_<T>&& other) noexcept : reference((T*)nullptr) {
			this->m_instance = other.m_instance;
			reference& rthis = *this;
			reference& rthat = *get_ptr(other);
			rthis = std::move(rthat);
		}
		reference_<T>& operator = (const reference_<T>& other) {
			this->m_instance = other.m_instance;
			reference& rthis = *this;
			const reference& rthat = *get_ptr(other);
			rthis = rthat;
			return *this;
		}
		reference_<T>& operator = (reference_<T>&& other) noexcept {
			this->m_instance = other.m_instance;
			reference& rthis = *this;
			const reference& rthat = *get_ptr(other);
			rthis = rthat;
			return *this;
		}
		reference_(const cyh::details::null_reference& _null) : reference((T*)nullptr), m_instance(this->get_instance_ptr()) { }
		reference_<T>& operator = (const cyh::details::null_reference& _null) {
			this->release_t();
			return *this;
		}
		T* operator->() const {
			return (T*)this->m_instance;
		}
		T& operator*() const {
			return *(T*)this->m_instance;
		}
		operator T* () const {
			return (T*)this->m_instance;
		}

		bool operator == (const T* _ptr) const {
			return this->m_instance == (void*)_ptr;
		}

		template<class... Args>
		void create_new(Args&&... args) {
			this->release_t();
			this->set_instance((T*)MemoryHelper::CreateObject<T>(std::forward<Args>(args)...));
			this->m_instance = this->get_instance_ptr();
		}
		template<class... Args>
		void new_if_empty(Args&&... args) {
			if (this->empty()) {
				this->create_new(std::forward<Args>(args)...);
			}
		}

		template<class U>
		reference_(U&& instance) requires (std::is_base_of_v<T, U>) : reference((T*)nullptr) {
			void* addr = nullptr;
			T* inst = (T*)MemoryHelper::CreateAbstractBaseObject<T, U>(&addr, std::move(instance));
			this->set_instance(inst, addr);
			this->m_instance = inst;
		}
		template<class U>
		reference_(const reference_<U>& other) requires (std::is_base_of_v<T, U>) : reference((T*)nullptr) {
			if (!other.empty()) {
				T* inst_t = (T*)other;
				if (inst_t != nullptr) {
					this->m_instance = inst_t;
					reference& rthis = *this;
					const reference& rthat = *get_ptr(other);
					rthis = rthat;
				} else {
					throw cyh::exception::invalid_operation_exception();
				}
			}
		}
		template<class U>
		reference_<T>& operator = (const reference_<U>& other) requires (std::is_base_of_v<T, U>) {
			if (other.empty()) {
				this->release_t();
			} else {
				T* inst_t = (T*)other;
				if (inst_t != nullptr) {
					this->release_t();
					this->m_instance = inst_t;
					reference& rthis = *this;
					const reference& rthat = *get_ptr(other);
					rthis = rthat;
				} else {
					throw cyh::exception::invalid_operation_exception();
				}
			}
			return *this;
		}
		template<class U, class... Args>
		void create_new(Args&&... args) requires(std::is_base_of_v<T, U>) {
			this->release_t();
			void* addr = nullptr;
			this->m_instance = MemoryHelper::CreateAbstractBaseObject<T, U>(&addr, std::forward<Args>(args)...);
			this->set_instance((T*)this->m_instance, addr);
		}

		template<class X>
		X* get() const requires(std::is_same_v<void, X>) {
			return (X*)this->m_instance;
		}

		template<class X>
		X* get() const requires(std::is_same_v<T, X> && !std::is_same_v<void, X>) {
			return (X*)this->m_instance;
		}

		template<class X>
		X* get() const requires(!cyh::type::is_primitive_type_v<T> && !cyh::type::is_primitive_type_v<X> && !std::is_same_v<T, X> && !std::is_same_v<void, X>) {
			return dynamic_cast<X*>((T*)this->m_instance);
		}

		template<class X>
		X* get() const requires(cyh::type::is_primitive_type_v<T> || cyh::type::is_primitive_type_v<X> && !std::is_same_v<T, X> && !std::is_same_v<void, X>) {
			return nullptr;
		}

		template<class X>
		reference_<X> as() const requires(std::is_base_of_v<X, T>) {
			return *this;
		}

		template<class X>
		reference_<X> as() const requires(!std::is_base_of_v<X,T>) {
			reference_<X> result{};
			X* inst = this->get<X>();
			if (inst != nullptr) {
				reference& result_base = result;
				const reference& this_base = *this;
				result_base = this_base;
				result.m_instance = inst;
			}
			return result;
		}

		template<class Y>
		operator Y* () const {
			return this->get<Y>();
		}
	};

	template<class T>
	using ref = reference_<T>;

	namespace details {
		struct unsafe_opt {
			template<class T>
			static ref<T> directly_create(T* inst, void* addr) {
				return ref<T>(inst, addr);
			}
		};
	};

	template<class T>
	reference_<T> make_ref(T&& _val) requires(std::is_move_constructible_v<T>) {
		return { std::move(_val) };
	}

	template<class T>
	reference_<T> make_ref(const T& _val) requires(std::is_copy_constructible_v<std::remove_cvref_t<T>>) {
		reference_<T> result{};
		result.create_new(_val);
		return result;
	}

	template<class T>
	bool set_value_if_not_null(ref<T> _ref, T _val) requires(std::is_copy_assignable_v<std::decay_t<T>>) {
		if (_ref.empty()) 
			return false;
		*_ref = _val;
		return true;
	}

	template<class T>
	reference_<T> reference::as_ref() const {
		reference_<T> result{};
		auto ptr = dynamic_cast<reference_<T>*>((reference*)this);
		if (ptr != nullptr)
			result = *ptr;
		return result;
	}

	template<class T>
	T value_or(const reference_<T>& _ref, const T& _if_null = {}) {
		return _ref ? *_ref : _if_null;
	}
};
