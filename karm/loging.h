#ifndef _LOGING_H_
#define _LOGING_H_

#include "preferences.h"
#include <qstring.h>

class Task;

class Loging {

private:
  Preferences *_preferences;
  static Loging *_instance;
  void log( Task *task, short type, long minutes = 0);

public:
  static Loging *instance();
  Loging();
  ~Loging();
  void start( Task *task);
  void stop( Task *task);
  void newTotalTime( Task *task, long minutes);
  void newSessionTime( Task *task, long minutes);
  QString constructTaskName(Task *task);
  QString escapeXML( QString string);

};

#endif



