/*!
 * \file	include\dcm_pool\_object_in_pool_imp.h.
 *
 * \brief		Implement the ObjectInPool template class.
 * \license		MIT
 * \autor		Ronen Ness.
 * \since		2018
 */

#include "object_in_pool.h"

#ifndef __OBJECT_IN_POOL_IMP__
#define __OBJECT_IN_POOL_IMP__

namespace dcm_pool
{
	namespace _internal
	{

		template <typename T>
		ObjectInPool<T>::ObjectInPool(ObjectId id) : _id(id), _is_used(false)
		{
		}

		template <typename T>
		ObjectInPool<T>::ObjectInPool(const ObjectInPool<T>& other)
		{
			(*this) = other;
		}

		template <typename T>
		ObjectId ObjectInPool<T>::get_id() const
		{
			return _id;
		}

		template <typename T>
		void ObjectInPool<T>::set_id(ObjectId id)
		{
			_id = id;
		}

		template <typename T>
		T& ObjectInPool<T>::get_object()
		{
			return _obj;
		}

		template <typename T>
		bool ObjectInPool<T>::is_used() const
		{
			return _is_used;
		}

		template <typename T>
		void ObjectInPool<T>::set_is_used(bool is_used)
		{
			_is_used = is_used;
		}

		template <typename T>
		ObjectInPool<T>& ObjectInPool<T>::operator=(const ObjectInPool<T>& other)
		{
			_obj = other._obj;
			_is_used = other._is_used;
			_id = other._id;
			return *this;
		}

		template <typename T>
		ObjectInPool<T>& ObjectInPool<T>::operator=(ObjectInPool<T>&& other)
		{
			_obj = std::move(other._obj);
			_is_used = other._is_used;
			_id = other._id;
			other._is_used = false;
			other._id = ObjectPoolMaxIndex;
			return *this;
		}
	}
}

#endif