#include <iostream.h>
#include <qdir.h>
#include <qfile.h>
#include <kapp.h>
#include "logFile.h"

CLogFile::CLogFile(const char* dataBase, bool writeMode)
{
  QString fileName = kapp->localkdedir();
  fileName += "/share/apps/kpilot/DBBackup/";
  fileName += dataBase;
  fileName += ".log";

  fLogFile.setName(fileName);
  if(writeMode)
    fLogFile.open(IO_WriteOnly | IO_Truncate);
  else
    if(!fLogFile.open(IO_ReadOnly))
      {
	cerr << "Could not open " << dataBase << "'s log file." << endl;
	return;
      }
  fTextStream.setDevice(&fLogFile);
}

CLogFile::~CLogFile()
{
  fLogFile.close();
}

// Possible return values from getNextEntry  
enum RecordStatus { Modified, New, Deleted };


// Returns the next entry if there is one, else
// id == 0.
void 
CLogFile::getNextEntry(recordid_t& id, RecordStatus& status)
{
  if(!fTextStream.eof())
    {
      fTextStream >> id;
      fTextStream >> (int&)status;
    }
  else
    id = 0;
}

// Writes id & status in a format that can be read in with getNextEntry()
void
CLogFile::writeEntry(recordid_t id, RecordStatus status)
{
  fTextStream << id;
  fTextStream << status << endl;
}
