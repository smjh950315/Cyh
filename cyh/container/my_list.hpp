#pragma once
#include "container_.hpp"
#include "../memory_helper.hpp"
#include "../reference.hpp"
namespace cyh::container {
	namespace details {
		template<class T>
		struct list_node {
			using MemHelper = cyh::MemoryHelper;
			using node = list_node<T>;
			void* m_address;
			T* m_instance;
			list_node<T>* prev;
			list_node<T>* next;
			list_node(T* instance, void* address) : m_instance(instance), m_address(address), prev(nullptr), next(nullptr) {}
			~list_node() {
				if (this->m_instance != nullptr) {
					MemHelper::ReleaseAbstract<T>(this->m_instance, this->m_address);
				}
			}
		};

		template<class T>
		struct list_iterator {
			using iterator = list_iterator<T>;
			using value_type = std::decay_t<T>;
			using reference = value_type&;
			using pointer = value_type*;
			using node = list_node<T>;
			node* m_ptr{};
			list_iterator() {}
			list_iterator(const node* _ptr) : m_ptr((node*)_ptr) {}
			pointer operator->() {
				if (!this->m_ptr)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				else
					return this->m_ptr->m_instance;
			}
			constexpr reference operator*() {
				if (!this->m_ptr)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				if (!this->m_ptr->m_instance)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				else
					return *(this->m_ptr->m_instance);
			}
			constexpr iterator& operator++() {
				this->m_ptr = this->m_ptr->next;
				return *this;
			}
			constexpr iterator& operator--() {
				this->m_ptr = this->m_ptr->prev;
				return *this;
			}
			constexpr iterator operator++(int) {
				iterator temp = *this;
				++(*this);
				return temp;
			}
			constexpr iterator operator--(int) {
				iterator temp = *this;
				--(*this);
				return temp;
			}
			constexpr bool operator == (const iterator& _other) const {
				return this->m_ptr == _other.m_ptr;
			}
		};

		template<class T>
		struct list_reverse_iterator {
			using iterator = list_reverse_iterator<T>;
			using value_type = std::decay_t<T>;
			using reference = value_type&;
			using pointer = value_type*;
			using node = list_node<T>;
			node* m_ptr{};
			list_reverse_iterator() {}
			list_reverse_iterator(const node* _ptr) : m_ptr((node*)_ptr) {}
			pointer operator->() {
				if (!this->m_ptr)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				else
					return this->m_ptr->m_instance;
			}
			constexpr reference operator*() {
				if (!this->m_ptr)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				if (!this->m_ptr->m_instance)
					throw cyh::exception::null_pointer_exception().src(__POSITION__);
				else
					return *(this->m_ptr->m_instance);
			}
			constexpr iterator& operator++() {
				this->m_ptr = this->m_ptr->prev;
				return *this;
			}
			constexpr iterator& operator--() {
				this->m_ptr = this->m_ptr->next;
				return *this;
			}
			constexpr iterator operator++(int) {
				iterator temp = *this;
				++(*this);
				return temp;
			}
			constexpr iterator operator--(int) {
				iterator temp = *this;
				--(*this);
				return temp;
			}
			constexpr bool operator == (const iterator& _other) const {
				return this->m_ptr == _other.m_ptr;
			}
		};
	};

	template<class T>
	class mylist {
	public:
		using node = cyh::container::details::list_node<T>;
		using value_type = T;
		using iterator = cyh::container::details::list_iterator<T>;
		using reverse_iterator = cyh::container::details::list_reverse_iterator<T>;
	private:
		static node* get_first_node(node* _curr) {
			auto iter = _curr;
			while (iter->prev)
				iter = iter->prev;
			return iter;
		}
		static node* get_last_node(node* _curr) {
			auto iter = _curr;
			while (iter->next)
				iter = iter->next;
			return iter;
		}

		static void emplate_node_back(node* dst, node* src) {
			auto last = get_last_node(dst);
			last->next = src;
			src->prev = last;
		}
		static void emplate_node_front(node* dst, node* src) {
			auto first = get_first_node(dst);
			first->prev = src;
			src->next = first;
		}

