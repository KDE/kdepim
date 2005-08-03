#ifndef BOOST_TT_REMOVE_CV_HPP_INCLUDED
#define BOOST_TT_REMOVE_CV_HPP_INCLUDED
#ifndef BOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
#define BOOST_TT_DETAIL_CV_TRAITS_IMPL_HPP_INCLUDED
// ADAPTED (TAKEN) FROM BOOST
//
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
//




namespace boost {
namespace detail {

// implementation helper:

template <typename T> struct cv_traits_imp {};

template <typename T>
struct cv_traits_imp<T*>
{
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<const T*>
{
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<volatile T*>
{
    typedef T unqualified_type;
};

template <typename T>
struct cv_traits_imp<const volatile T*>
{
    typedef T unqualified_type;
};

} // namespace detail

template <typename T>
struct remove_cv {
	typedef typename detail::cv_traits_imp<T*>::unqualified_type type;
};
} // namespace boost 



#endif
#endif
