/* 
** Copyright (C) 2003 by Reinhold Kainhofer
**
** This is just a very simple programm to check the compress/uncompress 
** routines by taking one string, compress and then decompress it and
** see if it is the original string.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include <stdio.h>
#include <iostream.h>


#include "../makedoc9.h"

void main () 
{
	tBuf fText;
	char*text="asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";

	fText.setText((const byte*)text);
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	
	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;

	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;

	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;


}
