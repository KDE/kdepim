/* strToken.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is something horrible. It's a weirdly written tokenizer,
** used by the address import function. It's badly in need of repair
** or replacement.
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


#include "strToken.h"
#include <string.h>

static const char *strToken_id =
	"$Id:$";

StrTokenizer::StrTokenizer(const char* string, const char* delims)
    {
    fOrigString = fString = new char[strlen(string) + 1];
    strcpy(fString, string);
    fDelims = new char[strlen(delims) + 1];
    strcpy(fDelims, delims);
    }

const char*
StrTokenizer::getNextField()
    {
    char* strStart = fString;

    if(*fString == 0L)
	return 0L;
    while(*fString && !(strchr(fDelims, *fString)))
	fString++;
    if(*fString)
	{
	*fString = 0L;
	fString++;
	}
    return strStart;
    }


// $Log$
// Revision 1.4  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
