#ifndef _KPILOT_VCAL_CONDUIT_H
#define _KPILOT_VCAL_CONDUIT_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the vcal-conduit plugin.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <libkcal/event.h>
#include "vcal-conduitbase.h"

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotDateEntry;


class VCalConduit : public VCalConduitBase
{
Q_OBJECT
public:
	VCalConduit( KPilotDeviceLink *, const char *name=0L,
		const QStringList &args = QStringList() );
	virtual ~VCalConduit();

	class VCalData : public VCalConduitBase::VCalDataBase
	{
	public:
		VCalData( RecordConduit *conduit, VCalConduitSettings *cfg );
		virtual ~VCalData();
		virtual bool initData();
		virtual QString description() const { return i18n("calendar"); }
	}; 
	
	
protected:
	virtual PCData*initializePCData();
	virtual QString dbName() const { return CSL1("DatebookDB"); };
	virtual VCalConduitSettings *config();
	virtual const QString getTitle(PilotAppCategory*de);
	virtual PilotAppCategory *createPalmEntry( PilotRecord *rec );

	enum {
		eqFlagsDesc=0x04,
		eqFlagsDates=0x08,
		eqFlagsRecurrence=0x10,
		eqFlagsAlarm=0x20,
		eqFlagsSecrecy=0x40,
	};
	virtual bool _equal( const PilotAppCategory *palmEntry, const PCEntry *pcEntry, 
				int flags = eqFlagsAlmostAll ) const;
	virtual bool _copy( PilotAppCategory *toPalmEntry, const PCEntry *fromPCEntry );
	virtual bool _copy( PCEntry *toPCEntry, const PilotAppCategory *fromPalmEntry );
// 	virtual bool smartMergeEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry, 
// 		PilotAppCategory *palmEntry );
	
	void setStartEndTimes( KCal::Event *toIncidence, const PilotDateEntry *fromRecord );
	void setAlarms( KCal::Event *toIncidence, const PilotDateEntry *fromRecord );
	void setRecurrence( KCal::Event *toIncidence, const PilotDateEntry *fromRecord );
	void setExceptions( KCal::Event *toIncidence, const PilotDateEntry *fromRecord );

	void setStartEndTimes( PilotDateEntry *toRecord, const KCal::Event *fromIncidence );
	void setAlarms( PilotDateEntry *toRecord, const KCal::Event *fromIncidence );
	void setRecurrence( PilotDateEntry *toRecord, const KCal::Event *fromIncidence );
	void setExceptions( PilotDateEntry *toRecord, const KCal::Event *fromIncidence );

} ;

#endif
