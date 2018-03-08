/*!
* \file	include\dcm_pool\_object_ptr_imp.h.
*
* \brief		Implement the ObjectPtr template class.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/

#include "object_ptr.h"

#ifndef __OBJECT_PTR_IMP__
#define __OBJECT_PTR_IMP__
#include "objects_pool.h"

namespace dcm_pool
{
	template <typename T>
	ObjectPtr<T>::ObjectPtr(ObjectsPool<T>* pool, ObjectId id) : 
		_pool(pool), 
		_id(id),
		_pool_defrag_version(-1)
	{
	}

	template <typename T>
	ObjectId ObjectPtr<T>::_get_id() const
	{
		return _id;
	}

	template <typename T>
	T& ObjectPtr<T>::operator*(void)
	{
		// check if we have a valid cached pointer to return
		if (_pool_defrag_version == _pool->_get_defrags_count())
			return *_cached_ptr;

		// if not get the pointer and cache it
		T* ret = &(_pool->_get_object(_id));
		_cached_ptr = ret;
		_pool_defrag_version = _pool->_get_defrags_count();

		// return pointer
		return *ret;
	}

	template <typename T>
	T* ObjectPtr<T>::operator->(void)
	{
		return &(this->operator*());
	}
}

#endif