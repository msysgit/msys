/** \file ref.hpp
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#ifndef REF_HPP_INC
#define REF_HPP_INC


#include "boost/shared_ptr.hpp"


template< class T >
struct RefType
{
	typedef boost::shared_ptr< T > Ref;
};

template< class T >
inline static T* RefGetPtr(const boost::shared_ptr< T >& p)
{
	return p.get();
}


#endif // REF_HPP_INC
