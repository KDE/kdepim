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

    PilotRecord* pack() { return PilotAppCategory::pack(); }

    protected:
    //void *pack(int *i);
    void *pack(void *, int *);
    void unpack(void *, int = 0) { }

    private:
//     int fSize;
    struct Address fAddressInfo;
    //    void *internalPack(unsigned char *);
    };



#endif
