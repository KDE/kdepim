#include <ctype.h>
#include <qfile.h>
#include <stdlib.h>
#include <stdio.h>
#include <kiconloader.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qcstring.h>
#include "kapplication.h"
#include "kemailsettings.h"
#include "kdebug.h"
#include "event.h"
#include "task.h"
#include "logging.h"


QPtrVector<QPixmap> *Task::icons = 0;

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopListType desktops, QListView *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
};

Task::Task( const QString& taskName, long minutes, long sessionTime,
            DesktopListType desktops, QListViewItem *parent)
  : QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
}

Task::Task( KCal::Incidence* event, QListView* parent )
  : QObject(), QListViewItem( parent )
{
  long minutes = 0;
  QString name;
  int level = 0;
  long sessionTime = 0;
  DesktopListType desktops;
  parseIncidence( event, minutes, name, level, desktops );
  init( name, minutes, sessionTime, desktops );
}

void Task::init( const QString& taskName, long minutes, long sessionTime,
                 DesktopListType desktops)
{
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

  // is this the right place?
  _logging = Logging::instance();

  _name = taskName.stripWhiteSpace();
  _totalTime = minutes;
  _sessionTime = sessionTime;
  noNegativeTimes();
  _timer = new QTimer(this);
  _desktops = desktops;
  connect(_timer, SIGNAL(timeout()), this, SLOT(updateActiveIcon()));
  setPixmap(1, UserIcon(QString::fromLatin1("empty-watch.xpm")));
  update();
  _i = 0;
}


void Task::setRunning( bool on )
{
  if (on) {
    if (!_timer->isActive()) {
      _timer->start(1000);
      _logging->start(this);
      _i=7;
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
  _name = name;
  _logging->rename( this, oldname );
  update();
}

void Task::setTotalTime ( long minutes )
{
  long oldtime = _totalTime;
  _totalTime = minutes;
  noNegativeTimes();
  _logging->newTotalTime( this, _totalTime, (_totalTime - oldtime) );
  update();
}

void Task::setSessionTime ( long minutes )
{
  long oldtime = _sessionTime;
  _sessionTime = minutes;
  noNegativeTimes();
  _logging->newSessionTime( this, _sessionTime, (_sessionTime - oldtime) );
  update();
}

void Task::setDesktopList ( DesktopListType desktopList )
{
  _desktops = desktopList;
}

void Task::incrementTime( long minutes )
{
  _totalTime += minutes;
  _sessionTime += minutes;
  noNegativeTimes();
  update();
}

void Task::decrementTime(long minutes)
{
  _totalTime -= minutes;
  _sessionTime -= minutes;
  noNegativeTimes();
  update();
}


void Task::updateActiveIcon()
{
  _i = (_i+1) % 8;
  setPixmap(1, *(*icons)[_i]);
}

void Task::resetSessionTime()
{
    setTotalTime( _totalTime - _sessionTime );
    setSessionTime( 0 );
    noNegativeTimes();
    update();
}

void Task::noNegativeTimes()
{
  if ( _totalTime < 0 )
      _totalTime = 0;
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
                            QString::number( totalTime() ) );
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
                           int& level, DesktopListType& desktops )
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
  for ( DesktopListType::const_iterator iter = _desktops.begin();
        iter != _desktops.end();
        ++iter ) {
    desktopstr += QString::number( *iter ) + QString::fromLatin1( "," );
  }
  desktopstr.remove( desktopstr.length() - 1, 1 );
  return desktopstr;
}

#include "task.moc"
