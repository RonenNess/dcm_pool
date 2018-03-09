/*!
* \file	include\dcm_pool\defs.h.
*
* \brief		Basic defs for the pool objects.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/


#pragma once


using namespace std;

namespace dcm_pool
{
	// predeclare objects pool
	template <typename T>
	class DcmPool;

	/*! \brief	Invalid index / max object id. */
	const size_t ObjectPoolMaxIndex = std::numeric_limits<std::size_t>::max();

	/*!
	* \typedef	unsigned int ObjectId
	*
	* \brief	Represent an internal object id while in objects pool.
	*/
	typedef size_t ObjectId;

	/*!
	* \enum	IterationReturnCode
	*
	* \brief	Different return code the iteration callback can return.
	*/
	enum IterationReturnCode
	{
		ITER_CONTINUE,
		ITER_BREAK
	};

	/*!
	* \typedef	void(*pool_iterator)(T&, ObjectId, DcmPool<T>&)
	*
	* \brief	Callback used to iterate objects pool with extended options.
	*/
	template <typename T>
	using PoolIteratorEx = IterationReturnCode(*)(T&, ObjectId, DcmPool<T>&);

	/*!
	* \typedef	void(*pool_iterator)(T&, ObjectId)
	*
	* \brief	A simple callback used to iterate objects pool.
	*/
	template <typename T>
	using PoolIterator = void(*)(T&, ObjectId);

    	/*!
	* \typedef	void(*pool_iterator)(const T&, ObjectId, const DcmPool<T>&)
	*
	* \brief	Callback used to iterate objects pool with extended options.
	*/
	template <typename T>
	using ConstPoolIteratorEx = IterationReturnCode(*)(const T&, ObjectId, const DcmPool<T>&);

	/*!
	* \typedef	void(*pool_iterator)(const T&, ObjectId)
	*
	* \brief	A simple callback used to iterate objects pool.
	*/
	template <typename T>
	using ConstPoolIterator = void(*)(const T&, ObjectId);
    
	/*!
	* \enum	DefragModes
	*
	* \brief	Different ways we can handle fragmentation in our objects pool, eg 'holes' we need to close after releasing objects.
	*/
	enum DefragModes
	{
		/* \brief	Will close holes immediately, eg the moment you release an object it will make sure to populate its place. */
		DEFRAG_IMMEDIATE,

		/* \brief	Will close holes next time we try to iterate the pool. the advantage here is that sometimes we release and
		then alloc, so in this case we won't do a useless moving, or if we release a huge chunk from the end. */
		DEFRAG_DEFERRED,

		/* \brief	Will never call defragging automatically, you need to call Defrag() yourself. */
		DEFRAG_MANUAL,
	};
}