
/* pilotRecord.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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


// $Log:$
