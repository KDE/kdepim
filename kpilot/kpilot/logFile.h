#ifndef __LOG_FILE_H
#define __LOG_FILE_H

#include <time.h>
#include <qtextstream.h>
#include "pi-macros.h"

class CLogFile
{
public:
  CLogFile(const char* dataBase, bool writeMode = false);
  ~CLogFile();

  // Possible return values from getNextEntry  
  enum RecordStatus { Modified, New, Deleted };


  // Returns the next entry if there is one, else
  // id == 0.
  void getNextEntry(recordid_t& id, RecordStatus& status);

  // Writes id & status in a format that can be read in with getNextEntry()
  void writeEntry(recordid_t id, RecordStatus status);

private:
  QFile fLogFile;
  QTextStream fTextStream;
};

#endif
