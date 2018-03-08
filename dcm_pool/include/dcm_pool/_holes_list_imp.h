/*!
* \file	include\dcm_pool\_holes_list.h.
*
* \brief		Implement the HolesList template class.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/

#ifndef __HOLES_LIST_IMP__
#define __HOLES_LIST_IMP__

namespace dcm_pool
{
	namespace _internal
	{
		template <typename T>
		void HolesList<T>::push_back(size_t hole_index)
		{
			// if not empty, take the current first index and set it as the id of the new hole
			if (_size)
			{
				_objects[hole_index].set_id(_first_index);
			}

			// set new first index and increase size
			_first_index = hole_index;
			_size++;
		}

		template <typename T>
		size_t HolesList<T>::pop_back()
		{
			// if size is 0, exception
			if (!_size)
			{
				throw new std::out_of_range("Objects pool holes list out of range!");
			}

			// special case - if its last item just return _first_index and zero size
			if (_size == 1)
			{
				_size = 0;
				return _first_index;
			}

			// get index to return
			size_t to_ret = _first_index;

			// set the new first index
			_first_index = _objects[to_ret].get_id();

			// decrease size and return index
			_size--;
			return to_ret;
		}

		template <typename T>
		void HolesList<T>::clear()
		{
			_size = 0;
		}
	}
}

#endif