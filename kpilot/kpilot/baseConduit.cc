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

#include <iostream.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <qpixmap.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <ksock.h>
#include <kconfig.h>
#include <kdebug.h>

#include "kpilotConfig.h"

#include "baseConduit.moc"
#include "statusMessages.h"

static const char *baseconduit_id="$Id$";

BaseConduit::BaseConduit(eConduitMode mode)
  : QObject(), fMode(mode), fDaemonSocket(0L)//, fReportData(false)
{
	FUNCTIONSETUP;

	if((mode == BaseConduit::HotSync) || 
		(mode == BaseConduit::Backup))
	{
#ifdef DEBUG
		if (debug_level & SYNC_MINOR)
		{
			kdDebug() << fname
				<< ": Creating kpilotlink connection"
				<< endl;
		}
#endif

		// KPilotDaemon Status Socket
		fDaemonSocket = new KSocket("localhost", KPILOTLINK_PORT); 
		if (fDaemonSocket == 0L)
		{
			kdError() << __FUNCTION__
				<< ": Can't create socket"
				<< endl ;
			fMode=BaseConduit::Error;
			return;
		}
		if (fDaemonSocket->socket()<0)
		{
			kdError() << __FUNCTION__
				<< ": Socket is not connected"
				<< endl ;
			fMode=BaseConduit::Error;
			return;
		}
		fcntl(fDaemonSocket->socket(), F_SETFL, O_APPEND);
	}
}

BaseConduit::~BaseConduit()
{
	if(fDaemonSocket)
	{
		delete fDaemonSocket;
		fDaemonSocket=0L;
	}
}

int BaseConduit::getDebugLevel(KConfig& c)
{
	return KPilotConfig::getDebugLevel(c);
	/* NOTREACHED */
	(void) baseconduit_id;
}

// For a description of the protocol used to
// send log messages, see the other end of the
// link in kpilotlink.cc.
//
//
#define SYNC_MSG_SIZE	(64)

int BaseConduit::addSyncLogMessage(const char *s)
{
	FUNCTIONSETUP;

	char buffer[SYNC_MSG_SIZE];
	const char *check_s;
	char *bufp;
	int l,r;

	// Sanity check the message.
	//
	//
	if (s == 0L) return 0;
	l=strlen(s);
	if (l>(SYNC_MSG_SIZE / 2)) return 0;
	check_s=s;
	while (*check_s)
	{
		if ((!isprint(*check_s)) || 
			(*check_s == '\r') ||
			(*check_s == '\n'))
			return 0;
		check_s++;
	}

	memset(buffer,0,SYNC_MSG_SIZE);
	// Make l a multiple of sizeof(int)
	//
	//
	if (l % sizeof(int))
	{
		l/=sizeof(int);
		l*=sizeof(int);
		l+=sizeof(int);
	}

	bufp=buffer;
	*((int *)bufp)=CStatusMessages::LOG_MESSAGE;
	bufp+=sizeof(int);
	*((int *)bufp)=l;
	bufp+=sizeof(int);
	strcpy(bufp,s);

	r=write(fDaemonSocket->socket(),buffer,
		2*sizeof(int)+l);

	return 1; // (r == (2*sizeof(int)+l));
}


int BaseConduit::readAppInfo(unsigned char *buffer)
{
	FUNCTIONSETUP;

	int len = 0;
	int recvlen;

	CStatusMessages::write(fDaemonSocket->socket(),
		CStatusMessages::READ_APP_INFO);
	if (read(fDaemonSocket->socket(),&len,sizeof(int)))
	{
		// Sanity check
		//
		//
		if ((len<0) || (len>16000))
		{
			kdError() << __FUNCTION__
				<< ": Got crazy length for READ_APP_INFO"
				<< endl;
			return 0;
		}

		if ((recvlen=read(fDaemonSocket->socket(),buffer,len))!=len)
		{
			kdWarning() << __FUNCTION__
				<< ": Expected length "
				<< len
				<< " and read "
				<< recvlen
				<< endl;
		}

		return recvlen;
	}
	else
	{
		kdWarning() << __FUNCTION__
			<< ": Got no response to READ_APP_INFO"
			<< endl;
		return 0;
	}
}
// Returns 0L if no more modified records.  User must delete
// the returned record when finished with it.
PilotRecord* 
BaseConduit::readNextModifiedRecord()
{
	FUNCTIONSETUP;

  int result = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::NEXT_MODIFIED_REC);
  if(read(fDaemonSocket->socket(), &result, sizeof(int)))
    {
      if(result == CStatusMessages::NO_SUCH_RECORD)
	{
#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname
				<< ": Got NO_SUCH_RECORD" 
				<< endl;
		}
#endif
	  return 0L;
	}
      else
	{
		return getRecord(fDaemonSocket);
	}
    }
  else
    {
      kdWarning() << __FUNCTION__ << ": Failure on read" << endl;
      return 0L;
    }
}