		static node* remove_node(node* target) {
			node* prev = target->prev;
			target->prev = nullptr;
			node* next = target->next;
			target->next = nullptr;
			if (prev != nullptr) { prev->next = next; }
			if (next != nullptr) { next->prev = prev; }
			return target;
		}
		// return: new last node
		static node* remove_forward(node* target) {
			if (target == nullptr) { return nullptr; }
			node* newlast = target->prev;
			if (newlast != nullptr) {
				newlast->next = nullptr;
			}
			node* current = target;
			while (current->next) {
				current = current->next;
				MemoryHelper::ReleaseAbstract<node>(current->prev);
			}
			MemoryHelper::ReleaseAbstract<node>(current);
			return newlast;
		}

		template<class... Args>
		static node* create_node(Args&&...args) {
			T* inst = (T*)MemoryHelper::CreateObject<T>(std::forward<Args>(args)...);
			return (node*)MemoryHelper::CreateObject<node>(inst, inst);
		}
		template<class U, class... Args>
		static node* create_node(Args&&...args) requires(std::is_base_of_v<T, U>) {
			void* addr = nullptr;
			T* inst = (T*)MemoryHelper::CreateAbstractBaseObject<T, U, Args...>(&addr, std::forward<Args>(args)...);
			return (node*)MemoryHelper::CreateObject<node>(inst, addr);
		}
		template<class U, class... Args>
		static node* create_node_by_move(Args&&...args) requires(std::is_base_of_v<T, U>) {
			void* addr = nullptr;
			T* inst = (T*)MemoryHelper::CreateAbstractBaseObject<T, U, Args...>(&addr, std::forward<Args>(args)...);
			return (node*)MemoryHelper::CreateObject<node>(inst, addr);
		}

		node* m_first{};
		node* m_last{};
		size_t m_count{};
		void _emplace_back(node* _node) {
			if (this->m_first == nullptr) {
				this->m_first = _node;
				this->m_last = _node;
				this->m_count = 1;
			} else {
				emplate_node_back(this->m_last, _node);
				this->m_last = _node;
				this->m_count++;
			}
		}
		node* _pop_back() {
			node* ret = nullptr;
			if (this->m_count == 0) { return nullptr; }
			if (this->m_count == 1) {
				ret = this->m_first;
				this->m_first = nullptr;
				this->m_last = nullptr;
				this->m_count = 0;
			} else {
				auto new_last = this->m_last->prev;
				ret = remove_node(this->m_last);
				this->m_last = new_last;
				this->m_count -= 1;
			}
			return ret;
		}
		void _clear_all() {
			if (this->m_first != nullptr) {
				remove_forward(this->m_first);
				this->m_first = nullptr;
				this->m_last = nullptr;
				this->m_count = 0;
			}
		}

		void _push_backs() { }
		template<class Item, class ... Items>
		void _push_backs(Item&& item, Items&& ... items) {
			this->push_back(std::forward<Item>(item));
			this->_push_backs(std::forward<Items>(items)...);
		}
	public:
		virtual ~mylist() { this->_clear_all(); }
		iterator begin() { return this->m_first; }
		iterator end() { return nullptr; }
		const iterator begin() const { return this->m_first; }
		const iterator end() const { return nullptr; }
		reverse_iterator rbegin() { return this->m_last; }
		reverse_iterator rend() { return nullptr; }
		const reverse_iterator rbegin() const { return this->m_last; }
		const reverse_iterator rend() const { return nullptr; }
		size_t size() const { return this->m_count; }
		T& operator[](size_t index) {
			if (index >= this->m_count) {
				throw cyh::exception::out_of_range_exception().src(__POSITION__);
			}
			auto iter = this->m_first;
			for (size_t i = 0; i < index; ++i) {
				iter = iter->next;
			}
			return *(iter->m_instance);
		}
		bool first(T** pfirst) {
			if (this->m_first == nullptr) { return false; }
			if (this->m_first->m_instance == nullptr) { return false; }
			*pfirst = this->m_first->m_instance;
			return true;
		}
		bool last(T** plast) {
			if (this->m_last == nullptr) { return false; }
			if (this->m_last->m_instance == nullptr) { return false; }
			*plast = this->m_last->m_instance;
			return true;
		}

