/* pilotConduitDatabase.h			KPilot
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

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#include <fcntl.h>
#include <ctype.h>


#ifndef _KSOCK_H
#include <ksock.h>
#endif



#ifndef _KPILOT_PILOTCONDUITDATABASE_H
#include "pilotConduitDatabase.h"
#endif
#ifndef _KPILOT_STATUSMESSAGES_H
#include "statusMessages.h"
#endif

PilotConduitDatabase::PilotConduitDatabase() : PilotDatabase(),
      fDaemonSocket(0L)
    {
    openDatabase();
    }
PilotConduitDatabase::~PilotConduitDatabase()
    {
    closeDatabase();
    }
      
/** Reads the application block info, returns size. */
int PilotConduitDatabase::readAppBlock(unsigned char* buffer, int )
    {
    FUNCTIONSETUP;

    int len = 0;
    int recvlen = 0;
    
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

/** Writes the application block info. */
int PilotConduitDatabase::writeAppBlock(unsigned char* , int )
    {
    qDebug("PilotConduitDatabase::writeAppBlock not implemented yet");
    return 0;
    }

PilotRecord*
PilotConduitDatabase::readRecordById(recordid_t id)
{
  int result = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::READ_REC_BY_ID);
  write(fDaemonSocket->socket(), &id, sizeof(recordid_t));
  read(fDaemonSocket->socket(), &result, sizeof(int));
  if(result == CStatusMessages::NO_SUCH_RECORD)
    return 0L;
  return _getRecord();
}

// Returns 0L if index is invalid.  User must delete the
// returned record when finished with it.
PilotRecord* 
PilotConduitDatabase::readRecordByIndex(int index)
{
  int result = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::READ_REC_BY_INDEX);
  write(fDaemonSocket->socket(), &index, sizeof(int));
  read(fDaemonSocket->socket(), &result, sizeof(int));
  if(result == CStatusMessages::NO_SUCH_RECORD)
    return 0L;
  return _getRecord();
}

// Writes a record to the current database.  If rec->getID() == 0,
// a new ID will be assigned and returned.  Else, rec->getID() is
// returned
recordid_t PilotConduitDatabase::writeRecord(PilotRecord* rec)
{
  int result = 0;
  recordid_t id = 0;

  CStatusMessages::write(fDaemonSocket->socket(), 
  	CStatusMessages::WRITE_RECORD);
  _writeRecord(rec);
  read(fDaemonSocket->socket(), &result, sizeof(int));
  read(fDaemonSocket->socket(), &id, sizeof(recordid_t));
  return id;
}

/** Reads the next record from database in category 'category' */
PilotRecord* PilotConduitDatabase::readNextRecInCategory(int category)
    {
    int result = 0;
    CStatusMessages::write(fDaemonSocket->socket(), 
			   CStatusMessages::NEXT_REC_IN_CAT);
    write(fDaemonSocket->socket(), &category, sizeof(int));
    read(fDaemonSocket->socket(), &result, sizeof(int));
    if(result == CStatusMessages::NO_SUCH_RECORD)
	return 0L;
    return _getRecord();
    }

/** Reads the next record from database that has the dirty flag set. */
PilotRecord* PilotConduitDatabase::readNextModifiedRec()
    {
    FUNCTIONSETUP;
    
    int result = 0;
    
    CStatusMessages::write(fDaemonSocket->socket(), 
			   CStatusMessages::NEXT_MODIFIED_REC);
    if(read(fDaemonSocket->socket(), &result, sizeof(int)))
	{
	if(result == CStatusMessages::NO_SUCH_RECORD)
	    {
		kdDebug() << fname
			  << ": Got NO_SUCH_RECORD" 
			  << endl;
	    return 0L;
	    }
	else
	    {
	    return _getRecord();
	    }
	}
    else
	{
	kdWarning() << __FUNCTION__ << ": Failure on read" << endl;
	return 0L;
	}
    }
/** Writes a new ID to the record specified the index.  Not supported on Serial connections */
recordid_t PilotConduitDatabase::writeID(PilotRecord* )
    {
    qDebug("PilotConduitDatabase::writeID not implemented yet");
    return 0;
    }

void PilotConduitDatabase::openDatabase()
    {
    FUNCTIONSETUP;
    if (isDBOpen())
	return;
    
    // KPilotDaemon Status Socket
    fDaemonSocket = new KSocket("localhost", KPILOTLINK_PORT); 
    if (fDaemonSocket == 0L)
	{
	kdError() << __FUNCTION__
		  << ": Can't create socket"
		  << endl ;
	return;
	}
    if (fDaemonSocket->socket()<0)
	{
	kdError() << __FUNCTION__
		  << ": Socket is not connected"
		  << endl ;
	return;
	}
    fcntl(fDaemonSocket->socket(), F_SETFL, O_APPEND);
    setDBOpen(true);
    }

void PilotConduitDatabase::closeDatabase()
    {
    if (!isDBOpen())
	return;
    
    delete fDaemonSocket;
    fDaemonSocket=0L;
    setDBOpen(false);
    }

void PilotConduitDatabase::_writeRecord(PilotRecord* rec)
    {
    int len = rec->getLen();
    int attrib = rec->getAttrib();
    int cat = rec->getCat();
    recordid_t uid = rec->getID();
    char* data = rec->getData();
    
    CStatusMessages::write(fDaemonSocket->socket(), 
			   CStatusMessages::REC_DATA);
    write(fDaemonSocket->socket(), &len, sizeof(int));
    write(fDaemonSocket->socket(), &attrib, sizeof(int));
    write(fDaemonSocket->socket(), &cat, sizeof(int));
    write(fDaemonSocket->socket(), &uid, sizeof(recordid_t));
    write(fDaemonSocket->socket(), data, len);
    }

// REC_DATA has already been read!  This just grabs the actual
// record.
PilotRecord* PilotConduitDatabase::_getRecord()
    {
    FUNCTIONSETUP;
    
    int len, attrib, cat;
    recordid_t uid;
    char* data;
    PilotRecord* newRecord;
    
    read(fDaemonSocket->socket(), &len, sizeof(int));
    read(fDaemonSocket->socket(), &attrib, sizeof(int));
    read(fDaemonSocket->socket(), &cat, sizeof(int));
    read(fDaemonSocket->socket(), &uid, sizeof(recordid_t));
    data = new char[len];
    read(fDaemonSocket->socket(), data, len);
    newRecord = new PilotRecord((void*)data, len, attrib, cat, uid);
    delete [] data;
	kdDebug() << fname
		  << ": Read"
		  << " len=" << len
		  << " att=" << attrib
		  << " cat=" << cat 
		  << " UID=" << uid
		  << endl;
    return newRecord;
    }


// For a description of the protocol used to
// send log messages, see the other end of the
// link in kpilotlink.cc.
//
//
#define SYNC_MSG_SIZE	(64)

bool PilotConduitDatabase::addSyncLogMessage(const char *s)
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
			return false;
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

	return true; // (r == (2*sizeof(int)+l));
}
