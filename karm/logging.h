#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "preferences.h"
#include <qstring.h>

class Task;

/**
 * Log changes to a file.
 */

class Logging {

private:
  Preferences *_preferences;
  static Logging *_instance;
  void log( Task *task, short type, long minutes = 0);

public:
  static Logging *instance();
  Logging();
  ~Logging();
  void start( Task *task);
  void stop( Task *task);
  void newTotalTime( Task *task, long minutes);
  void newSessionTime( Task *task, long minutes);
  QString constructTaskName(Task *task);
  QString escapeXML( QString string);

};

#endif