		mylist() {}
		template<class ... Items>
		mylist(Items&& ... items) requires(std::is_class_v<T>&& cyh::type::is_base_multipule_v<T, Items...>) {
			this->_push_backs(std::forward<Items>(items)...);
		}
		template<class ... Items>
		mylist(Items&& ... items) requires(!std::is_class_v<T>&& cyh::type::is_assignable_multiple_v<T, Items...>) {
			this->_push_backs(std::forward<Items>(items)...);
		}
		mylist(std::initializer_list<T> vals) requires(!std::is_abstract_v<T>) {
			for (auto& val : vals) {
				this->push_back(val);
			}
		}
		mylist(const mylist<T>& other) {
			for (const auto& t : other) {
				this->push_back(t);
			}
		}
		mylist(mylist<T>&& other) noexcept {
			this->m_first = other.m_first;
			this->m_last = other.m_last;
			this->m_count = other.m_count;
			other.m_first = nullptr;
			other.m_last = nullptr;
			other.m_count = 0;
		}
		mylist<T>& operator=(const mylist<T>& other) {
			this->clear();
			for (const auto& t : other) {
				this->push_back(t);
			}
			return *this;
		}
		mylist<T>& operator=(mylist<T>&& other) noexcept {
			this->clear();
			this->m_first = other.m_first;
			this->m_last = other.m_last;
			this->m_count = other.m_count;
			other.m_first = nullptr;
			other.m_last = nullptr;
			other.m_count = 0;
			return *this;
		}

		void clear() { this->_clear_all(); }
		T* find(std::function<bool(const T*)> _func) const {
			auto pThis = (mylist<T>*)this;
			for (auto& t : *pThis) {
				if (_func(&t))return &t;
			}
			return nullptr;
		}
		
		bool remove_first(std::function<bool(const T*)> _func) {
			if (this->m_count == 0) return false;
			if (this->m_count == 1) {
				if (_func(this->m_first->m_instance)) {
					this->clear();
					return true;
				}
				return false;
			} else {
				if (_func(this->m_first->m_instance)) {
					this->m_first = this->m_first->next;
					MemoryHelper::ReleaseAbstract<node>(this->m_first->prev);
					this->m_first->prev = nullptr;
					this->m_count--;
					return true;
				}
				if (_func(this->m_last->m_instance)) {
					this->m_last = this->m_last->prev;
					MemoryHelper::ReleaseAbstract<node>(this->m_last->next);
					this->m_last->next = nullptr;
					this->m_count--;
					return true;
				}
				node* temp = this->m_first->next;
				while (temp && temp != this->m_last) {
					if (_func(temp->m_instance)) {
						remove_node(temp);
						MemoryHelper::ReleaseAbstract<node>(temp);
						this->m_count--;
						return true;
					}
					temp = temp->next;
				}
				return false;
			}
		}
		size_t remove_all(std::function<bool(const T*)> _func) {
			if (this->m_count == 0) { return 0; }
			if (this->m_count == 1) {
				if (_func(this->m_first->m_instance)) {
					this->clear();
					return 1;
				}
				return 0;
			} else {
				bool remove_first = _func(this->m_first->m_instance);
				bool remove_last = _func(this->m_last->m_instance);
				std::vector<node*> remove_nodes;
				node* temp = this->m_first->next;
				while (temp && temp != this->m_last) {
					if (_func(temp->m_instance)) {
						remove_nodes.push_back(temp);
					}
					temp = temp->next;
				}
				size_t remove_count = remove_nodes.size();
				for (auto& _node : remove_nodes) {
					remove_node(_node);
					MemoryHelper::ReleaseAbstract<node>(_node);
				}
				if (remove_first) {
					if (this->m_first->next == nullptr) {
						size_t _count = this->size();
						this->clear();
						return _count;
					}
					this->m_first = this->m_first->next;
					MemoryHelper::ReleaseAbstract<node>(this->m_first->prev);
					this->m_first->prev = nullptr;
					remove_count++;
				}
				if (remove_last) {
					if (this->m_last->prev ==nullptr) {
						size_t _count = this->size();
						this->clear();
						return _count;
					} 
					this->m_last = this->m_last->prev;
					MemoryHelper::ReleaseAbstract<node>(this->m_last->next);
					this->m_last->next = nullptr;
					remove_count++;
				}
				this->m_count -= remove_count;
				return remove_count;
			}
		}

