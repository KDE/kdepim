#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "preferences.h"
#include <qstring.h>
#include <qdatetime.h>

class Task;

#define DONT_LOG false

class KarmLogEvent
{
 protected:
   QString name;
   // full name includes all parents
   QString fullName;
   QDateTime eventTime;
   void loadCommonFields( Task *task);
   // this really doesn't belong here ...
   QString escapeXML( const QString &string);
   
 public:
   virtual QString toXML(void) =0;
   // needed to avoid compiler warnings about subclasses having
   // "virtual functions but non-virtual destructor."
   virtual ~KarmLogEvent () {};
};

class StartLogEvent: public KarmLogEvent
{
 public:
   StartLogEvent( Task *task);
   QString toXML();
};

class StopLogEvent: public KarmLogEvent
{
 public:
   StopLogEvent ( Task *task);
   QString toXML();
};

class RenameLogEvent: public KarmLogEvent
{
 private:
   QString oldName;
 public:
   RenameLogEvent( Task *task, QString& old);
   QString toXML();
};

class SessionTimeLogEvent: public KarmLogEvent
{
 private:
   long newTotal, delta;
 public:
   SessionTimeLogEvent( Task *task, long newTotal, long delta);
   QString toXML();
};

class TotalTimeLogEvent: public KarmLogEvent
{
 private:
   long newTotal, delta;
 public:
   TotalTimeLogEvent( Task *task, long newTotal, long delta );
   QString toXML();
};


class Logging
{
 private:
   Preferences *_preferences;
   static Logging *_instance;
   void log( KarmLogEvent* event );

 public:
   static Logging *instance();
   Logging();
   ~Logging();
   void start( Task *task);
   void stop( Task *task);
   void rename( Task *task, QString& oldName);
   void newTotalTime( Task *task, long minutes, long change);
   void newSessionTime( Task *task, long minutes, long change);
};

#endif
