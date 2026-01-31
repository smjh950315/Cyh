#include "ndarray.hpp"
#include <cyh/buffer.hpp>
template<class T>
using vec = std::vector<T>;
namespace cyh::container::details
{
	size_t cross(const vec<size_t>& _vec)
	{
		size_t res = 1;
		for (const auto& v : _vec)
		{
			res *= v;
		}
		return res;
	}
	bool is_equal(const vec<size_t>& l, const vec<size_t>& r)
	{
		if (l.size() != r.size()) return false;
		auto begl = l.begin();
		auto begr = r.begin();
		auto endr = r.end();
		while (begr != endr)
		{
			if (*begr++ != *begl++)
				return false;
		}
		return true;
	}
	size_t cross(const myvec<size_t>& _vec)
	{
		size_t res = 1;
		for (const auto& v : _vec)
		{
			res *= v;
		}
		return res;
	}
	bool is_equal(const myvec<size_t>& l, const myvec<size_t>& r)
	{
		if (l.size() != r.size()) return false;
		auto begl = l.begin();
		auto begr = r.begin();
		auto endr = r.end();
		while (begr != endr)
		{
			if (*begr++ != *begl++)
				return false;
		}
		return true;
	}
	void nditerator::copy_to_left(nditerator* l, const nditerator* r)
	{
		l->data = r->data;
		l->dim = r->dim;
		l->pos = r->pos;
		l->range = r->range;
		l->shape_r = r->shape_r;
		l->typesize = r->typesize;
	}
	static void move_to_left(nditerator* l, nditerator* r)
	{
		l->data = std::move(r->data);
		l->dim = std::move(r->dim);
		l->pos = std::move(r->pos);
		l->range = std::move(r->range);
		l->shape_r = std::move(r->shape_r);
		l->typesize = std::move(r->typesize);
	}
	void nditerator::move_next(nditerator* it)
	{
		auto itpos = it->pos.rbegin();
		auto itrange = it->range.rbegin();
		auto itposE = it->pos.rend();
		size_t last = 0;
		while (itpos != itposE)
		{
			if (*itpos < itrange->y) {
				(*itpos) = (*itpos) + 1;
				break;
			} else {
				(*itpos) = itrange->x;
			}
			++itpos; ++itrange;
		}
		if (itpos == itposE)
		{
			--itpos;
			if (*itpos == it->range[0].x)
				*itpos = it->range[0].y + 1;
		}
	}
	static size_t get_offset(nditerator* it)
	{
		auto itPos = it->pos.rbegin();
		auto itPosE = it->pos.rend();
		auto& dataShape = it->shape_r;
		auto itDS = dataShape.rbegin();
		size_t ratio = 1;
		size_t offset = 0;
		while (itPos != itPosE)
		{
			ratio *= *itDS++;
			offset += (ratio * (*itPos));
			itPos++;
		}
		return offset * (it->typesize);
	}
	nditerator::nditerator(const nditerator& _other)
	{
		copy_to_left(this, &_other);
	}
	nditerator::nditerator(nditerator&& _other) noexcept
	{
		move_to_left(this, &_other);
	}
	nditerator& nditerator::operator=(const nditerator& _other)
	{
		copy_to_left(this, &_other);
		return *this;
	}
	nditerator& nditerator::operator=(nditerator&& _other) noexcept
	{
		move_to_left(this, &_other);
		return *this;
	}
	nditerator::nditerator(const ref<array_data>& d, const vec<MyVec2<size_t>>& r, size_t ts)
	{
		this->data = d; this->range = r;
		for (const auto& _r : r)
		{
			this->pos.push_back(_r.x);
		}
		this->dim = r.size();
		this->shape_r = d->shape;
		this->shape_r[shape_r.size() - 1] = 1;
		this->typesize = ts;
	}
	nditerator::~nditerator()
	{
	}
	const nditerator::value_type& nditerator::operator*() const
	{
		return *(this->operator->());
	}
	nditerator::value_type& nditerator::operator*()
	{
		return *(this->operator->());
	}
	const nditerator::pointer nditerator::operator->() const
	{
		return ((iterator*)this)->operator->();
	}
	nditerator::pointer nditerator::operator->()
	{
		return data->data.data() + get_offset(this);
	}
	nditerator::iterator& nditerator::operator++()
	{
		move_next(this);
		return *this;
	}
	nditerator::iterator nditerator::operator++(int)
	{
		iterator ret = *this;
		move_next(this);
		return ret;
	}
	bool nditerator::operator == (const iterator& rhs) const
	{
		return is_equal(pos, rhs.pos);
	}
	bool nditerator::operator != (const iterator& rhs) const
	{
		return !(*this == rhs);
	}
}
#include <string>
namespace cyh::container
{
	using namespace details;
	static void copy_to_left(ndarray* l, const ndarray* r) {
		l->m_data = r->m_data;
		l->m_range = r->m_range;
		l->m_typesize = r->m_typesize;
	}
	static void move_to_left(ndarray* l, ndarray* r) {
		l->m_data = std::move(r->m_data);
		l->m_range = std::move(r->m_range);
		l->m_typesize = std::move(r->m_typesize);
		r->m_typesize = 1;
	}
	ndarray::ndarray()
	{
	}
	ndarray::ndarray(const ndarray& _other)
	{
		copy_to_left(this, &_other);
	}
	ndarray::ndarray(ndarray&& _other) noexcept
	{
		move_to_left(this, &_other);
	}
	ndarray& ndarray::operator=(const ndarray& _other)
	{
		copy_to_left(this, &_other);
		return *this;
	}
	ndarray& ndarray::operator=(ndarray&& _other) noexcept
	{
		move_to_left(this, &_other);
		return *this;
	}
	size_t ndarray::typesize() const {
		return m_data ? this->m_typesize : 0;
	}
	size_t ndarray::dims() const {
		return m_data ? m_data->shape.size() : 0;
	}
	vec<size_t> ndarray::shape() const {
		if (this->m_data.empty())
			return {};
		vec<size_t> ret{};
		for (auto& r : m_range) {
			ret.push_back(r.y - r.x + 1);
		}
		return ret;
	}
	void ndarray::resize(const vec<size_t>& _shape) {
		if (m_data.empty() || !m_data.is_uniqe())
			m_data.create_new();
		m_data->resize(_shape, this->typesize());
		m_range.clear();
		for (const auto& s : _shape)
		{
			m_range.push_back({ size_t(), s - 1 });
		}
	}
	bool ndarray::crop(ndarray& out, const vec<size_t>& beg, const vec<size_t>& end)
	{
		if (beg.size() != end.size())
			return false;
		size_t d = this->dims();
		if (d != beg.size())
			return false;
		auto myShape = shape();
		vec<MyVec2<size_t>> outRange{};
		for (size_t i = 0; i < d; ++i) {
			auto myBegin = m_range[i].x;
			auto l = myBegin + std::clamp(beg[i], size_t{}, myShape[i] - 1);
			auto r = myBegin + std::clamp(end[i], size_t{}, myShape[i] - 1);
			//if (l == r) return false;
			outRange.push_back({ std::min(l, r), std::max(l, r) });
		}
		out.m_data = this->m_data;
		out.m_range = outRange;
		out.m_typesize = this->typesize();
		return true;
	}
	ndarray ndarray::crop(const vec<size_t>& beg, const vec<size_t>& end)
	{
		ndarray ret{};
		this->crop(ret, beg, end);
		return ret;
	}
	ndarray::iterator ndarray::begin()
	{
		return iterator(m_data, m_range, this->typesize());
	}
	ndarray::iterator ndarray::end()
	{
		auto it = iterator(m_data, m_range, this->typesize());
		auto itItPos = it.pos.begin();
		auto itRange = m_range.begin();
		auto itRangeE = m_range.end();
		while (itRange != itRangeE)
		{
			(*itItPos) = itRange->y;
			++itRange;
			++itItPos;
		}
		++it;
		return it;
	}
	const ndarray::iterator ndarray::begin() const
	{
		return ((ndarray*)this)->begin();
	}
	const ndarray::iterator ndarray::end() const
	{
		return ((ndarray*)this)->end();
	}
	static void copy_data_to_left(void* l, void* r, size_t sz) 
	{		
		memcpy(l, r, sz);
	}
	void ndarray::copy_to(const ndarray& _other, func_op_data _convert)
	{
		auto opdataSize = std::min(this->typesize(), _other.typesize());
		if (_convert)
			this->operate_with_other(_other, _convert);
		else 
			this->operate_with_other(_other,
									 [=](void* l, void* r)
									 {
										 copy_data_to_left(l, r, opdataSize);
									 });
	}
	void ndarray::operate_with_other(const ndarray& _other, const std::function<void(void*, void*)>& _op) {
		if (_op == nullptr)
			return;
		auto opdataSize = std::min(this->typesize(), _other.typesize());
		if (opdataSize == 0) return;
		auto itbegThis = this->begin();
		auto itendThis = this->end();
		auto itbegThat = _other.begin();
		auto itendThat = _other.end();
		while (itbegThis != itendThis && itbegThat != itendThat)
		{
			auto& dataThis = *itbegThis;
			auto& dataThat = *itbegThat;
			_op(&dataThat, &dataThis);
			++itbegThis;
			++itbegThat;
		}
	}
	static void print_child(ndarray::iterator& it, size_t* child_count, size_t current_layer, size_t last_layer, ndarray::func_print_data _func) {
		for (size_t i = 0; i < child_count[current_layer]; ++i) {
			if (current_layer < last_layer) {
				std::cout << "[";
				print_child(it, child_count, current_layer + 1, last_layer, _func);
				std::cout << "]";
				if (i < child_count[current_layer] - 1)
					std::cout << ",";
			} else {
				auto& pdata = *it;
				_func(&pdata);
				if (i < child_count[current_layer] - 1)
					std::cout << ",";
				++it;
			}			
		}
	}
	void ndarray::print_format(func_print_data _print)
	{
		auto beg = this->begin();
		auto _shape = this->shape();
		if (_shape.size() == 0) {
			std::cout << "[]";
			return;
		}		
		auto lastLayer = _shape.size() - 1;
		auto layerPtr = _shape.data();
		print_child(beg, layerPtr, 0, lastLayer, _print);
	}
};

