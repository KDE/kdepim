/* pilotRecord.cc               PilotDaemon
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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

static const char *pilotRecord_id = "$Id$";

#include <config.h>
#include "../lib/debug.h"

#include "pilotRecord.h"



PilotRecord::PilotRecord(void *data, 
	int len, int attrib, int cat, pi_uid_t uid) :
	fData(0L), fLen(len), fAttrib(attrib), fCat(cat), fID(uid)
{
	fData = new char[len];

	memcpy(fData, data, len);
	(void) pilotRecord_id;
}

PilotRecord::PilotRecord(PilotRecord * orig)
{
	fData = new char[orig->getLen()];

	memcpy(fData, orig->getData(), orig->getLen());
	fLen = orig->getLen();
	fAttrib = orig->getAttrib();
	fCat = orig->getCat();
	fID = orig->getID();
}

PilotRecord & PilotRecord::operator = (PilotRecord & orig)
{
	if (fData)
		delete[]fData;
	fData = new char[orig.getLen()];

	memcpy(fData, orig.getData(), orig.getLen());
	fLen = orig.getLen();
	fAttrib = orig.getAttrib();
	fCat = orig.getCat();
	fID = orig.getID();
	return *this;
}

void PilotRecord::setData(const char *data, int len)
{
	if (fData) delete[]fData;
	fData = new char[len];

	memcpy(fData, data, len);
	fLen = len;
}

bool PilotRecord::isArchived() const
{
	return getAttrib() & dlpRecAttrArchived;
}

bool PilotRecord::isDeleted() const
{
	return getAttrib() & dlpRecAttrDeleted;
}

bool PilotRecord::isSecret() const
{
	return getAttrib() & dlpRecAttrSecret;
}


void PilotRecord::makeDeleted()
{
	fAttrib |= dlpRecAttrDeleted;
}



void PilotRecord::makeSecret()
{
	fAttrib |= dlpRecAttrSecret;
}

// $Log$
// Revision 1.1.1.1  2001/06/21 19:50:04  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
