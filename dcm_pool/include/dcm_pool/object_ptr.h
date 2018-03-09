/*!
* \file	include\dcm_pool\object_ptr.h.
*
* \brief		Define the ObjectPtr template class.
* 				This class wraps a pointer to an object inside the objects pool.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/

#pragma once
#include "object_ptr.h"
#include "object_in_pool.h"
#include "defs.h"


using namespace std;

namespace dcm_pool
{
	/*!
	 * \class	ObjectPtr
	 *
	 * \brief	A pointer to an object inside the fast objects pool.
	 * 			Use this for direct access to objects from the pool, or to release the object when
	 * 			you're done using it.
	 *
	 * \author	Ronen
	 * \date	2/21/2018
	 *
	 * \tparam	T	Base object type that you store in pool.
	 */
	template <typename T>
	class ObjectPtr
	{
	private:

		/*! \brief	The pool containing this object. */
		DcmPool<T>* _pool;

		/*! \brief	The object's unique id. */
		ObjectId _id;

		/*! \brief	Saving a cache of the actual object pointer. */
		T* _cached_ptr;

		/*! \brief	Last pool version to indicate if cached pointer is still valid to use. */
		unsigned int _pool_defrag_version;

	public:

		/*!
		 * \fn	ObjectPtr::ObjectPtr(size_t index);
		 *
		 * \brief	Constructor
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	pool	The parent objects pool.
		 * \param	id		Object's unique id in pool.
		 */
		ObjectPtr(DcmPool<T>* pool = NULL, ObjectId id = ObjectPoolMaxIndex);

		/*!
		 * \fn	inline ObjectId ObjectPtr::_get_id() const;
		 *
		 * \brief	Gets the object id in pool.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \return	Return the object id.
		 */
		inline ObjectId _get_id() const;

		/*!
		 * \fn	T ObjectPtr::*operator*(void) const;
		 *
		 * \brief	Return the object itself.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \return	The object instance.
		 */
		T& operator*(void);

		/*!
		 * \fn	T& ObjectPtr::operator->(void) const;
		 *
		 * \brief	Member dereference operator.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \return	The dereferenced object.
		 */
		T* operator->(void);

		/*!
		 * \fn	inline bool ObjectPtr::operator==(const ObjectPtr<T>& other) const
		 *
		 * \brief	Equality operator.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	other	The other pointer to compare to.
		 *
		 * \return	True if the parameters are considered equivalent.
		 */
		inline bool operator==(const ObjectPtr<T>& other) const { _id == other._id && _pool == other._pool; }

		/*!
		 * \fn	inline bool ObjectPtr::operator!=(const ObjectPtr<T>& other) const
		 *
		 * \brief	Inequality operator.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	other	The other pointer to compare to.
		 *
		 * \return	True if the parameters are not considered equivalent.
		 */
		inline bool operator!=(const ObjectPtr<T>& other) const { return !(*this == other); }

		/*!
		 * \fn	inline void ObjectPtr::operator=(const ObjectPtr<T>& other)
		 *
		 * \brief	Assignment operator.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	other	Other pointer to assign.
		 */

		inline void operator=(const ObjectPtr<T>& other) {
			_id = other._id; 
			_pool = other._pool;
		}

		/*!
		 * \fn	inline void ObjectPtr::_set_cached_ptr(T* ptr, unsigned int defrag_version)
		 *
		 * \brief	Sets cached pointer and defrag version.
		 * 			We use this to populate the pointer upon creation, since we already know the object address
		 * 			at this point and its very likely that the user will want to use the pointer immediately after
		 * 			getting it (to init the object).
		 *
		 * \author	Ronen
		 * \date	2/23/2018
		 *
		 * \param [in,out]	ptr			  	Object pointer.
		 * \param 		  	defrag_version	The pool's defrag version.
		 */
		inline void _set_cached_ptr(T* ptr, unsigned int defrag_version) { 
			_cached_ptr = ptr; 
			_pool_defrag_version = defrag_version; 
		}
	};
}


// include implementation
#include "_object_ptr_imp.h"