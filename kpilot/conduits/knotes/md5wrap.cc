/* md5wrap.cc
**
** KNotes Conduit Copyright (C) 2000 by    Adriaan de Groot
**
** A simple C++ wrapper for the md5.c stuff provided by Mario.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
  
/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

static const char *id = "$Revision:$" ;

#include "md5wrap.h"


QString MD5Context::finalize()
{
	unsigned char digest[16];
	static const char *hex="0123456789abcdef";

	finalize(digest);

	QString s;

	for (int i=0; i<16; i++)
	{
		s += hex[(digest[i] & 0xf0) >> 4];
		s += hex[digest[i] & 0xf ];
		if (i<15)
		{
			s+= ':';
		}
	}

	return s;
}


// $Log:$
