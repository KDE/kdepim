/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_RECORD_H
#define __PILOT_RECORD_H

#include <time.h>
#include "pi-file.h"

class PilotRecord 
    {
    public:
    PilotRecord(void* data, int len, int attrib, int cat, pi_uid_t uid);
    PilotRecord(PilotRecord* orig);
    ~PilotRecord() { delete [] fData; }
    
    PilotRecord& operator=(PilotRecord& orig);

    char* getData() const { return fData; }
    int   getLen() const { return fLen; }
    void setData(const char* data, int len);
    int   getAttrib() const { return fAttrib; }
    void  setAttrib(int attrib) { fAttrib = attrib; }

    int   getCat() const { return fCat; }
    void  setCat(int cat) { fCat = cat; }

    unsigned long getID() const { return fID; }
    void setID(unsigned long id) { fID = id; }

    private:
    char* fData;
    int   fLen;
    int   fAttrib;
    int   fCat;
    unsigned long fID;

public:
	// New public functions by ADE
	//
	//
	bool isDeleted() const;
	bool isSecret() const;
	void makeDeleted() ;
	void makeSecret() ;

    };

#endif
