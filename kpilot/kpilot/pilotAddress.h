/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
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
