#include <ctype.h>
#include <qfile.h>
#include <stdlib.h>
#include <stdio.h>
#include <kiconloader.h>
#include <qtimer.h>
#include"task.h"
#include"logging.h"


QPtrVector<QPixmap> *Task::icons = 0;

Task::Task(const QString& taskName, long minutes, long sessionTime, DesktopListType desktops, QListView *parent)
	: QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
};

Task::Task(const QString& taskName, long minutes, long sessionTime, DesktopListType desktops, QListViewItem *parent)
  :QObject(), QListViewItem(parent)
{
  init(taskName, minutes, sessionTime, desktops);
}

void Task::init(const QString& taskName, long minutes, long sessionTime, DesktopListType desktops)
{
  if (icons == 0) {
    icons = new QPtrVector<QPixmap>(8);
    for (int i=0; i<8; i++) {
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


void Task::setRunning(bool on)
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
  _name = name;
  update();
}

void Task::setTotalTime ( long minutes )
{
  _totalTime = minutes;
  noNegativeTimes();
  _logging->newTotalTime( this, _totalTime );
  update();
}

void Task::setSessionTime ( long minutes )
{
  _sessionTime = minutes;
  noNegativeTimes();
  _logging->newSessionTime( this, _sessionTime );
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

#include "task.moc"
