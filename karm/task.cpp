#include <qcstring.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qtimer.h>

#include <kiconloader.h>

#include "kapplication.h"       // kapp
#include "kemailsettings.h"
#include "kdebug.h"

#include "event.h"

#include "logging.h"
#include "karmutility.h"
#include "task.h"
#include "taskview.h"


QPtrVector<QPixmap> *Task::icons = 0;

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopList desktops, TaskView *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
};

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopList desktops, Task *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
}

Task::Task( KCal::Incidence* event, TaskView* parent )
  : QObject(), QListViewItem( parent )
{
  long minutes = 0;
  QString name;
  int level = 0;
  long sessionTime = 0;
  DesktopList desktops;
  parseIncidence( event, minutes, name, level, desktops );
  init( name, minutes, sessionTime, desktops );
}

void Task::init( const QString& taskName, long minutes, long sessionTime,
                 DesktopList desktops)
{
  // if our parent is the taskview then connect our totalTimesChanged
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
  _logging = Logging::instance();

  _name = taskName.stripWhiteSpace();
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

void Task::setRunning( bool on )
{
  if (on) {
    if (!_timer->isActive()) {
      _timer->start(1000);
      _logging->start(this);
      _currentPic=7;
      updateActiveIcon();
    }
  }
  else {
    if (_timer->isActive()) {
      _timer->stop();
      _logging->stop(this);
      setPixmap(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
    }
  }
}

bool Task::isRunning() const
{
  return _timer->isActive();
}

void Task::setName( const QString& name )
{
  QString oldname = _name;
  // only rename if new name is different than old
  if ( oldname != name ) {
    _name = name;
    _logging->rename( this, oldname );
    update();
  }
}

void Task::setDesktopList ( DesktopList desktopList )
{
  _desktops = desktopList;
}

void Task::changeTimes( long minutesSession, long minutes, bool do_logging )
{
  if( minutesSession != 0 || minutes != 0) {
    _sessionTime += minutesSession;
    _time += minutes;
    if ( do_logging )
      _logging->changeTimes( this, minutesSession, minutes);

    noNegativeTimes();
    changeTotalTimes( minutesSession, minutes );
  }
}

void Task::changeTotalTimes( long minutesSession, long minutes )
{
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
  if ( isRoot() )
    emit totalTimesChanged( minutesSession, minutes );
  else
    parent()->changeTotalTimes( minutesSession, minutes );
}

void Task::remove( QPtrList<Task>& activeTasks)
{
  if( isRunning() ) setRunning( false );
  for ( Task* child= this->firstChild();
              child;
              child= child->nextSibling() )
    child->remove( activeTasks );
  // TODO: by now this tasks _totalSessionTime and _totalTime should be == 0!
  changeParentTotalTimes( -_sessionTime, -_time); 
  delete this;
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

KCal::Event* Task::asEvent( int level )
{
  KCal::Event* event = new KCal::Event;
  event->setSummary( name() );
  QDateTime current = QDateTime::currentDateTime();
  event->setDtStart( current );
  event->setDtEnd( current.addSecs( totalTimeInSeconds() ) );
  event->setCustomProperty( kapp->instanceName(),
                            QCString( "durationInMinutes" ),
                            QString::number( _time ) );
  event->setCustomProperty( kapp->instanceName(),
                            QCString( "desktopList" ),
                            getDesktopStr() );
  event->setCustomProperty( kapp->instanceName(),
                            QCString( "level" ),
                            QString::number( level ) );
  KEMailSettings settings;
  event->setOrganizer( settings.getSetting( KEMailSettings::RealName ) );
  return event;
}

bool Task::parseIncidence( KCal::Incidence* event, long& minutes, QString& name,
                           int& level, DesktopList& desktops )
{
  bool ok = false;

  name = event->summary();

  minutes = event->customProperty( kapp->instanceName(),
                                   QCString( "durationInMinutes" )
                                 ).toInt( &ok );
  if ( !ok )
    minutes = 0;

  level = event->customProperty( kapp->instanceName(), QCString( "level" )
                               ).toInt( &ok );
  if ( !ok )
    level = 0;

  QString desktopList = event->customProperty( kapp->instanceName(),
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

  kdDebug() << "Parsed event: Name: " << name << ", Minutes: " << minutes
            << ", level: " << level << ", desktop: " << desktopList << endl;

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

void Task::addComment( QString comment )
{
  _logging->comment(this, comment );
}

#include "task.moc"
