/* pilotAddress.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the pilot's address database structures.
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


static const char *pilotadress_id="$Id$";

#include "options.h"

#include <stdlib.h>
#include "pilotAddress.h"

PilotAddress::PilotAddress(PilotRecord* rec)
  : PilotAppCategory(rec)
    {
    unpack_Address(&fAddressInfo, (unsigned char*)rec->getData(), rec->getLen());
    }

void 
PilotAddress::setField(int field, const char* text)
    {
    // This will have either been created with unpack_Address, and/or will
    // be released with free_Address, so use malloc/free here:
    if(fAddressInfo.entry[field])
	{
	free(fAddressInfo.entry[field]);
	}
    if (text)
      {
	fAddressInfo.entry[field] = (char*)malloc(strlen(text) + 1);
	strcpy(fAddressInfo.entry[field], text);
      }
    else
      fAddressInfo.entry[field] = 0L;
    }

void*
PilotAddress::pack(void *buf, int *len)
    {
    int i;
    i = pack_Address(&fAddressInfo, (unsigned char*)buf, *len);
    *len = i;
    return buf;
    }

// $Log$
// Revision 1.8  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
