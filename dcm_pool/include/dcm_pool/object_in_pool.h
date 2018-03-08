/*!
* \file	include\dcm_pool\object_in_pool.h.
*
* \brief		Define the ObjectInPool template class.
* 				This class represent an object inside the objects pool.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/

#pragma once
#include "defs.h"
using namespace std;


namespace dcm_pool
{
	namespace _internal
	{

		/*!
		* \class	ObjectInPool
		*
		* \brief	A container that holds an object in the objects pool.
		*
		* \author	Ronen
		* \date	2/21/2018
		*
		* \tparam	T	Base object type that you store in pool.
		*/
		template <typename T>
		class ObjectInPool
		{
		private:

			/*! \brief	The object itself. */
			T _obj;

			/*! \brief	Object id in objects pool. */
			ObjectId _id;

			/*! \brief	Is this object currently used? */
			bool _is_used;

		public:

			/*!
			* \fn	ObjectInPool::ObjectInPool(size_t index);
			*
			* \brief	Constructor
			*
			* \author	Ronen
			* \date	2/22/2018
			*
			* \param	id		Object unique id.
			*/
			ObjectInPool(ObjectId id = 0);

			/*!
			 * \fn	ObjectInPool::ObjectInPool(const ObjectInPool<T>& other);
			 *
			 * \brief	Constructor.
			 *
			 * \author	Ronen
			 * \date	2/23/2018
			 *
			 * \param	other	The other.
			 */
			ObjectInPool(const ObjectInPool<T>& other);

			/*!
			 * \fn	V& ObjectInPool::operator=(ObjectInPool<T>&& other);
			 *
			 * \brief	Move assignment operator.
			 *
			 * \author	Ronen
			 * \date	2/23/2018
			 *
			 * \param [in,out]	other	The other.
			 *
			 * \return	A shallow copy of this object.
			 */
			ObjectInPool<T>& operator=(ObjectInPool<T>&& other);

			/*!
			 * \fn	ObjectInPool<T>& ObjectInPool::operator=(ObjectInPool<T>& other);
			 *
			 * \brief	Assignment operator.
			 *
			 * \author	Ronen
			 * \date	2/23/2018
			 *
			 * \param [in,out]	other	The other.
			 *
			 * \return	A shallow copy of this object.
			 */
			ObjectInPool<T>& operator=(const ObjectInPool<T>& other);

			/*!
			* \fn	inline ObjectId get_id() const;
			*
			* \brief	Gets the object id.
			*
			* \author	Ronen
			* \date	2/22/2018
			*
			* \return	Return the object unique id.
			*/
			inline ObjectId get_id() const;

			/*!
			* \fn	inline void set_id(ObjectId id);
			*
			* \brief	Sets the object id.
			*
			* \author	Ronen
			* \date	2/22/2018
			*/
			inline void set_id(ObjectId id);

			/*!
			 * \fn	inline bool ObjectInPool::is_used() const;
			 *
			 * \brief	Query if this object is used
			 *
			 * \author	Ronen
			 * \date	2/22/2018
			 *
			 * \return	True if used, false if not.
			 */
			inline bool is_used() const;

			/*!
			 * \fn	inline void ObjectInPool::set_is_used();
			 *
			 * \brief	Sets if the object is used.
			 *
			 * \author	Ronen
			 * \date	2/22/2018
			 *
			 * \param	is_used	Is the object used.
			 */
			inline void set_is_used(bool is_used);

			/*!
			 * \fn	inline T& ObjectInPool::get_object();
			 *
			 * \brief	Gets the object itself.
			 *
			 * \author	Ronen
			 * \date	2/22/2018
			 *
			 * \return	The object.
			 */
			inline T& get_object();
		};
	}
}

// include implementation
#include "_object_in_pool_imp.h"