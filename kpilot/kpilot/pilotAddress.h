/* pilotAddress.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a wrapper for pilot-link's address structures.
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
#ifndef __KPILOT_ADDRESS_H
#define __KPILOT_ADDRESS_H

#include <time.h>
#include <string.h>
#include <pi-macros.h>
#include <pi-address.h>
#include "pilotAppCategory.h"
#include "pilotRecord.h"
#include <string.h>

class PilotAddress : public PilotAppCategory
    {
    public:
    PilotAddress(void) : PilotAppCategory() 
            { memset(&fAddressInfo, 0, sizeof(struct Address)); }
    PilotAddress(PilotRecord* rec);
    ~PilotAddress() { free_Address(&fAddressInfo); }
    
    void setField(int field, const char* text);
    char* getField(int field) { return fAddressInfo.entry[field]; }
    int  getPhoneLabelIndex(int index) { return fAddressInfo.phoneLabel[index]; }
	/**
	* Returns the (adjusted) index of the phone number
	* selected by the user to be shown in the
	* overview of addresses. Adjusted here means
	* that it's actually an index into 3..8, the fields
	* that store phone numbers, so 0 means field 3 is selected.
	*/
	int getShownPhone() const { return fAddressInfo.showPhone; }

    PilotRecord* pack() { return PilotAppCategory::pack(); }

    protected:
    //void *pack(int *i);
    void *pack(void *, int *);
    void unpack(const void *, int = 0) { }

    private:
//     int fSize;
    struct Address fAddressInfo;
    //    void *internalPack(unsigned char *);
    };



#endif


// $Log:$