// Returns 0L if no more records in category.  User must delete
// the returned record when finished with it.
PilotRecord*
BaseConduit::readNextRecordInCategory(int category)
{
  int result = 0;
	CStatusMessages::write(fDaemonSocket->socket(), 
		CStatusMessages::NEXT_REC_IN_CAT);
  write(fDaemonSocket->socket(), &category, sizeof(int));
  read(fDaemonSocket->socket(), &result, sizeof(int));
  if(result == CStatusMessages::NO_SUCH_RECORD)
    return 0L;
  return getRecord(fDaemonSocket);
}

// Returns 0L if ID is invalid.  User must delete the
// returned record when finished with it.
PilotRecord*
BaseConduit::readRecordById(recordid_t id)
{
  int result = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::READ_REC_BY_ID);
  write(fDaemonSocket->socket(), &id, sizeof(recordid_t));
  read(fDaemonSocket->socket(), &result, sizeof(int));
  if(result == CStatusMessages::NO_SUCH_RECORD)
    return 0L;
  return getRecord(fDaemonSocket);
}

// Returns 0L if index is invalid.  User must delete the
// returned record when finished with it.
PilotRecord* 
BaseConduit::readRecordByIndex(int index)
{
  int result = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::READ_REC_BY_INDEX);
  write(fDaemonSocket->socket(), &index, sizeof(int));
  read(fDaemonSocket->socket(), &result, sizeof(int));
  if(result == CStatusMessages::NO_SUCH_RECORD)
    return 0L;
  return getRecord(fDaemonSocket);
}

// Writes a record to the current database.  If rec->getID() == 0,
// a new ID will be assigned and returned.  Else, rec->getID() is
// returned
recordid_t 
BaseConduit::writeRecord(PilotRecord* rec)
{
  int result = 0;
  recordid_t id = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::WRITE_RECORD);
  writeRecord(fDaemonSocket, rec);
  read(fDaemonSocket->socket(), &result, sizeof(int));
  read(fDaemonSocket->socket(), &id, sizeof(recordid_t));
  return id;
}


void
BaseConduit::writeRecord(KSocket* theSocket, PilotRecord* rec)
{
  int len = rec->getLen();
  int attrib = rec->getAttrib();
  int cat = rec->getCat();
  recordid_t uid = rec->getID();
  char* data = rec->getData();
  
  CStatusMessages::write(theSocket->socket(), 
  	CStatusMessages::REC_DATA);
  write(theSocket->socket(), &len, sizeof(int));
  write(theSocket->socket(), &attrib, sizeof(int));
  write(theSocket->socket(), &cat, sizeof(int));
  write(theSocket->socket(), &uid, sizeof(recordid_t));
  write(theSocket->socket(), data, len);
}

// REC_DATA has already been read!  This just grabs the actual
// record.
PilotRecord*
BaseConduit::getRecord(KSocket* in)
{
	FUNCTIONSETUP;

  int len, attrib, cat;
  recordid_t uid;
  char* data;
  PilotRecord* newRecord;

  read(in->socket(), &len, sizeof(int));
  read(in->socket(), &attrib, sizeof(int));
  read(in->socket(), &cat, sizeof(int));
  read(in->socket(), &uid, sizeof(recordid_t));
  data = new char[len];
  read(in->socket(), data, len);
  newRecord = new PilotRecord((void*)data, len, attrib, cat, uid);
  delete [] data;
#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
  		kdDebug() << fname
			<< ": Read"
			<< " len=" << len
			<< " att=" << attrib
			<< " cat=" << cat 
			<< " UID=" << uid
			<< endl;
	}
#endif
  return newRecord;
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