		void push_back(const std::decay_t<T>& val) {
			auto pnode = create_node(val);
			if (pnode != nullptr) {
				this->_emplace_back(pnode);
			}
		}
		void push_back(std::decay_t<T>&& val) {
			auto pnode = create_node(std::forward<T>(val));
			if (pnode != nullptr) {
				this->_emplace_back(pnode);
			}
		}

		template<class U>
		void push_back(const std::decay_t<U>& val) requires(std::is_base_of_v<T, U>) {
			auto pnode = create_node<U>(val);
			if (pnode != nullptr) {
				this->_emplace_back(pnode);
			}
		}
		template<class U>
		void push_back(std::decay_t<U>&& val) requires(std::is_base_of_v<T, U>) {
			auto pnode = create_node_by_move<U>(std::move(val));
			if (pnode != nullptr) {
				this->_emplace_back(pnode);
			}
		}
		ref<T> pop_back() {
			node* old_last = this->_pop_back();
			if (old_last != nullptr) {
				ref<T> temp = cyh::details::unsafe_opt::directly_create<T>(old_last->m_instance, old_last->m_address);
				old_last->m_address = nullptr;
				old_last->m_instance = nullptr;
				MemoryHelper::ReleaseAbstract<node>(old_last);
				return temp;
			} else {
				return null;
			}
		}

		void emplace_back(mylist<T>&& other) {
			if (!other.size()) { return; }
			if (!this->size()) {
				*this = std::move(other);
			} else {
				this->m_last->next = other.m_first;
				other.m_first->prev = this->m_last;
				other.m_first = nullptr;
				this->m_last = other.m_last;
				other->m_last = nullptr;
				this->m_count += other.m_count;
				other->m_count = 0;
			}
		}

		mylist<T> get_segment(size_t begin, size_t count) {
			if (!this->size() || this->size() <= begin) { return {}; }
			mylist<T> result;
			if (begin == 0 && count == this->size()) {
				result = std::move(*this);
				return result;
			}
			size_t remaining = this->size() - begin;
			size_t max_count = remaining < count ? remaining : count;
			node* iter = this->m_first;
			size_t current_index = 0;
			if (begin != 0 && remaining != max_count) {
				size_t max_index = begin + max_count;
				node* sub_begin_node = nullptr;
				while (current_index < max_index) {
					if (current_index == begin) {
						sub_begin_node = iter;
					}
					iter = iter->next;
					++current_index;
				}
				result.m_first = sub_begin_node;
				result.m_last = iter->prev;

				iter->prev = sub_begin_node->prev;
				sub_begin_node->prev->next = iter;

				result.m_first->prev = nullptr;
				result.m_last->next = nullptr;
			} else if (begin == 0 && remaining != max_count) {
				size_t max_index = begin + max_count;
				while (current_index < max_index) {
					iter = iter->next;
					++current_index;
				}
				result.m_first = this->m_first;
				result.m_last = iter->prev;
				this->m_first = iter;
				result.m_last->next = nullptr;
				this->m_first->prev = nullptr;
			} else if (begin != 0 && remaining == max_count) {

				while (current_index < begin) {
					iter = iter->next;
					++current_index;
				}
				result.m_first = iter;
				result.m_last = this->m_last;
				this->m_last = iter->prev;
				result.m_first->prev = nullptr;
				this->m_last->next = nullptr;
			}
			result.m_count = max_count;
			this->m_count -= max_count;
			return result;
		}
	};
};
