/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "pilotRecord.h"

#include <string.h>

#include <QtCore/QRegExp>

#include <kglobal.h>
#include <kcharsets.h>

#include "options.h"
#include "pilot.h"

/* virtual */ QString PilotRecordBase::textRepresentation() const
{
	return CSL1("[ %1,%2,%3 ]")
		.arg( attributes() )
		.arg( category() )
		.arg( id() );
}

/* virtual */ QString PilotRecord::textRepresentation() const
{
	return CSL1("[ %1,%2 ]")
		.arg(PilotRecordBase::textRepresentation())
		.arg(size());
}



/* static */ int PilotRecord::fAllocated = 0;
/* static */ int PilotRecord::fDeleted = 0;

/* static */ void PilotRecord::allocationInfo()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "Allocated " << fAllocated << "  Deleted " << fDeleted;
}

PilotRecord::PilotRecord(void *data, int len, int attrib, int cat, recordid_t uid) :
	PilotRecordBase(attrib,cat,uid),
	fData(0L),
	fLen(len),
	fBuffer(0L)
{
	FUNCTIONSETUPL(4);
	fData = new char[len];

	memcpy(fData, data, len);

	fAllocated++;
}

PilotRecord::PilotRecord(PilotRecord * orig) :
	PilotRecordBase( orig->attributes(), orig->category(), orig->id() ) ,
	fBuffer(0L)
{
	FUNCTIONSETUPL(4);
	fData = new char[orig->size()];

	memcpy(fData, orig->data(), orig->size());
	fLen = orig->size();
	fAllocated++;
}

PilotRecord & PilotRecord::operator = (PilotRecord & orig)
{
	FUNCTIONSETUP;
	if (fBuffer)
	{
		pi_buffer_free(fBuffer);
		fBuffer=0L;
		fData=0L;
	}

	if (fData)
		delete[]fData;
	fData = new char[orig.size()];

	memcpy(fData, orig.data(), orig.size());
	fLen = orig.size();
	setAttributes( orig.attributes() );
	setCategory( orig.category() );
	setID( orig.id() );
	return *this;
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

