#include <iostream.h>
#include <qdir.h> // Just to avoid namespace problems in old Qt versions
#include <ksock.h>
#include <unistd.h>
#include <fcntl.h>

#include "baseConduit.moc"
#include "statusMessages.h"
#include "pilotRecord.h"

BaseConduit::BaseConduit(eConduitMode mode)
  : QObject(), fMode(mode), fDaemonSocket(0L)//, fReportData(false)
{
  if((mode == BaseConduit::HotSync) || (mode == BaseConduit::Backup))
    {
      cout << "Creating kpilotlink connection..." << endl;
      fDaemonSocket = new KSocket("localhost", KPILOTLINK_PORT); // KPilotDaemon Status Socket
      fcntl(fDaemonSocket->socket(), F_SETFL, O_APPEND);
    }
}

BaseConduit::~BaseConduit()
{
  if(fDaemonSocket)
    delete fDaemonSocket;
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

