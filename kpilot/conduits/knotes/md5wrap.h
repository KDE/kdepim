/* md5wrap.h
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

// $Revision:$

#ifndef MD5_WRAP_H
#define MD5_WRAP_H

#include <qstring.h>
#include "md5.h"

class MD5Context
{
public:
	MD5Context() { init(); } ;

	void init() { Bin_MD5Init(&md5c); } ;
	void update(unsigned char const *buf, unsigned len)
		{ Bin_MD5Update(&md5c,buf,len); } ;
	QString finalize();
	void finalize(unsigned char digest[16])
		{ Bin_MD5Final(digest,&md5c); } ;

private:
	Bin_MD5Context md5c;
} ;

#endif


// $Log:$
