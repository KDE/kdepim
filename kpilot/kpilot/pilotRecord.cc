/* pilotRecord.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a wrapper -- and base class -- for pilot-link's general
** Pilot database structures. It serves as a base class for other
** classes specialized for a particular database.
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
#include <string.h>
#include "pilotRecord.h"

static const char *pilotRecord_id =
	"$Id:$";


PilotRecord::PilotRecord(void* data, int len, int attrib, int cat, pi_uid_t uid)
  : fData(0L), fLen(len), fAttrib(attrib), fCat(cat), fID(uid)
    {
    fData = new char[len];
    memcpy(fData, data, len);
    }

PilotRecord::PilotRecord(PilotRecord* orig)
    {
    fData = new char[orig->getLen()];
    memcpy(fData, orig->getData(), orig->getLen());
    fLen = orig->getLen();
    fAttrib = orig->getAttrib();
    fCat = orig->getCat();
    fID = orig->getID();
    }

PilotRecord& PilotRecord::operator=(PilotRecord& orig)
    {
    if(fData)
	delete [] fData;
    fData = new char[orig.getLen()];
    memcpy(fData, orig.getData(), orig.getLen());
    fLen = orig.getLen();
    fAttrib = orig.getAttrib();
    fCat = orig.getCat();
    fID = orig.getID();
    return *this;
    }

void PilotRecord::setData(const char* data, int len)
    {
    if(fData)
	delete [] fData;
    fData = new char[len];
    memcpy(fData, data, len);
    fLen = len;
    }

bool PilotRecord::isDeleted() const
{
	return getAttrib() & dlpRecAttrDeleted ;
}

bool PilotRecord::isSecret() const
{
	return getAttrib() & dlpRecAttrSecret ;
}


void PilotRecord::makeDeleted()
{
	fAttrib |= dlpRecAttrDeleted ;
}



void PilotRecord::makeSecret()
{
	fAttrib |= dlpRecAttrSecret ;
}


// $Log$
// Revision 1.4  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
