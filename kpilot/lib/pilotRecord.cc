/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a wrapper for pilot-link's general
** Pilot database structures. These records are
*** just collections of bits. See PilotAppCategory
** for interpreting the bits in a meaningful way.
**
** As a crufty hack, the non-inline parts of
** PilotAppCategory live in this file as well.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "options.h"

#include <string.h>

#include <qtextcodec.h>
#include <qregexp.h>

#include <kglobal.h>
#include <kcharsets.h>

// PilotAppCategory includes pilotRecord and we
// provide its implementation here as well.
//
#include "pilotAppCategory.h"



static const char *pilotRecord_id =
	"$Id$";

/* static */ int PilotRecord::fAllocated = 0;
/* static */ int PilotRecord::fDeleted = 0;

/* static */ void PilotRecord::allocationInfo()
{
#ifdef DEBUG
	FUNCTIONSETUP;
	DEBUGKPILOT << fname
		<< ": Allocated " << fAllocated
		<< "  Deleted " << fDeleted;
#endif
}

PilotRecord::PilotRecord(void *data, int len, int attrib, int cat,
	pi_uid_t uid) :
	fData(0L),
	fLen(len),
	fAttrib(attrib),
	fCat(cat),
	fID(uid),
	fBuffer(0L)
{
	FUNCTIONSETUP;
	fData = new char[len];

	memcpy(fData, data, len);

	fAllocated++;
	(void) pilotRecord_id;
}

PilotRecord::PilotRecord(PilotRecord * orig) :
	fBuffer(0L)
{
	FUNCTIONSETUP;
	fData = new char[orig->getLen()];

	memcpy(fData, orig->getData(), orig->getLen());
	fLen = orig->getLen();
	fAttrib = orig->getAttrib();
	fCat = orig->category();
	fID = orig->id();

	fAllocated++;
}

PilotRecord & PilotRecord::operator = (PilotRecord & orig)
{
	FUNCTIONSETUP;
	if (fBuffer)
	{
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
		kdWarning() << kde_funcinfo << ": Uninitialized pi_buffer" << endl;
#else
		pi_buffer_free(fBuffer);
		fBuffer=0L;
		fData=0L;
#endif
	}

	if (fData)
		delete[]fData;
	fData = new char[orig.getLen()];

	memcpy(fData, orig.getData(), orig.getLen());
	fLen = orig.getLen();
	fAttrib = orig.getAttrib();
	fCat = orig.category();
	fID = orig.id();
	return *this;
}

recordid_t PilotRecord::getID() const
{
	return id();
}

void PilotRecord::makeDeleted()
{
	setDeleted(true);
}

void PilotRecord::makeSecret()
{
	setSecret(true);
}

int PilotRecord::getCat() const
{
	return category();
}

void PilotRecord::setCat(int i)
{
	return setCategory(i);
}

void PilotRecord::setData(const char *data, int len)
{
	FUNCTIONSETUP;
	if (fData)
		delete[]fData;
	fData = new char[len];

	memcpy(fData, data, len);
	fLen = len;
}


/* static */ QTextCodec *PilotAppCategory::pilotCodec = 0L;

/* static */ QTextCodec *PilotAppCategory::setupPilotCodec(const QString &s)
{
	FUNCTIONSETUP;
	QString encoding(KGlobal::charsets()->encodingForName(s));

#ifdef DEBUG
	DEBUGKPILOT << fname << ": Creating codec " << encoding << endl;
#endif

	// if the desired codec can't be found, latin1 will be returned anyway, no need to do this manually
	pilotCodec = KGlobal::charsets()->codecForName(encoding);

#ifdef DEBUG
	DEBUGKPILOT << fname
		<< ": Got codec " << codecName() << " for setting "
		<< s << endl;
#endif
	return codec();
}

/* static */ QString PilotAppCategory::codecName()
{
	return codec()->name();
}
