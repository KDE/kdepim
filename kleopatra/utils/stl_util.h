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

namespace kdtools {

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

}

#endif /* __KDTOOLSCORE_STL_UTIL_H__ */

