// pilotAddress.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
static const char *id="$Id$";

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

	  
