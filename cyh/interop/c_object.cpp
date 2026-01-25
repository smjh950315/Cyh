#include "c_object.hpp"

namespace cyh::interop
{
	size_t c_object::type_size() const
	{
		return this->m_typesize;
	}

	size_t c_object::hash_code() const
	{
		return this->m_typeinfo->hash_code();
	}

	const char* c_object::type_name() const
	{
		return this->m_typeinfo->name();
	}
};

