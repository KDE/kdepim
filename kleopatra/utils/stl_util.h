/****************************************************************************
** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/
#ifndef __KDTOOLSCORE_STL_UTIL_H__
#define __KDTOOLSCORE_STL_UTIL_H__

#include <algorithm>
#include <numeric>
#include <utility>

#include <boost/range.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace kdtools {

    struct nodelete {
        template <typename T>
        void operator()( const T * ) const {}
    };

    template <typename InputIterator, typename OutputIterator, typename UnaryPredicate>
    OutputIterator copy_if( InputIterator first, InputIterator last, OutputIterator dest, UnaryPredicate pred ) {
	while ( first != last ) {
	    if ( pred( *first ) ) {
		*dest = *first;
		++dest;
	    }
	    ++first;
	}
	return dest;
    }

    template <typename OutputIterator, typename InputIterator, typename UnaryFunction, typename UnaryPredicate>
    OutputIterator transform_if( InputIterator first, InputIterator last, OutputIterator dest, UnaryPredicate pred, UnaryFunction filter ) {
        return std::transform( boost::make_filter_iterator( filter, first, last ),
                               boost::make_filter_iterator( filter, last,  last ),
                               dest, pred );
    }

    template <typename Value, typename InputIterator, typename UnaryPredicate>
    Value accumulate_if( InputIterator first, InputIterator last, UnaryPredicate filter, const Value & value=Value() ) {
        return std::accumulate( boost::make_filter_iterator( filter, first, last ),
                                boost::make_filter_iterator( filter, last,  last ), value );
    }

    template <typename InputIterator, typename OutputIterator1, typename OutputIterator2, typename UnaryPredicate>
    std::pair<OutputIterator1,OutputIterator2> separate_if( InputIterator first, InputIterator last, OutputIterator1 dest1, OutputIterator2 dest2, UnaryPredicate pred ) {
        while ( first != last ) {
            if ( pred( *first ) ) {
                *dest1 = *first;
                ++dest1;
            } else {
                *dest2 = *first;
                ++dest2;
            }
            ++first;
        }
        return std::make_pair( dest1, dest2 );
    }

    template <typename InputIterator>
    bool any( InputIterator first, InputIterator last ) {
	while ( first != last )
	    if ( *first )
		return true;
	    else
		++first;
	return false;
    }

    template <typename InputIterator, typename UnaryPredicate>
    bool any( InputIterator first, InputIterator last, UnaryPredicate pred ) {
	while ( first != last )
	    if ( pred( *first ) )
		return true;
	    else
		++first;
	return false;
    }

    template <typename InputIterator>
    bool all( InputIterator first, InputIterator last ) {
	while ( first != last )
	    if ( *first )
		++first;
	    else
		return false;
	return true;
    }

    template <typename InputIterator, typename UnaryPredicate>
    bool all( InputIterator first, InputIterator last, UnaryPredicate pred ) {
	while ( first != last )
	    if ( pred( *first ) )
		++first;
	    else
		return false;
	return true;
    }

    template <typename InputIterator>
    bool none_of( InputIterator first, InputIterator last ) {
        return !any( first, last );
    }

    template <typename InputIterator, typename UnaryPredicate>
    bool none_of( InputIterator first, InputIterator last, UnaryPredicate pred ) {
        return !any( first, last, pred );
    }

    template <typename InputIterator, typename BinaryOperation>
    BinaryOperation for_each_adjacent_pair( InputIterator first, InputIterator last, BinaryOperation op ) {
        typedef typename std::iterator_traits<InputIterator>::value_type ValueType;
        if ( first == last )
            return op;
        ValueType value = *first;
        while ( ++first != last ) {
            ValueType tmp = *first;
            op( value, tmp );
            value = tmp;
        }
        return op;
    }

    //@{
    /**
       Versions of std::set_intersection optimized for ForwardIterator's
    */
    template <typename ForwardIterator, typename ForwardIterator2, typename OutputIterator, typename BinaryPredicate>
    OutputIterator set_intersection( ForwardIterator first1, ForwardIterator last1, ForwardIterator2 first2, ForwardIterator2 last2, OutputIterator result ) {
        while ( first1 != last1 && first2 != last2 ) {
            if ( *first1 < *first2 ) {
                first1 = std::lower_bound( ++first1, last1, *first2 );
            } else if ( *first2 < *first1 ) {
                first2 = std::lower_bound( ++first2, last2, *first1 );
            } else {
                *result = *first1;
                ++first1;
                ++first2;
                ++result;
            }
        }
        return result;
    }

    template <typename ForwardIterator, typename ForwardIterator2, typename OutputIterator, typename BinaryPredicate>
    OutputIterator set_intersection( ForwardIterator first1, ForwardIterator last1, ForwardIterator2 first2, ForwardIterator2 last2, OutputIterator result, BinaryPredicate pred ) {
        while ( first1 != last1 && first2 != last2 ) {
            if ( pred( *first1, *first2 ) ) {
                first1 = std::lower_bound( ++first1, last1, *first2, pred );
            } else if ( pred( *first2, *first1 ) ) {
                first2 = std::lower_bound( ++first2, last2, *first1, pred );
            } else {
                *result = *first1;
                ++first1;
                ++first2;
                ++result;
            }
        }
        return result;
    }
    //@}

    template <typename ForwardIterator, typename ForwardIterator2, typename BinaryPredicate>
        bool set_intersects( ForwardIterator first1,  ForwardIterator last1,
                             ForwardIterator2 first2, ForwardIterator2 last2,
                             BinaryPredicate pred )
    {
        while ( first1 != last1 && first2 != last2 ) {
            if ( pred( *first1, *first2 ) ) {
                first1 = std::lower_bound( ++first1, last1, *first2, pred );
            } else if ( pred( *first2, *first1 ) ) {
                first2 = std::lower_bound( ++first2, last2, *first1, pred );
            } else {
                return true;
            }
        }
        return false;
    }

    //@{
    /*! Versions of std algorithms that take ranges */

    template <typename C, typename V>
    bool contains( const C & c, const V & v ) {
        return std::find( boost::begin( c ), boost::end( c ), v ) != boost::end( c ) ;
    }

    template <typename C, typename P>
    bool contains_if( const C & c, P p ) {
        return std::find_if( boost::begin( c ), boost::end( c ), p ) != boost::end( c );
    }

    template <typename C, typename V>
    size_t count( const C & c, const V & v ) {
        return std::count( boost::begin( c ), boost::end( c ), v );
    }

    template <typename C, typename P>
    size_t count_if( const C & c, P p ) {
        return std::count_if( boost::begin( c ), boost::end( c ), p );
    }

    template <typename O, typename I, typename P>
    O transform( const I & i, P p ) {
        O o;
        std::transform( boost::begin( i ), boost::end( i ),
                        std::back_inserter( o ), p );
        return o;
    }

    template <typename O, typename I, typename P, typename F>
    O transform_if( const I & i, P p, F f ) {
        O o;
        transform_if( boost::begin( i ), boost::end( i ),
                      std::back_inserter( o ), p, f );
        return o;
    }

    template <typename V, typename I, typename F>
    V accumulate_if( const I & i, F f, V v=V() ) {
        return accumulate_if( boost::begin( i ), boost::end( i ), f, v );
    }

    template <typename O, typename I>
    O copy( const I & i ) {
        O o;
        std::copy( boost::begin( i ), boost::end( i ), std::back_inserter( o ) );
        return o;
    }

    template <typename O, typename I, typename P>
    O copy_if( const I & i, P p ) {
        O o;
        copy_if( boost::begin( i ), boost::end( i ), std::back_inserter( o ), p );
        return o;
    }

    template <typename I, typename P>
    P for_each( const I & i, P p ) {
        return std::for_each( boost::begin( i ), boost::end( i ), p );
    }

    //@}

    template <typename C>
    bool any( const C & c ) {
        return any( boost::begin( c ), boost::end( c ) );
    }

    template <typename C, typename P>
    bool any( const C & c, P p ) {
        return any( boost::begin( c ), boost::end( c ), p );
    }

    template <typename C>
    bool all( const C & c ) {
        return all( boost::begin( c ), boost::end( c ) );
    }

    template <typename C, typename P>
    bool all( const C & c, P p ) {
        return all( boost::begin( c ), boost::end( c ), p );
    }

    template <typename C>
    bool none_of( const C & c ) {
        return none_of( boost::begin( c ), boost::end( c ) );
    }

    template <typename C, typename P>
    bool none_of( const C & c, P p ) {
        return none_of( boost::begin( c ), boost::end( c ), p );
    }

    template <typename C, typename B>
    B for_each_adjacent_pair( const C & c, B b ) {
        return for_each_adjacent_pair( boost::begin( c ), boost::end( c ), b );
    }

    template <typename C>
    void sort( C & c ) {
        return std::sort( boost::begin( c ), boost::end( c ) );
    }

    template <typename C, typename P>
    void sort( C & c, P p ) {
        return std::sort( boost::begin( c ), boost::end( c ), p );
    }

}

#endif /* __KDTOOLSCORE_STL_UTIL_H__ */

