/*!
* \file	include\dcm_pool\_object_pool_imp.h.
*
* \brief		Implement the ObjectsPool template class.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/

#include "exceptions.h"
#include "objects_pool.h"


#ifndef __OBJECT_POOL_IMP__
#define __OBJECT_POOL_IMP__


namespace dcm_pool
{
	template <typename T>
	ObjectsPool<T>::ObjectsPool(size_t max_size, size_t reserve, size_t shrink_threshold, DefragModes defrag_mode) :
		_max_size(max_size),
		_allocated_objects_count(0),
		_next_object_id(0),
		_max_used_index_in_vector(0),
		_shrink_pool_threshold(shrink_threshold),
		_defrag_mode(defrag_mode),
		_defrags_count(0),
		_holes(_objects)
	{
		// pre-alloc desired size
		if (reserve)
		{
			_objects.reserve(reserve);
		}
	}

	template <typename T>
	typename ObjectsPool<T>::Ptr ObjectsPool<T>::Alloc()
	{
		// make sure didn't exceed pool limit
		if (_max_size && _allocated_objects_count >= _max_size)
		{
			throw ExceededPoolLimit();
		}
		
		// will hole the index to allocate from
		std::size_t alloc_index;

		// do we have a hole to fill? if so, use it
		if (_holes.size())
		{
			// get index to alloc on and remove from holes vector
			alloc_index = _holes.pop_back();

			// return the new object pointer
			return AssignObject(alloc_index);
		}

		// do we have unused objects at the end of the vector? fill them
		if (_max_used_index_in_vector + 1 < _objects.size())
		{
			return AssignObject(_max_used_index_in_vector + 1);
		}

		// if we got here it means we don't have any hole to fill, and must allocate a new object in pool's vector
		auto index = _objects.size();
		_objects.push_back(_internal::ObjectInPool<T>());
		return AssignObject(index);
	}

	template <typename T>
	typename ObjectsPool<T>::Ptr ObjectsPool<T>::ObjectsPool<T>::AssignObject(size_t index)
	{
		// increase allocated objects count
		_allocated_objects_count++;

		// update max used index, if needed
		if (index > _max_used_index_in_vector)
		{
			_max_used_index_in_vector = index;
		}

		// get object and id, set it as used
		_internal::ObjectInPool<T>& obj = _objects[index];
		ObjectId id = _next_object_id++;
		obj.set_id(id);
		obj.set_is_used(true);

		// update the pointers table and return object pointer
		_pointers[id] = index;
		auto ret = ObjectsPool<T>::Ptr(this, id);
		ret._set_cached_ptr(&obj.get_object(), _defrags_count);
		return ret;
	}

	template <typename T>
	T& ObjectsPool<T>::_get_object(ObjectId id)
	{
		size_t index = _pointers.at(id);
		_internal::ObjectInPool<T>& obj = *(&_objects[index]);
		return obj.get_object();
	}

	template <typename T>
	void ObjectsPool<T>::Release(typename ObjectsPool<T>::Ptr obj)
	{
		Release(obj._get_id());
	}

	template <typename T>
	void ObjectsPool<T>::Release(ObjectId id)
	{
		// get object index in pool and a reference to the object itself
		auto index = _pointers[id];
		_internal::ObjectInPool<T>& obj_ref = _objects[index];

		// sanity test
		if (!obj_ref.is_used())
		{
			throw AccessViolation();
		}

		// first, remove object from pointers map
		_pointers.erase(id);

		// now decrease actual pool size
		_allocated_objects_count--;

		// set as no longer used
		obj_ref.set_is_used(false);

		// if we happened to release the last object in pool, its the easier case - we just decrease the used index as well
		if (index == _max_used_index_in_vector)
		{
			_max_used_index_in_vector--;
			return;
		}

		// if got here it means we created a hole. add it to holes vector
		_holes.push_back(index);

		// if in immediate defrag mode, do it now
		if (_defrag_mode == DEFRAG_IMMEDIATE)
		{
			Defrag();
		}
	}

	template <typename T>
	size_t ObjectsPool<T>::size() const
	{
		return _allocated_objects_count;
	}

	template <typename T>
	void ObjectsPool<T>::Clear()
	{
		_pointers.clear();
		_objects.clear();
		_holes.clear();
		_allocated_objects_count = 0;
		_next_object_id = 0;
		_max_used_index_in_vector = 0;
	}

	template <typename T>
	void ObjectsPool<T>::Defrag()
	{
		// no holes to fill? nothing to do here
		if (!_holes.size())
		{
			return;
		}

		// increase defragging count
		_defrags_count++;

		// iterate and close holes until we no longer have holes to close
		while (_holes.size())
		{
			// get current index to move
			auto index_to_fill = _holes.pop_back();

			// if index to fill is outside max used index, skip
			if (index_to_fill >= _max_used_index_in_vector)
			{
				continue;
			}

			// move last object into this position
			_objects[index_to_fill] = std::move(_objects[_max_used_index_in_vector]);
			
			// update max used index in vector
			do 
			{
				_max_used_index_in_vector--;
			}
			while (_max_used_index_in_vector > 0 && !_objects[_max_used_index_in_vector].is_used());

			// update the pointers table
			_pointers[_objects[index_to_fill].get_id()] = index_to_fill;
		}

		// check if we need to resize vector
		if (_objects.size() - _max_used_index_in_vector > _shrink_pool_threshold)
		{
			ClearUnusedMemory();
		}
	}

	template <typename T>
	void ObjectsPool<T>::Reserve(size_t amount)
	{
		_objects.reserve(amount);
	}

	template <typename T>
	void ObjectsPool<T>::IterateEx(PoolIteratorCallbackEx<T> callback)
	{
		// if in deferred defrag mode, do it now
		if (_defrag_mode == DEFRAG_DEFERRED)
		{
			Defrag();
		}

		// iterate objects
		for (size_t i = 0; i <= _max_used_index_in_vector; ++i)
		{
			_internal::ObjectInPool<T>& obj = _objects[i];
			if (obj.is_used())
			{
				if (callback(obj.get_object(), obj.get_id(), *this) == IterationReturnCode::ITER_BREAK)
					break;
			}
		}
	}

	template <typename T>
	void ObjectsPool<T>::Iterate(PoolIteratorCallback<T> callback)
	{
		// if in deferred defrag mode, do it now
		if (_defrag_mode == DEFRAG_DEFERRED)
		{
			Defrag();
		}

		// iterate objects
		for (size_t i = 0; i <= _max_used_index_in_vector; ++i)
		{
			_internal::ObjectInPool<T>& obj = _objects[i];
			if (obj.is_used())
			{
				callback(obj.get_object(), obj.get_id());
			}
		}
	}

	template <typename T>
	void ObjectsPool<T>::ClearUnusedMemory()
	{
		// make sure there are not holes
		if (_holes.size())
		{
			throw CannotResizeWhileNotDefragged();
		}

		// resize objects pool
		_objects.resize(_max_used_index_in_vector + 1);
	}
}

#endif // !__OBJECT_POOL_IMP__