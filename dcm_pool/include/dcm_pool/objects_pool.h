/*!
* \file	include\dcm_pool\objects_pool.h.
*
* \brief		Define the main ObjectsPool template class.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/


#pragma once

#include <vector>
#include <unordered_map>
#include "object_ptr.h"
#include "holes_list.h"
#include "defs.h"

using namespace std;

namespace dcm_pool
{
	/*!
	 * \class	ObjectsPool
	 *
	 * \brief	Dynamic, Contiguous-Memory objects pool.
	 * 			This object allows you to quickly allocate and release objects from a dynamic pool, without unnecessary new/release calls.
	 * 			In addition, it keeps all objects in a single block of continuous memory, so iterating them is ultra-fast and benefits greatly
	 * 			from CPU-caching of RAM blocks.
	 *
	 * 			In short, this pool is..
	 *				- Dynamic: does not pre-allocate all objects. It grows and shrink as you use it.
	 *				- Contiguous: will hold all objects in a contiguous memory block, and defrag when needed.
	 *
	 * 			Notes:
	 * 				- To access an object from the pool externally (eg not via iteration) you need to use the ObjectsPool<T>::Ptr object.
	 * 				- The pool use the move assignment operator internally, so implementing it will boost performance greatly.
	 * 				- Your objects (T) must have a default constructor, and obviously an Init() function you'll need to call manually after allocating.
	 * 				- The pool is not thread safe!
	 *
	 * 			Performance:
	 * 				- Iterating objects in the pool is optimal, eg O(N) on a contiguous memory block.
	 * 				- Allocting is normally O(1) (unless exceed memory block and need to realloc the whole pool - can be avoided with reserved).
	 * 				- Releasing is normally O(1).
	 * 				- Accessing from the object pointer is ~O(1), sometimes will invoke accessing unordered map.
	 *
	 * 			Defragging:
	 * 				To keep the memory Contiguous, there's a need to 'close holes' whenever they are created, eg when an object is 
	 * 				released from the pool (and its index is not the last index). There are 3 modes to handle defragging:
	 * 				- DEFRAG_IMMEDIATE: will close holes the moment they are created. This option is not optimal but have predictable speed.  
	 * 				- DEFRAG_DEFERRED: will do defragging when trying to iterate the pool. More efficient, but less predictable.
	 *				- DEFRAG_MANUAL: will not do defragging automatically, you need to call Defrag() yourself when you see fit.
	 *
	 * 			Usecase:
	 * 				This pool is useful for scenarios where you need to do a lot of allocating and releasing of objects, while
	 * 				having an update loop that needs to process all of them on a timely-basis.
	 * 				For example, think of a game with pool of bullets - whenever an enemy shoots a bullet we need to allocate an
	 * 				object, when it hits we need to release it, and every frame we need to iterate and move all bullets.
	 * 				This pool help us manage those bullets pool in a smart way that iterating them is easy, and we don't really
	 * 				allocate and release them all the time.
	 *
	 *				How does it work:		
	 *			1. The pool uses a vector internally to store the objects in memory.
	 *			2. The vector grows and shrink as the pool changes its size.
	 *			3. When you release an object, it creates a 'hole' in the contiguous vector memory. 
	 *				3. a. That hole is closed during the defragging process. 
	 *				3. b. We can accumulate a list of holes if defragging mode is not immediate.
	 *				3. c. Defragging takes O(N), where N is number of holes (not number of pool). We close holes by taking objects from the end of the vector.
	 *			4. To iterate the vector you call Iterate(), which takes a function pointer to run on every valid object in pool.
	 *			5. Since defragging shuffles memory, you can't access objects by index. To solve this, we use a special pointer class to allow access to specific objects:
	 *				5. a. Every object in pool is asigned with a unique id.
	 *				5. b. The pool keeps an internal hash table to convert id to actual index (hidden from the user).
	 *				5. c. When the pointer tries to fetch the object it points on, if the pool was defragged since last access it use the hash table to find the new objects index.
	 *			6. To store the list of holes in the vector we reuse the free objects, so no additional memory is wasted.
	 *
	 *
	 * \author	Ronen
	 * \date	2/21/2018
	 *
	 * \tparam	T	Type of objects to place in pool.
	 * 				Note: to fully enjoy the benefit of continuous memory and cpu caching, its recommended to
	 * 				provide an actual type and not a pointer.
	 */
	template <typename T>
	class ObjectsPool
	{
	public:

		/*!
		 * \class	Ptr
		 *
		 * \brief	A pointer to an object inside this pool.
		 *
		 * \author	Ronen
		 * \date	2/23/2018
		 */
		class Ptr : public ObjectPtr<T>
		{
		public:
			Ptr(ObjectsPool<T>* pool = NULL, ObjectId id = ObjectPoolMaxIndex) :
				ObjectPtr<T>(pool, id) {}
		};

	private:

		/*! \brief	The pooled objects. */
		vector<_internal::ObjectInPool<T> > _objects;

		/*! \brief	Convert unique object id to its index in pools vector. */
		unordered_map <ObjectId, size_t> _pointers;

		// holes inside the pool
		_internal::HolesList<T> _holes;

		/*! \brief	Used to assign unique object ids to new objects in pool. */
		ObjectId _next_object_id;

		/*! \brief	Max objects count in pool. */
		size_t _max_size;

		/*! \brief	Current pool actual size (allocated objects). */
		size_t _allocated_objects_count;

		/*! \brief	Highest index we actually use in the objects vector (remember that there could be holes in the vector if not defragged). */
		size_t _max_used_index_in_vector;

		/*! \brief	Required diff between actually used objects and vector size to make us resize the vector. */
		size_t _shrink_pool_threshold;

		/*! \brief	How to handle defragging. */
		DefragModes _defrag_mode;

		/*! \brief	How many times was this pool defragged? */
		unsigned int _defrags_count;

	public:

		/*!
		 * \fn	ObjectsPool::ObjectsPool(std::size_t max_size);
		 *
		 * \brief	Constructor
		 *
		 * \author	Ronen
		 * \date	2/21/2018
		 *
		 * \param	max_size			Maximum objects count in pool. Set to 0 for unlimited count.
		 * \param	reserve				How many objects to reserve in the pool memory (using vector's reserve).
		 * \param	shrink_threshold	The pool uses a vector internally to hold objects. When you allocate more objects, the vector grows.
		 * 								This number decides when to shrink the vector down, if objects are released.
		 * \param	defrag_mode			How to handle defragging.
		 */
		ObjectsPool(size_t max_size = 0, size_t reserve = 0, size_t shrink_threshold = 1024, DefragModes defrag_mode = DEFRAG_DEFERRED);

		/*!
		 * \fn	Ptr ObjectsPool::Alloc();
		 *
		 * \brief	Allocate an object from the pool.
		 *
		 * \author	Ronen
		 * \date	2/21/2018
		 *
		 * \return	An ObjectPtr pointing at the newly-allocated object.
		 * 			You must keep it to later release the object.
		 */
		Ptr Alloc();

		/*!
		 * \fn	void ObjectsPool::Release(ObjectPtr<T> obj);
		 *
		 * \brief	Releases the given object and return it to the pool.
		 *
		 * \author	Ronen
		 * \date	2/21/2018
		 *
		 * \param	obj		Object to release.
		 */
		void Release(Ptr obj);

		/*!
		* \fn	void ObjectsPool::Release(ObjectId id);
		*
		* \brief	Releases the given object and return it to the pool.
		*
		* \author	Ronen
		* \date	2/21/2018
		*
		* \param	id		Object to release.
		*/
		void Release(ObjectId id);

		/*!
		 * \fn	void ObjectsPool::Reserve(size_t amount);
		 *
		 * \brief	Reserves the given amount in the internal vector used to hold the pool.
		 *
		 * \author	Ronen
		 * \date	3/8/2018
		 *
		 * \param	amount	The amount of objects to reserve.
		 */
		void Reserve(size_t amount);

		/*!
		 * \fn	void ObjectsPool::Iterate(PoolIterator<T> callback);
		 *
		 * \brief	Iterates all the objects in pool.
		 * 			Note: if working in deferred defrag mode, this will trigger defrag.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	callback	The callback to use on the objects while iterating.
		 * 						Return false to break the iteration.
		 */
		void Iterate(PoolIterator<T> callback);

		/*!
		* \fn	void ObjectsPool::Iterate(PoolIteratorEx<T> callback);
		*
		* \brief	Iterates all the objects in pool with extended options.
		* 			Note: if working in deferred defrag mode, this will trigger defrag.
		*
		* \author	Ronen
		* \date	2/22/2018
		*
		* \param	callback	The callback to use on the objects while iterating.
		* 						Return false to break the iteration.
		*/
		void IterateEx(PoolIteratorEx<T> callback);

		/*!
		 * \fn	void ObjectsPool::Iterate(ConstPoolIterator<T> callback) const;
		 *
		 * \brief	Iterates all the objects in pool.
		 * 			Note: if working in deferred defrag mode, this will trigger defrag.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	callback	The callback to use on the objects while iterating.
		 * 						Return false to break the iteration.
		 */
		void Iterate(ConstPoolIterator<T> callback) const;

		/*!
		* \fn	void ObjectsPool::Iterate(ConstPoolIteratorEx<T> callback) const;
		*
		* \brief	Iterates all the objects in pool with extended options.
		* 			Note: if working in deferred defrag mode, this will trigger defrag.
		*
		* \author	Ronen
		* \date	2/22/2018
		*
		* \param	callback	The callback to use on the objects while iterating.
		* 						Return false to break the iteration.
		*/
		void IterateEx(ConstPoolIteratorEx<T> callback) const;
        
		/*!
		 * \fn	void ObjectsPool::Clear();
		 *
		 * \brief	Clears the entire pool, making all objects in it free.
		 * 			Watch out, don't use this if you still hold pointer to objects from outside.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 */
		void Clear();

		/*!
		 * \fn	inline size_t ObjectsPool::get_size() const;
		 *
		 * \brief	Gets the size of the pool, eg allocated objects count.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \return	Pool size.
		 */
		inline size_t size() const;

		/*!
		 * \fn	T ObjectsPool::_get_object(ObjectId id);
		 *
		 * \brief	Gets an object from id.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	id		Object id to get..
		 *
		 * \return	The object itself.
		 */
		T& _get_object(ObjectId id);

		/*!
		 * \fn	void ObjectsPool::ClearUnusedMemory();
		 *
		 * \brief	Force the pool to clear unused memory now.
		 * 			This process happens automatically as you release objects, based on shrink_threshold.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 */
		void ClearUnusedMemory();

		/*!
		* \fn	void ObjectsPool::Defrag();
		*
		* \brief	Defrags the pool to make the memory continuous.
		*
		* \author	Ronen
		* \date	2/22/2018
		*/
		void Defrag();

		/*!
		 * \fn	inline unsigned int ObjectsPool::_get_defrags_count() const
		 *
		 * \brief	Gets defrags count.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \return	The defrags count.
		 */
		inline unsigned int _get_defrags_count() const { return _defrags_count; }

	private:

		/*!
		 * \fn	Ptr ObjectsPool<T>::AssignObject(size_t index);
		 *
		 * \brief	Assign an object in the pool (internally) and return its object pointer.
		 *
		 * \author	Ronen
		 * \date	2/22/2018
		 *
		 * \param	index	Zero-based index of the object in pool.
		 *
		 * \return	New object pointer.
		 */
		Ptr AssignObject(size_t index);

	};
}

// include implementation
#include "_objects_pool_imp.h"