#ifndef KARM_LOGGING_H
#define KARM_LOGGING_H

class QDateTime;
class QString;

class Preferences;
class Task;

#define DONT_LOG false

/** base class for specific log events */
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

//@{ class for loggin of specific events
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

  class TimechangeLogEvent: public KarmLogEvent
  {
   private:
     long newSession, deltaSession;
     long newTime, deltaTime;
   public:
     TimechangeLogEvent( Task *task, long deltaSession, long deltaTime);
     QString toXML();
  };

  class CommentLogEvent: public KarmLogEvent
  {
   private:
     QString comment;
   public:
     CommentLogEvent( Task *task, QString& comment);
     QString toXML();
  };

  class RemoveLogEvent: public KarmLogEvent
  {
   public:
     RemoveLogEvent( Task *task);
     QString toXML();
  };

  class StartSessionLogEvent: public KarmLogEvent
  {
   public:
     StartSessionLogEvent();
     QString toXML();
  };

  class StopSessionLogEvent: public KarmLogEvent
  {
   public:
     StopSessionLogEvent();
     QString toXML();
  };
//@}

class Logging
{
 private:
   Preferences* _preferences;
   static Logging *_instance;
   void log( KarmLogEvent* event );

 public:
   static Logging *instance();
   Logging();
   ~Logging();
   void start( Task *task );
   void stop( Task *task );
   void rename( Task *task, QString& oldName );
   void changeTimes( Task *task, long deltaSession, long deltaTotal);
   void comment( Task *task, QString& comment );
   void remove( Task *task );
   void startSession();
   void stopSession();
};

#endif // KARM_LOGGING_H
