/* baseConduit.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This file defines the base class for all conduits, including
** various utility functions they all need.
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
#include "options.h"

#include <unistd.h>
#include <qpixmap.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdebug.h>
#include <assert.h>

#include "kpilotConfig.h"
#include "pilotLocalDatabase.h"
#include "pilotConduitDatabase.h"

#include "baseConduit.moc"

static const char *baseconduit_id="$Id$";

BaseConduit::BaseConduit(eConduitMode mode)
      : QObject(), fMode(mode), fDB(0L), fDBSource(ConduitSocket)
{
}

BaseConduit::BaseConduit(eConduitMode mode, DatabaseSource source)
      : QObject(), fMode(mode), fDB(0L), fDBSource(source)
{
}

void BaseConduit::init()
    {
	FUNCTIONSETUP;

	// in case the user calls this twice
	if (fDB)
	    return ;

	if (fDBSource == ConduitSocket)
	    {
	    fDB = new PilotConduitDatabase();
	    if (!fDB->isDBOpen())
		{
		delete fDB;
		fDB = 0L;
		}
	     
#ifdef DEBUG
	    if (debug_level & SYNC_MINOR)
		{
		kdDebug() << fname
			  << ": Creating kpilotlink connection"
			  << endl;
		}
#endif

	    
	    }
	else
	    if( fDBSource == Local )
	    {
	    QString dbPath = KPilotConfig::getDefaultDBPath();
	    QString localDB = dbInfo();
	    fDB = new PilotLocalDatabase( dbPath, localDB );
	    if (!fDB->isDBOpen())
		{
		delete fDB;
		fDB = 0L;
		}
	    
	    }
}

BaseConduit::~BaseConduit()
    {
    delete fDB;
    fDB = 0L;
    }

int BaseConduit::getDebugLevel(KConfig& c)
{
	return KPilotConfig::getDebugLevel(c);
	/* NOTREACHED */
	(void) baseconduit_id;
}


bool BaseConduit::addSyncLogMessage(const char *s)
    {
    PilotConduitDatabase *conduitDb = dynamic_cast<PilotConduitDatabase *>(fDB);
    if (conduitDb)
	return conduitDb->addSyncLogMessage(s);
    return false;
    }


int BaseConduit::readAppInfo(unsigned char *buffer)
    {
    assert(fDB);
    return fDB->readAppBlock(buffer, sizeof(buffer));
    }
// Returns 0L if no more modified records.  User must delete
// the returned record when finished with it.
PilotRecord* 
BaseConduit::readNextModifiedRecord()
    {    
    assert(fDB);
    return fDB->readNextModifiedRec();
    }

// Returns 0L if no more records in category.  User must delete
// the returned record when finished with it.
PilotRecord*
BaseConduit::readNextRecordInCategory(int category)
    {
    assert(fDB);
    return fDB->readNextRecInCategory(category);
    }

// Returns 0L if ID is invalid.  User must delete the
// returned record when finished with it.
PilotRecord*
BaseConduit::readRecordById(recordid_t id)
    {
    assert(fDB);
    return fDB->readRecordById(id);
    }

// Returns 0L if index is invalid.  User must delete the
// returned record when finished with it.
PilotRecord* 
BaseConduit::readRecordByIndex(int index)
    {
    assert(fDB);
    return fDB->readRecordByIndex(index);
    }

// Writes a record to the current database.  If rec->getID() == 0,
// a new ID will be assigned and returned.  Else, rec->getID() is
// returned
recordid_t 
BaseConduit::writeRecord(PilotRecord* rec)
    {
    assert(fDB);
    return fDB->writeRecord(rec);
    }



#include "kpilot_conduit.xpm"

/* virtual */ QPixmap BaseConduit::icon() const
{
	FUNCTIONSETUP;

	KGlobal::iconLoader()->addAppDir("kpilot");
	QPixmap p = KGlobal::iconLoader()->loadIcon("conduit",
		KIcon::Toolbar,0,KIcon::DefaultState,0,true);
	if (p.isNull())
	{
		kdWarning() << __FUNCTION__ 
			<< ": Conduit icon not found."
			<< endl;
		p = QPixmap((const char **)kpilot_conduit);
	}
	return p;
}


bool BaseConduit::getFirstTime(KConfig& c)
{
	bool b = c.readBoolEntry("FirstTime",true);
	if (b) return b;

	KConfigGroupSaver g(&c,QString::null);
	b = c.readBoolEntry("ForceFirst",false);

	return b;
}

void BaseConduit::setFirstTime(KConfig& c,bool b)
{
	c.writeEntry("FirstTime",b);
}


// $Log$
// Revision 1.20  2001/03/29 21:41:49  stern
// Added local database support in the command line for conduits
//
// Revision 1.19  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.18  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.17  2001/03/02 16:59:35  adridg
// Added new protocol message READ_APP_INFO for conduit->daemon communication
//
// Revision 1.16  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.15  2001/02/08 08:13:44  habenich
// exchanged the common identifier "id" with source unique <sourcename>_id for --enable-final build
//
// Revision 1.14  2001/02/05 20:55:07  adridg
// Fixed copyright headers for source releases. No code changed
//
// Revision 1.13  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.12  2000/12/21 00:42:50  adridg
// Mostly debugging changes -- added EFUNCTIONSETUP and more #ifdefs. KPilot should now compile -DNDEBUG or with DEBUG undefined
//
// Revision 1.11  2000/12/06 22:22:51  adridg
// Debug updates
//
// Revision 1.10  2000/11/27 02:20:20  adridg
// Internal cleanup
//
// Revision 1.9  2000/11/17 08:31:59  adridg
// Minor changes
//
// Revision 1.8  2000/11/14 23:01:51  adridg
// Proper first-time handling
//
// Revision 1.7  2000/11/14 06:32:26  adridg
// Ditched KDE1 stuff
//
