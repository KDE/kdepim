#ifndef LPC_BITIO_H1103129408_INCLUDE_GUARD_
#define LPC_BITIO_H1103129408_INCLUDE_GUARD_

/* This file is part of indexlib.
 * Copyright (C) 2005 Luís Pedro Coelho <luis@luispedro.org>
 *
 * Indexlib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation and available as file
 * GPL_V2 which is distributed along with indexlib.
 * 
 * Indexlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this program with any edition of
 * the Qt library by Trolltech AS, Norway (or with modified versions
 * of Qt that use the same license as Qt), and distribute linked
 * combinations including the two.  You must obey the GNU General
 * Public License in all respects for all of the code used other than
 * Qt.  If you modify this file, you may extend this exception to
 * your version of the file, but you are not obligated to do so.  If
 * you do not wish to do so, delete this exception statement from
 * your version.
 */

#include <inttypes.h>
#include "boost-compat/remove_cv.hpp"

/**
 * \namespace byte_io
 *
 * This namespace aggregates all input/output functions
 * for our in-disk format, as well as information relating to
 * that format.
 *
 */
namespace byte_io {
	template <typename T>
	void write( unsigned char*, const T );

	template<typename T>
	T read( const unsigned char* );

	template<typename T>
	struct byte_lenght_struct { };

	template<typename T>
	struct byte_lenght_struct<const T> {
		static const int value = byte_lenght_struct<T>::value;
	};
	

	/**
	 * Returns how many bytes type T occupies on disk. It's only defined
	 * for supported types.
	 */
	template<typename T>
	unsigned byte_lenght() {
		return byte_lenght_struct<T>::value;
	}
}

#include "bitio.tcc"

#endif /* LPC_BITIO_H1103129408_INCLUDE_GUARD_ */
