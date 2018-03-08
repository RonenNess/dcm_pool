/*!
* \file	include\dcm_pool\exceptions.h.
*
* \brief		Define internal exceptions that Fast Objects Pool may throw.
* \license		MIT
* \autor		Ronen Ness.
* \since		2018
*/


#pragma once
#include <exception>

namespace dcm_pool
{
	/*!
	 * \struct	ExceededPoolLimit
	 *
	 * \brief	Raised when tried to allocate more objects than limited in pool.
	 *
	 * \author	Ronen
	 * \date	2/21/2018
	 */
	struct ExceededPoolLimit : public std::exception
	{
		const char * what() const throw ()
		{
			return "Exceeded pool max limit!";
		}
	};

	/*!
	* \struct	AccessViolation
	*
	* \brief	Raised for when someone tries to use or release an object that is not in used.
	*
	* \author	Ronen
	* \date	2/21/2018
	*/
	struct AccessViolation : public std::exception
	{
		const char * what() const throw ()
		{
			return "Tried to release or use a released object!";
		}
	};

	/*!
	* \struct	CannotResizeWhileNotDefragged
	*
	* \brief	Raised for when someone tries to resize pool but it has holes to defrag.
	*
	* \author	Ronen
	* \date	2/21/2018
	*/
	struct CannotResizeWhileNotDefragged : public std::exception
	{
		const char * what() const throw ()
		{
			return "Cannot resize pool while there are holes to defrag!";
		}
	};

	/*!
	* \struct	InternalError
	*
	* \brief	Raised for internal errors or corrupted pool data.
	*
	* \author	Ronen
	* \date	2/21/2018
	*/
	struct InternalError : public std::exception
	{
		const char * what() const throw ()
		{
			return "Internal error or corrupted data!";
		}
	};
}