/*
 *   This file only:
 *     Copyright (C) 2003  Mark Bucciarelli <mark@hubcapconsutling.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the
 *      Free Software Foundation, Inc.
 *      59 Temple Place - Suite 330
 *      Boston, MA  02111-1307  USA.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>

#include <qfile.h>
#include <qdict.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "kapplication.h"       // kapp
#include <kdebug.h>
#include <kemailsettings.h>
#include <klocale.h>            // i18n
#include <taskview.h>

#include "incidence.h"

//#include <calendarlocal.h>
//#include <journal.h>
//#include <event.h>
//#include <todo.h>

#include "karmstorage.h"
#include "preferences.h"
#include "task.h"


KarmStorage *KarmStorage::_instance = 0;

KarmStorage *KarmStorage::instance()
{
  if (_instance == 0) {
    _instance = new KarmStorage();
  }
  return _instance;
}

KarmStorage::KarmStorage() { }

QString KarmStorage::load(TaskView* view, const Preferences* preferences)
{
  // When I tried raising an exception from this method, the compiler
  // complained that exceptions are not allowed.  Not sure how apps
  // typically handle error conditions in KDE, but I'll return the error
  // as a string (empty is no error).  -- Mark, Aug 8, 2003

  // Use KDE_CXXFLAGS=$(USE_EXCEPTIONS) in Makefile.am if you want to use
  // exceptions (David Faure)
  //
  // Ok, put on the TODO list.  :)  (Mark B)

  QString err;
  KEMailSettings settings;
  int handle;

  // if same file, don't reload
  if (preferences->iCalFile() == _icalfile)
    return err;

  // If file doesn't exist, create a blank one.  This avoids an error dialog
  // that libkcal presents when asked to load a non-existent file.  We make it
  // user and group read/write, others read.  This is masked by the users
  // umask.  (See man creat)
  handle = open(QFile::encodeName(preferences->iCalFile()),
      O_CREAT|O_EXCL|O_WRONLY,
      S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);

  if (handle != -1)
  {
    close(handle);
  }

  // Clear view and calendar from memory.
  view->clear();
  _calendar.close();

  // Load new file
  _icalfile = preferences->iCalFile();
  kdDebug() << "KarmStorage::load - loading " << _icalfile << endl;
  _calendar.setEmail( settings.getSetting( KEMailSettings::EmailAddress ) );
  _calendar.setOwner( settings.getSetting( KEMailSettings::RealName ) );
  if (!_calendar.load(_icalfile))
    err = i18n("Error loading file \"%1\"")
      .arg(_icalfile);

  // Build task view from iCal data
  if (!err)
  {
    KCal::Todo::List todoList;
    KCal::Todo::List::ConstIterator todo;
    QDict< Task > map;

    // Build dictionary to look up Task object from Todo uid.  Each task is a
    // QListViewItem, and is initially added with the view as the parent.
    todoList = _calendar.rawTodos();
    kdDebug() << "KarmStorage::load "
      << "rawTodo count (includes completed todos) ="
      << todoList.count() << endl;
    for( todo = todoList.begin(); todo != todoList.end(); ++todo )
    {
      // Initially, if a task was complete, it was removed from the view.
      // However, this increased the complexity of reporting on task history.
      //
      // For example, if a task is complete yet has time logged to it during
      // the date range specified on the history report, we have to figure out
      // how that task fits into the task hierarchy.  Currently, this
      // structure is held in memory by the structure in the list view.
      //
      // I considered creating a second tree that held the full structure of
      // all complete and incomplete tasks.  But this seemed to much of a
      // change with an impending beta release and a full todo list.
      //
      // Hence this "solution".  Include completed tasks, but mark them as
      // inactive in the view.
      //
      //if ((*todo)->isCompleted()) continue;

      Task* task = new Task(*todo, view);
      map.insert( (*todo)->uid(), task );
      view->setRootIsDecorated(true);
      if ((*todo)->isCompleted())
      {
        task->setEnabled(false);
        task->setOpen(false);
      }
      else
        task->setOpen(true);

    }

    // Load each task under it's parent task.
    for( todo = todoList.begin(); todo != todoList.end(); ++todo )
    {
      Task* task = map.find( (*todo)->uid() );

      // No relatedTo incident just means this is a top-level task.
      if ( (*todo)->relatedTo() )
      {
        Task* newParent = map.find( (*todo)->relatedToUid() );

        // Complete the loading but return a message
        if ( !newParent )
          err = i18n("Error loading \"%1\": could not find parent (uid=%2)")
            .arg(task->name())
            .arg((*todo)->relatedToUid());

        if (!err)
          task->move( newParent);
      }
    }

    kdDebug() << "KarmStorage::load - loaded " << view->count()
      << " tasks from " << _icalfile << endl;
  }

  return err;
}

void KarmStorage::save(TaskView* taskview)
{
  QPtrStack< KCal::Todo > parents;

  for (Task* task=taskview->first_child(); task; task = task->nextSibling())
  {
    writeTaskAsTodo(task, 1, parents );
  }

  _calendar.save(_icalfile);

  kdDebug()
    << "KarmStorage::save : wrote "
    << taskview->count() << " tasks to " << _icalfile << endl;
}

void KarmStorage::writeTaskAsTodo(Task* task, const int level,
    QPtrStack< KCal::Todo >& parents )
{

  KCal::Todo* todo;

  todo = _calendar.todo(task->uid());
  task->asTodo(todo);

  if ( !parents.isEmpty() )
    todo->setRelatedTo( parents.top() );

  parents.push( todo );

  for (Task* nextTask = task->firstChild(); nextTask;
      nextTask = nextTask->nextSibling() )
  {
    writeTaskAsTodo(nextTask, level+1, parents );
  }

  parents.pop();
}

bool KarmStorage::isEmpty()
{
  KCal::Todo::List todoList;

  todoList = _calendar.rawTodos();
  return todoList.empty();
}

bool KarmStorage::isNewStorage(const Preferences* preferences) const
{
  if (!_icalfile.isNull())
    return preferences->iCalFile() != _icalfile;
  else
    return false;
}

//----------------------------------------------------------------------------
// Routines that handle legacy flat file format.
// These only stored total and session times.
//

QString KarmStorage::loadFromFlatFile(TaskView* taskview,
    const QString& filename)
{
  QString err;

  kdDebug()
    << "KarmStorage::loadFromFlatFile: " << filename << endl;

  QFile f(filename);
  if( !f.exists() )
    err = i18n("File \"%1\" not found.").arg(filename);

  if (!err)
  {
    if( !f.open( IO_ReadOnly ) )
      err = i18n("Could not open \"%1\".").arg(filename);
  }

  if (!err)
  {

    QString line;

    QPtrStack<Task> stack;
    Task *task;

    QTextStream stream(&f);

    while( !stream.atEnd() ) {
      // lukas: this breaks for non-latin1 chars!!!
      // if ( file.readLine( line, T_LINESIZE ) == 0 )
      //   break;

      line = stream.readLine();
      kdDebug() << "DEBUG: line: " << line << "\n";

      if (line.isNull())
        break;

      long minutes;
      int level;
      QString name;
      DesktopList desktopList;
      if (!parseLine(line, &minutes, &name, &level, &desktopList))
        continue;

      unsigned int stackLevel = stack.count();
      for (unsigned int i = level; i<=stackLevel ; i++) {
        stack.pop();
      }

      if (level == 1) {
        kdDebug() << "KarmStorage::loadFromFlatFile - toplevel task: "
          << name << " min: " << minutes << "\n";
        task = new Task(name, minutes, 0, desktopList, taskview);
        task->setUid(addTask(task, 0));
      }
      else {
        Task *parent = stack.top();
        kdDebug() << "KarmStorage::loadFromFlatFile - task: " << name
            << " min: " << minutes << " parent" << parent->name() << "\n";
        task = new Task(name, minutes, 0, desktopList, parent);

        task->setUid(addTask(task, parent));

        // Legacy File Format (!):
        parent->changeTimes(0, -minutes, false);
        taskview->setRootIsDecorated(true);
        parent->setOpen(true);
      }
      if (!task->uid().isNull())
        stack.push(task);
      else
        delete task;
    }

    f.close();

  }

  return err;
}

QString KarmStorage::loadFromFlatFileCumulative(TaskView* taskview,
    const QString& filename)
{
  QString err = loadFromFlatFile(taskview, filename);
  if (!err)
  {
    for (Task* task = taskview->first_child(); task;
        task = task->nextSibling())
    {
      adjustFromLegacyFileFormat(task);
    }
  }
  return err;
}

bool KarmStorage::parseLine(QString line, long *time, QString *name,
    int *level, DesktopList* desktopList)
{
  if (line.find('#') == 0) {
    // A comment line
    return false;
  }

  int index = line.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString levelStr = line.left(index);
  QString rest = line.remove(0,index+1);

  index = rest.find('\t');
  if (index == -1) {
    // This doesn't seem like a valid record
    return false;
  }

  QString timeStr = rest.left(index);
  rest = rest.remove(0,index+1);

  bool ok;

  index = rest.find('\t'); // check for optional desktops string
  if (index >= 0) {
    *name = rest.left(index);
    QString deskLine = rest.remove(0,index+1);

    // now transform the ds string (e.g. "3", or "1,4,5") into
    // an DesktopList
    QString ds;
    int d;
    int commaIdx = deskLine.find(',');
    while (commaIdx >= 0) {
      ds = deskLine.left(commaIdx);
      d = ds.toInt(&ok);
      if (!ok)
        return false;

      desktopList->push_back(d);
      deskLine.remove(0,commaIdx+1);
      commaIdx = deskLine.find(',');
    }

    d = deskLine.toInt(&ok);

    if (!ok)
      return false;

    desktopList->push_back(d);
  }
  else {
    *name = rest.remove(0,index+1);
  }

  *time = timeStr.toLong(&ok);

  if (!ok) {
    // the time field was not a number
    return false;
  }
  *level = levelStr.toInt(&ok);
  if (!ok) {
    // the time field was not a number
    return false;
  }

  return true;
}

void KarmStorage::adjustFromLegacyFileFormat(Task* task)
{
  // unless the parent is the listView
  if ( task->parent() )
    task->parent()->changeTimes(-task->sessionTime(), -task->time(), false);

  // traverse depth first -
  // as soon as we're in a leaf, we'll substract it's time from the parent
  // then, while descending back we'll do the same for each node untill
  // we reach the root
  for ( Task* subtask = task->firstChild(); subtask;
      subtask = subtask->nextSibling() )
    adjustFromLegacyFileFormat(subtask);
}

//----------------------------------------------------------------------------
// Routines that handle logging KArm history
//

//
// public routines:
//

QString KarmStorage::addTask(const Task* task, const Task* parent)
{
  KCal::Todo* todo;
  QString uid;

  todo = new KCal::Todo();
  if (_calendar.addTodo(todo))
  {
    task->asTodo(todo);
    if (parent)
      todo->setRelatedTo(_calendar.todo(parent->uid()));
    uid = todo->uid();
  }

  return uid;
}

bool KarmStorage::removeTask(Task* task)
{

  // delete history
  KCal::Event::List eventList = _calendar.rawEvents();
  for(KCal::Event::List::iterator i = eventList.begin();
      i != eventList.end();
      ++i)
  {
    //kdDebug() << "KarmStorage::removeTask: "
    //  << (*i)->uid() << " - relatedToUid() "
    //  << (*i)->relatedToUid()
    //  << ", relatedTo() = " << (*i)->relatedTo() <<endl;
    if ( (*i)->relatedToUid() == task->uid()
        || ( (*i)->relatedTo()
            && (*i)->relatedTo()->uid() == task->uid()))
    {
      _calendar.deleteEvent(*i);
    }
  }

  // delete todo
  KCal::Todo *todo = _calendar.todo(task->uid());
  _calendar.deleteTodo(todo);

  // save entire file
  _calendar.save(_icalfile);

  return true;
}

void KarmStorage::addComment(const Task* task, const QString& comment)
{
  KCal::Todo* todo;

  todo = _calendar.todo(task->uid());

  // Do this to avoid compiler warnings about comment not being used.  once we
  // transition to using the addComment method, we need this second param.
  QString s = comment;

  // Need to wait until my libkcal-comment patch is applied for this ...
  //todo->addComment(comment);


  // temporary
  todo->setDescription(task->comment());

  _calendar.save(_icalfile);
}

void KarmStorage::stopTimer(const Task* task)
{
  long delta = task->startTime().secsTo(QDateTime::currentDateTime());
  changeTime(task, delta);
}

void KarmStorage::changeTime(const Task* task, const long deltaSeconds)
{

  KCal::Event* e;
  QDateTime end;

  e = baseEvent(task);

  // Don't use duration, as ICalFormatImpl::writeIncidence never writes a
  // duration, even though it looks like it's used in event.cpp.
  end = task->startTime();
  if (deltaSeconds > 0)
    end = task->startTime().addSecs(deltaSeconds);
  e->setDtEnd(end);

  // Use a custom property to keep a record of negative durations
  e->setCustomProperty( kapp->instanceName(),
      QCString("duration"),
      QString::number(deltaSeconds));

  _calendar.addEvent(e);

  // This saves the entire iCal file each time, which isn't efficient but
  // ensures no data loss.  A faster implementation would be to append events
  // to a file, and then when KArm closes, append the data in this file to the
  // iCal file.
  //_calendar.save(_icalfile);
  // Meanwhile, we simply use a timer to delay the full-saving until the GUI
  // has updated, for better user feedback. Feel free to get rid of this if/when
  // implementing the faster saving (DF).
  task->taskView()->scheduleSave();
}


KCal::Event* KarmStorage::baseEvent(const Task * task)
{
  KCal::Event* e;
  QStringList categories;

  e = new KCal::Event;
  e->setSummary(task->name());

  // Can't use setRelatedToUid()--no error, but no RelatedTo written to disk
  e->setRelatedTo(_calendar.todo(task->uid()));

  // Debugging: some events where not getting a related-to field written.
  assert(e->relatedTo()->uid() == task->uid());

  // Have to turn this off to get datetimes in date fields.
  e->setFloats(false);
  e->setDtStart(task->startTime());

  // So someone can filter this mess out of their calendar display
  categories.append(i18n("KArm"));
  e->setCategories(categories);

  return e;
}

HistoryEvent::HistoryEvent(QString uid, QString name, long duration,
        QDateTime start, QDateTime stop, QString todoUid)
{
  _uid = uid;
  _name = name;
  _duration = duration;
  _start = start;
  _stop = stop;
  _todoUid = todoUid;
}


QValueList<HistoryEvent> KarmStorage::getHistory(const QDate& from,
    const QDate& to)
{
  QValueList<HistoryEvent> retval;
  QStringList processed;
  KCal::Event::List events;
  KCal::Event::List::iterator event;
  QString duration;

  for(QDate d = from; d <= to; d = d.addDays(1))
  {
    events = _calendar.rawEventsForDate(d);
    for (event = events.begin(); event != events.end(); ++event)
    {

      // KArm events have the custom property X-KDE-Karm-duration
      if (! processed.contains( (*event)->uid()))
      {
        // If an event spans multiple days, CalendarLocal::rawEventsForDate
        // will return the same event on both days.  To avoid double-counting
        // such events, we (arbitrarily) attribute the hours from both days on
        // the first day.  This mis-reports the actual time spent, but it is
        // an easy fix for a (hopefully) rare situation.
        processed.append( (*event)->uid());

        duration = (*event)->customProperty(kapp->instanceName(),
            QCString("duration"));
        if ( ! duration.isNull() )
        {
          if ( (*event)->relatedTo()
              &&  ! (*event)->relatedTo()->uid().isEmpty() )
          {
            retval.append(HistoryEvent(
                (*event)->uid(),
                (*event)->summary(),
                duration.toLong(),
                (*event)->dtStart(),
                (*event)->dtEnd(),
                (*event)->relatedTo()->uid()
                ));
          }
          else
            // Something is screwy with the ics file, as this KArm history event
            // does not have a todo related to it.  Could have been deleted
            // manually?  We'll continue with report on with report ...
            kdDebug() << "KarmStorage::getHistory(): "
              << "The event " << (*event)->uid()
              << " is not related to a todo.  Dropped." << endl;
        }
      }
    }
  }

  return retval;
}

/*
 * Obsolete methods for writing to flat file format.
 * Aug 8, 2003, Mark
 *
void KarmStorage::saveToFileFormat()
{
  //QFile f(_preferences->saveFile());
  QFile f(_preferences->flatFile());

  if ( !f.open( IO_WriteOnly | IO_Truncate ) ) {
    QString msg = i18n( "There was an error trying to save your data file.\n"
                       "Time accumulated during this session will not be saved!\n");
    KMessageBox::error(0, msg );
    return;
  }
  const char * comment = "# TaskView save data\n";

  f.writeBlock(comment, strlen(comment));  //comment
  f.flush();

  QTextStream stream(&f);
  for (Task* child = firstChild();
             child;
             child = child->nextSibling())
    writeTaskToFile(&stream, child, 1);

  f.close();
  kdDebug() << "Saved data to file " << f.name() << endl;
}
void KarmStorage::writeTaskToFile( QTextStream *strm, Task *task,
                                int level)
{
  //lukas: correct version for non-latin1 users
  QString _line = QString::fromLatin1("%1\t%2\t%3").arg(level).
          arg(task->time()).arg(task->name());

  DesktopList d = task->getDesktops();
  int dsize = d.size();
  if (dsize>0) {
    _line += '\t';
    for (int i=0; i<dsize-1; i++) {
      _line += QString::number(d[i]);
      _line += ',';
    }
    _line += QString::number(d[dsize-1]);
  }
  *strm << _line << "\n";

  for ( Task* child= task->firstChild();
              child;
              child=child->nextSibling()) {
    writeTaskToFile(strm, child, level+1);
  }
}

*/
