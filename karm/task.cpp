#include <qcstring.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qtimer.h>

#include <kiconloader.h>

#include "kapplication.h"       // kapp
#include "kemailsettings.h"
#include "kdebug.h"

#include "event.h"

#include "karmutility.h"
#include "task.h"
#include "taskview.h"


const int gSecondsPerMinute = 60;


QPtrVector<QPixmap> *Task::icons = 0;

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopList desktops, TaskView *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
}

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopList desktops, Task *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
}

Task::Task( KCal::Todo* todo, TaskView* parent )
  : QObject(), QListViewItem( parent )
{
  long minutes = 0;
  QString name;
  long sessionTime = 0;
  DesktopList desktops;

  parseIncidence( todo, minutes, name, desktops );
  init( name, minutes, sessionTime, desktops );
}

void Task::init( const QString& taskName, long minutes, long sessionTime,
                 DesktopList desktops)
{
  // If our parent is the taskview then connect our totalTimesChanged
  // signal to its receiver
  if ( ! parent() )
    connect( this, SIGNAL( totalTimesChanged ( long, long ) ),
             listView(), SLOT( taskTotalTimesChanged( long, long) ));

  connect( this, SIGNAL( deletingTask( Task* ) ),
           listView(), SLOT( deletingTask( Task* ) ));

  if (icons == 0) {
    icons = new QPtrVector<QPixmap>(8);
    for (int i=0; i<8; i++)
    {
      QPixmap *icon = new QPixmap();
      QString name;
      name.sprintf("watch-%d.xpm",i);
      *icon = UserIcon(name);
      icons->insert(i,icon);
    }
  }

  _name = taskName.stripWhiteSpace();
  _lastStart = QDateTime::currentDateTime();
  _totalTime = _time = minutes;
  _totalSessionTime = _sessionTime = sessionTime;
  noNegativeTimes();
  _timer = new QTimer(this);
  _desktops = desktops;
  connect(_timer, SIGNAL(timeout()), this, SLOT(updateActiveIcon()));
  setPixmap(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
  _currentPic = 0;
  update();
  changeParentTotalTimes( _sessionTime, _time);
}

Task::~Task() {
  emit deletingTask(this);
  delete _timer;
}

void Task::setRunning( bool on, KarmStorage* storage )
{
  if (on) {
    if (!_timer->isActive()) {
      _timer->start(1000);
      storage->startTimer(this);
      _currentPic=7;
      _lastStart = QDateTime::currentDateTime();
      updateActiveIcon();
    }
  }
  else {
    if (_timer->isActive()) {
      _timer->stop();
      storage->stopTimer(this);
      setPixmap(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
    }
  }
}

void Task::setUid(QString uid) {
  _uid = uid;
}

bool Task::isRunning() const
{
  return _timer->isActive();
}

void Task::setName( const QString& name, KarmStorage* storage )
{
  kdDebug() << "Task:setName: " << name << endl;

  QString oldname = _name;
  if ( oldname != name ) {
    _name = name;
    storage->setName(this, oldname);
    update();
  }
}

void Task::setDesktopList ( DesktopList desktopList )
{
  _desktops = desktopList;
}

void Task::changeTimes( long minutesSession, long minutes, bool do_logging,
    KarmStorage* storage)
{
  if( minutesSession != 0 || minutes != 0) {
    _sessionTime += minutesSession;
    kdDebug() 
      << "Task::changeTimes: " << name() 
      << ", _sessionTime = " << minutesSession << endl;

    _time += minutes;
    if ( do_logging )
      storage->changeTime(this, minutes * gSecondsPerMinute);
      //_logging->changeTimes( this, minutesSession, minutes);

    noNegativeTimes();
    changeTotalTimes( minutesSession, minutes );
  }
}

void Task::changeTotalTimes( long minutesSession, long minutes )
{
  kdDebug() 
    << "Task::changeTotalTimes(" << minutesSession << ", "
    << minutes << ") for " << name() << endl;

  _totalSessionTime += minutesSession;
  _totalTime += minutes;
  noNegativeTimes();
  update();
  changeParentTotalTimes( minutesSession, minutes );
}

void Task::resetTimes()
{
  _totalSessionTime -= _sessionTime;
  _totalTime -= _time;
  changeParentTotalTimes( -_sessionTime, -_time);
  _sessionTime = 0;
  _time = 0;
  update();
}

void Task::changeParentTotalTimes( long minutesSession, long minutes )
{
  kdDebug() 
    << "Task::changeParentTotalTimes(" << minutesSession << ", "
    << minutes << ") for " << name() << endl;

  if ( isRoot() )
    emit totalTimesChanged( minutesSession, minutes );
  else
    parent()->changeTotalTimes( minutesSession, minutes );
}

bool Task::remove( QPtrList<Task>& activeTasks, KarmStorage* storage)
{
  bool ok = true;

  if (storage->removeTask(this))
  {
    if( isRunning() ) 
      setRunning( false, storage );

    for (Task* child= this->firstChild(); child; child= child->nextSibling() )
    {
      if (child->isRunning())
        child->setRunning(false, storage);
    }

    // original
    //for (Task* child= this->firstChild(); child; child= child->nextSibling() )
    //  child->remove( activeTasks, storage );
    // TODO: by now this tasks _totalSessionTime and _totalTime should be == 0!

    changeParentTotalTimes( -_sessionTime, -_time); 
    delete this;
  }
  else
    ok = false;

  kdDebug() << "Task::remove: ok = " << ok << endl;

  return ok;
}

void Task::updateActiveIcon()
{
  _currentPic = (_currentPic+1) % 8;
  setPixmap(1, *(*icons)[_currentPic]);
}

void Task::noNegativeTimes()
{
  if ( _time < 0 )
      _time = 0;
  if ( _sessionTime < 0 )
      _sessionTime = 0;
}

QString Task::fullName() const
{
  if (isRoot())
    return name();
  else
    return parent()->fullName() + QString::fromLatin1("/") + name();
}

KCal::Todo* Task::asTodo(KCal::Todo* todo) const
{

  todo->setSummary( name() );

  // Note: if the date start is empty, the KOrganizer GUI will have the
  // checkbox blank, but will prefill the todo's starting datetime to the
  // time the file is opened.
  //todo->setDtStart( current );
  
  todo->setCustomProperty( kapp->instanceName(),
      QCString( "totalTaskTime" ), QString::number( _time ) );
  todo->setCustomProperty( kapp->instanceName(),
      QCString( "desktopList" ), getDesktopStr() );

  KEMailSettings settings;
  todo->setOrganizer( settings.getSetting( KEMailSettings::RealName ) );

  return todo;
}

bool Task::parseIncidence( KCal::Incidence* incident, long& minutes,
    QString& name, DesktopList& desktops )
{
  bool ok = false;

  name = incident->summary();
  _uid = incident->uid();

  _comment = incident->description();

  minutes = incident->customProperty( kapp->instanceName(),
      QCString( "totalTaskTime" )).toInt( &ok );
  if ( !ok )
    minutes = 0;

  QString desktopList = incident->customProperty( kapp->instanceName(),
      QCString( "desktopList" ) );
  QStringList desktopStrList = QStringList::split( QString::fromLatin1( "\\," ),
      desktopList );
  desktops.clear();

  for ( QStringList::iterator iter = desktopStrList.begin();
        iter != desktopStrList.end();
        ++iter ) {
    int desktopInt = (*iter).toInt( &ok );
    if ( ok ) {
      desktops.push_back( desktopInt );
    }
  }

  //kdDebug() << "Task::parseIncidence: "
  //  << name << ", Minutes: " << minutes
  //  <<  ", desktop: " << desktopList << endl;

  return true;
}

QString Task::getDesktopStr() const
{
  if ( _desktops.empty() )
    return QString();

  QString desktopstr;
  for ( DesktopList::const_iterator iter = _desktops.begin();
        iter != _desktops.end();
        ++iter ) {
    desktopstr += QString::number( *iter ) + QString::fromLatin1( "," );
  }
  desktopstr.remove( desktopstr.length() - 1, 1 );
  return desktopstr;
}

void Task::cut()
{
  //kdDebug() << "Task::cut - " << name() << endl;
  changeParentTotalTimes( -_totalSessionTime, -_totalTime);
  if ( ! parent())
    listView()->takeItem(this);
  else
    parent()->takeItem(this);
}

void Task::move(Task* destination)
{
  cut();
  paste(destination);
}

void Task::paste(Task* destination)
{
  destination->insertItem(this);
  changeParentTotalTimes( _totalSessionTime, _totalTime);
}

void Task::update()
{
  setText(0, _name);
  setText(1, formatTime(_sessionTime));
  setText(2, formatTime(_time));
  setText(3, formatTime(_totalSessionTime));
  setText(4, formatTime(_totalTime));
}

void Task::addComment( QString comment, KarmStorage* storage )
{
  _comment = _comment + QString::fromLatin1("\n") + comment;
  storage->addComment(this, comment);
}

QString Task::comment() const
{
  return _comment;
}

#include "task.moc"
