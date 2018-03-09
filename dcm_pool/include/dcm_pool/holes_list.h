/*!
* \file	include\dcm_pool\holes_list.h.
*
* \brief		An internal list of holes in pool without wasting additional memory.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/


#pragma once

#include <vector>
#include "object_in_pool.h"
#include "defs.h"


using namespace std;

namespace dcm_pool
{
	namespace _internal
	{
		/*!
		* \class	DcmPool
		*
		* \brief	An internal object used to hold a vector of holes in a pool, without wasting any additional memory.
		* 			This list makes use of the objects-in-pool header of the already free objects.
		*
		* \author	Ronen
		* \date	2/21/2018
		*
		* \tparam	T	Type of objects to place in pool.
		*/
		template <typename T>
		class HolesList
		{
		private:

			// holes list current size.
			size_t _size;

			// the first index used to store holes in the vector
			size_t _first_index;

			// the actual vector of objects we use.
			vector<ObjectInPool<T> >& _objects;

		public:

			/*!
			 * \fn	HolesList::HolesList(vector<ObjectInPool<T> >& objects)
			 *
			 * \brief	Constructor
			 *
			 * \author	Ronen
			 * \date	3/8/2018
			 *
			 * \param [in,out]	objects	The objects.
			 */
			HolesList(vector<ObjectInPool<T> >& objects) : _objects(objects), _size(0) { }

			/*!
			 * \fn	inline size_t HolesList::size() const
			 *
			 * \brief	Gets the size of the holes list.
			 *
			 * \author	Ronen
			 * \date	3/8/2018
			 *
			 * \return	List size.
			 */
			inline size_t size() const { return _size; }

			/*!
			 * \fn	void HolesList::push_back(size_t hole_index);
			 *
			 * \brief	Pushes a hole index into the holes list.
			 *
			 * \author	Ronen
			 * \date	3/8/2018
			 *
			 * \param	hole_index	Zero-based index of the hole.
			 */
			void push_back(size_t hole_index);

			/*!
			 * \fn	void HolesList::pop_back();
			 *
			 * \brief	Pops the last hole from the list.
			 *
			 * \author	Ronen
			 * \date	3/8/2018
			 */
			size_t pop_back();

			/*!
			 * \fn	void HolesList::clear();
			 *
			 * \brief	Clears this list.
			 *
			 * \author	Ronen
			 * \date	3/8/2018
			 */
			void clear();
		};
	}
}

#include "_holes_list_imp.h"