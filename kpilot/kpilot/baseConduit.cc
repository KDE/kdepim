// baseConduit.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
// This is baseConduit.cc for KDE 2 / KPilot 4
 
 
 
// REVISION HISTORY
//
// 3.1b9        By Dan Pilone
// 3.1.14	By Adriaan de Groot. Added addSyncLogMessage(). Some
//		code cleanup.
// 4.0.0	By Adriaan de Groot. Stripped out includes. Moved
//		everything to QString. Ported to KDE2.

#include "options.h"

#ifdef KDE2
#include <stream.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <qpixmap.h>
#include <ksock.h>

#include "baseConduit.moc"
#include "statusMessages.h"
#else
#include <iostream.h>
#include <qdir.h> // Just to avoid namespace problems in old Qt versions
#include <ksock.h>
#include <unistd.h>
#include <fcntl.h>

#include "baseConduit.moc"
#include "statusMessages.h"
#include "pilotRecord.h"
#include "kpilot.h"
#endif

static const char *id="$Id$";

BaseConduit::BaseConduit(eConduitMode mode)
  : QObject(), fMode(mode), fDaemonSocket(0L)//, fReportData(false)
{
	FUNCTIONSETUP;

	if((mode == BaseConduit::HotSync) || 
		(mode == BaseConduit::Backup))
	{
		if (debug_level & SYNC_MINOR)
		{
			cerr << fname
				<< ": Creating kpilotlink connection"
				<< endl;
		}

		// KPilotDaemon Status Socket
		fDaemonSocket = new KSocket("localhost", KPILOTLINK_PORT); 
		if (fDaemonSocket == 0L)
		{
			cerr << fname
				<< ": Can't create socket"
				<< endl ;
			fMode=BaseConduit::Error;
			return;
		}
		if (fDaemonSocket->socket()<0)
		{
			cerr << fname
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

// For a description of the protocol used to
// send log messages, see the other end of the
// link in kpilotlink.cc.
//
//
int BaseConduit::addSyncLogMessage(const char *s)
{
	FUNCTIONSETUP;

	char buffer[64];
	const char *check_s;
	char *bufp;
	int l,r;

	// Sanity check the message.
	//
	//
	if (s == 0L) return 0;
	l=strlen(s);
	if (l>30) return 0;
	check_s=s;
	while (*check_s)
	{
		if ((!isprint(*check_s)) || 
			(*check_s == '\r') ||
			(*check_s == '\n'))
			return 0;
		check_s++;
	}

	memset(buffer,0,64);
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
	

// Returns 0L if no more modified records.  User must delete
// the returned record when finished with it.
PilotRecord* 
BaseConduit::readNextModifiedRecord()
{
  int result = 0;

  write(fDaemonSocket->socket(), &CStatusMessages::NEXT_MODIFIED_REC, sizeof(int));
  if(read(fDaemonSocket->socket(), &result, sizeof(int)))
    {
      if(result == CStatusMessages::NO_SUCH_RECORD)
	{
	  cout << "BaseConduit::nextModifiedRecord() - Got NO_SUCH_RECORD" << endl;
	  return 0L;
	}
      else
	return getRecord(fDaemonSocket);
    }
  else
    {
      cout << "CBaseConduit::nextModifiedRecord() - Failure on read??" << endl;
      return 0L;
    }
}

// Returns 0L if no more records in category.  User must delete
// the returned record when finished with it.
PilotRecord*
BaseConduit::readNextRecordInCategory(int category)
{
  int result = 0;
  write(fDaemonSocket->socket(), &CStatusMessages::NEXT_REC_IN_CAT, sizeof(int));
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

  write(fDaemonSocket->socket(), &CStatusMessages::READ_REC_BY_ID, sizeof(int));
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

  write(fDaemonSocket->socket(), &CStatusMessages::READ_REC_BY_INDEX, sizeof(int));
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

  write(fDaemonSocket->socket(), &CStatusMessages::WRITE_RECORD, sizeof(int));
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
  
  write(theSocket->socket(), &CStatusMessages::REC_DATA, sizeof(int));
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
  cout << "Read:" << endl;
  cout << "\tlen: " << len << endl;
  cout << "\tattrib: " << hex<< attrib << endl;
  cout << "\tcat: " << cat << endl;
  cout << "\tuid: " << hex << uid << endl;
  return newRecord;
}


#include "kpilot_conduit.xpm"

/* virtual */ QPixmap *BaseConduit::icon() const
{
	return new QPixmap((const char **)kpilot_conduit);
}

