#include <qdatetime.h>
#include <qfile.h>
#include <qstring.h>
#include <qstylesheet.h>
#include <qtextstream.h>

#include "kdebug.h"             // kdWarning

#include "logging.h"
#include "preferences.h"
#include "task.h"

#define QSl1(c) QString::fromLatin1(c)

//
//                            L O G    E V E N T S
//
QString KarmLogEvent::escapeXML(const QString& string )
{
  QString result = QStyleSheet::escape(string); // escapes <, >, &
  result.replace( '\'', QSl1("&apos;") );
  result.replace( '\"', QSl1("&quot;") );
  // protect also our task-separator
  result.replace( '/',  QSl1("&slash;"));

  return result;
}

//------------------------ Common LogEvent stuff ------------------------
//
void KarmLogEvent::loadCommonFields( Task *task)
{
  eventTime =  QDateTime::currentDateTime();
  
  fullName = escapeXML(task->name());
  while( ( task = task->parent() ) )
  {
    fullName = escapeXML(task->name()) + fullName.prepend('/');
  }
}

//------------------------- Start Task LogEvent ----------------------------
//
StartLogEvent::StartLogEvent( Task *task )
{
  loadCommonFields(task);
}

QString StartLogEvent::toXML()
{
  return QSl1("<starting     "
              " date=\"%1\""
              " task=\"%2\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName );
}

//------------------------- Stop Task LogEvent ---------------------------
//
StopLogEvent::StopLogEvent( Task *task )
{
  loadCommonFields(task);
}

QString StopLogEvent::toXML()
{
  return QSl1("<stopping     "
              " date=\"%1\""
              " task=\"%2\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName );
}

//------------------------ Rename Task LogEvent ---------------------------
//
RenameLogEvent::RenameLogEvent( Task *task, QString& old )
{
  loadCommonFields(task);
  oldName = old;
}

QString RenameLogEvent::toXML()
{
  return QSl1("<renaming     "
              " date=\"%1\""
              " task=\"%2\""
              " old_name=\"%3\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName ).arg(
                    oldName );
}

//---------------------- Change Times LogEvent ---------------------------
// 
TimechangeLogEvent::TimechangeLogEvent( Task *task, long changeSession,
                                                    long changeTime)
{
  loadCommonFields(task);
  deltaSession = changeSession;
  deltaTime    = changeTime;
  newSession = task->sessionTime();
  newTime    = task->time();
}

QString TimechangeLogEvent::toXML()
{
  return QSl1("<timechange   "
              " date=\"%1\""
              " task=\"%2\""
              " new_session=\"%3\""
              " change_session=\"%4\""
              " new_time=\"%5\""
              " change=\"%6\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName     ).arg(
                    newSession   ).arg(
                    deltaSession ).arg(
                    newTime      ).arg(
                    deltaTime    );
}

//------------------- Add Comment to Task LogEvent ----------------------
// 
CommentLogEvent::CommentLogEvent( Task *task, QString& newComment)
{
  loadCommonFields(task);
  comment = newComment;
}

QString CommentLogEvent::toXML()
{
  return QSl1("<comment      "
              " date=\"%1\""
              " task=\"%2\""
              " text=\"%3\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName ).arg(
                    comment );
}

//--------------------- Remove Task LogEvent ---------------------------
// 
RemoveLogEvent::RemoveLogEvent( Task *task )
{
  loadCommonFields(task);
}

QString RemoveLogEvent::toXML()
{
  return QSl1("<deleted      "
              " date=\"%1\""
              " task=\"%2\" />\n"
             ).arg( eventTime.toString() ).arg(
                    fullName );
}

//--------------------- Start Session LogEvent ---------------------------
// 
StartSessionLogEvent::StartSessionLogEvent( )
{
}

QString StartSessionLogEvent::toXML()
{
  return QSl1("<start_session"
              " date=\"%1\" />\n"
             ).arg( eventTime.toString() );
}

//--------------------- Stop Session LogEvent ---------------------------
// 
StopSessionLogEvent::StopSessionLogEvent( )
{
}

QString StopSessionLogEvent::toXML()
{
  return QSl1("<stop_session "
              " date=\"%1\" />\n"
             ).arg( eventTime.toString() );
}

//
//                              L O G G I N G
//
Logging *Logging::_instance = 0;

Logging::Logging()
{
  _preferences = Preferences::instance();
}

void Logging::start( Task *task)
{
  KarmLogEvent* event = new StartLogEvent(task);
  log(event);
}

void Logging::stop( Task *task)
{
  KarmLogEvent* event = new StopLogEvent(task);
  log(event);
}

void Logging::changeTimes( Task *task, long deltaSession, long deltaTime)
{
  KarmLogEvent* event = new TimechangeLogEvent(task, deltaSession, deltaTime);
  log(event);
}

void Logging::rename( Task *task, QString& oldName)
{
  KarmLogEvent* event = new RenameLogEvent(task, oldName);
  log(event);
}

void Logging::comment( Task *task, QString& comment)
{
  KarmLogEvent* event = new CommentLogEvent(task, comment);
  log(event);
}

void Logging::remove( Task *task )
{
  KarmLogEvent* event = new RemoveLogEvent(task);
  log(event);
}

void Logging::startSession()
{
  KarmLogEvent* event = new StartSessionLogEvent();
  log(event);
}

void Logging::stopSession()
{
  KarmLogEvent* event = new StopSessionLogEvent();
  log(event);
}

void Logging::log( KarmLogEvent* event)
{
  if(_preferences->timeLogging()) {
    QFile f(_preferences->timeLog());

    if ( f.open( IO_WriteOnly | IO_Append) ) {
      QTextStream out( &f );        // use a text stream
      out << event->toXML();
      f.close();
    } else {
      kdWarning() << "Couldn't write to time-log file" << endl;
    }
  }
}

Logging *Logging::instance()
{
  if (_instance == 0) {
    _instance = new Logging();
  }
  return _instance;
}


Logging::~Logging()
{
}

