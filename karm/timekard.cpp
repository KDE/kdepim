// #include <iostream>

#include <qdatetime.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qmap.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>            // i18n
#include <event.h>

#include "karmutility.h"        // formatTime()
#include "timekard.h"
#include "task.h"
#include "taskview.h"

const int taskWidth = 40;
const int timeWidth = 6; 
const int totalTimeWidth = 7;
const int reportWidth = taskWidth + (7 * timeWidth) + totalTimeWidth;

const QString cr = QString::fromLatin1("\n");

QString TimeKard::totalsAsText(TaskView* taskview, bool justThisTask)
{
  QString retval;
  QString taskhdr, totalhdr;
  QString line;
  QString buf;
  long sum;

  line.fill('-', reportWidth);
  line += cr;

  // header
  retval += i18n("Task Totals") + cr;
  retval += KGlobal::locale()->formatDateTime(QDateTime::currentDateTime());
  retval += cr + cr;
  retval += QString(QString::fromLatin1("%1    %2"))
    .arg(i18n("Time"), timeWidth)
    .arg(i18n("Task"), -taskWidth);
  retval += cr;
  retval += line;

  // tasks
  if (taskview->current_item())
  {
    if (justThisTask)
    {
      // a task's total time includes the sum of all subtask times
      sum = taskview->current_item()->totalTime();
      printTask(taskview->current_item(), retval, 0);
    }
    else
    {
      sum = 0;
      for (Task* task= taskview->current_item(); task;
          task= task->nextSibling())
      {
        sum += task->totalTime();
        printTask(task, retval, 0);
      }
    } 

    // total
    buf.fill('-', timeWidth + 4);
    retval += QString(QString::fromLatin1("%1")).arg(buf, timeWidth) + cr;
    retval += QString(QString::fromLatin1("%1 %2"))
      .arg(formatTime(sum),timeWidth)
      .arg(i18n("Total"), -taskWidth);
  }
  else
    retval += i18n("No tasks!");
  
  return retval;
}

void TimeKard::printTask(Task *task, QString &s, int level)
{
  QString buf;

  s += buf.fill(' ', level);
  s += QString(QString::fromLatin1("%1    %2"))
    .arg(formatTime(task->totalTime()), timeWidth)
    .arg(task->name(), -taskWidth);
  s += cr;

  for (Task* subTask = task->firstChild();
      subTask;
      subTask = subTask->nextSibling())
  {
    printTask(subTask, s, level+1);
  }      
}

void TimeKard::printWeekTask(const Task *task, 
    const QMap<QString,long>& datamap, 
    const Week& week, const int level, QString& s, long& sum)
{
  QString buf;
  QString key;
  QDate day;
  long weeksum;

  day = week.start();
  weeksum = 0;
  for (int i = 0; i < 7; i++)
  {
    key = QString::fromLatin1("%1_%2")
      .arg(day.toString(QString::fromLatin1("yyyyMMdd")))
      .arg(task->uid());

    if (datamap.contains(key))
    {
      s += QString::fromLatin1("%1")
        .arg(formatTime(datamap[key]/60), timeWidth);
      sum += datamap[key];  // in seconds
      weeksum += datamap[key];  // in seconds
    }
    else
    {
      buf.fill(' ', timeWidth);
      s += buf;
    }

    day = day.addDays(1);
  }

  // Total for task this week
  s += QString::fromLatin1("%1").arg(formatTime(weeksum/60), totalTimeWidth);
  
  // Task name
  s += buf.fill(' ', level + 1);
  s += QString::fromLatin1("%1").arg(task->name(), -taskWidth);
  s += cr;

  for (Task* subTask = task->firstChild();
      subTask;
      subTask = subTask->nextSibling())
  {
    printWeekTask(subTask, datamap, week, level+1, s, sum);
  }      
}

QString TimeKard::historyAsText(TaskView* taskview, const QDate& from,
    const QDate& to, bool justThisTask)
{
  QString retval;
  QString taskhdr, totalhdr;
  QString line, buf;
  long sum;
  
  QValueList<Week>::iterator week;
  QValueList<HistoryEvent> events;
  QValueList<HistoryEvent>::iterator event;
  QMap<QString, long> datamap;
  QString key;

  line.fill('-', reportWidth);
  line += cr;

  // header
  retval += i18n("Task History") + cr;
  retval += KGlobal::locale()->formatDateTime(QDateTime::currentDateTime());

  // output one time card table for each week in the date range
  QValueList<Week> weeks = Week::weeksFromDateRange(from, to);
  for (week = weeks.begin(); week != weeks.end(); ++week)
  {

    events = taskview->getHistory((*week).start(), (*week).end());

    // Build lookup dictionary used to output data in table cells.  keys are
    // QStrings in this format: YYYYMMDD_NNNNNN, where Y = year, M = month, d
    // = day and NNNNN = the VTODO uid.  The value is the total seconds logged
    // against that task on that day.  Note the UID is the todo id, not the
    // event id, so things are accumulated for each task.
    datamap.clear();
    for (event = events.begin(); event != events.end(); ++event)
    {
      key = QString(QString::fromLatin1("%1_%2"))
        .arg((*event).start().date().toString(QString::fromLatin1("yyyyMMdd")))
        .arg((*event).todoUid());
      if (datamap.contains(key))
        datamap.replace(key, datamap[key] + (*event).duration());
      else
        datamap.insert(key, (*event).duration());
    }

    // week name
    retval += cr + cr;
    buf.fill(' ', int((reportWidth - (*week).name().length()) / 2));
    retval += buf + (*week).name() + cr;

    // day headings
    for (int i = 0; i < 7; i++)
    {
      retval += QString::fromLatin1("%1")
          .arg((*week).start().addDays(i).day(), timeWidth);
    }
    retval += cr;
    retval += line;

    // the tasks
    if (events.empty())
    {
      retval += i18n("  No hours logged this week.");
    }
    else
    {
      sum = 0;
      if (justThisTask)
      {
        printWeekTask(taskview->current_item(), datamap, (*week), 0, retval,
            sum);
      }
      else
      {
        for (Task* task= taskview->current_item(); task;
            task= task->nextSibling())
        {
          printWeekTask(task, datamap, (*week), 0, retval, sum);
        }
      } 

      // total
      retval += line;
      buf.fill(' ', 7 * timeWidth);
      retval += buf;
      retval += QString::fromLatin1("%1 %2")
        .arg(formatTime(sum/60), totalTimeWidth)
        .arg(i18n("Total"), -taskWidth);
      
    }
  }
  return retval;
}

Week::Week() {}

Week::Week(QDate from)
{
  _start = from;
}

QDate Week::start() const 
{ 
  return _start;
}

QDate Week::end() const
{
  return _start.addDays(7);
}

QString Week::name() const
{
  return QString(i18n("Week of %1"))
    .arg(KGlobal::locale()->formatDate(start()));
}

QValueList<Week> Week::weeksFromDateRange(const QDate& from, const QDate& to)
{
  QDate start;
  QValueList<Week> weeks;

  // The QDate weekNumber() method always puts monday as the first day of the
  // week.  
  //
  // Not that it matters here, but week 1 always includes the first Thursday
  // of the year.  For example, January 1, 2000 was a Saturday, so
  // QDate(2000,1,1).weekNumber() returns 52.  

  // Since report always shows a full week, we generate a full week of dates,
  // even if from and to are the same date.  The week starts on the day
  // that is set in the locale settings.
  start = from.addDays( 
      -((7 - KGlobal::locale()->weekStartDay() + from.dayOfWeek()) % 7));

  for (QDate d = start; d <= to; d = d.addDays(7))
    weeks.append(Week(d));

  return weeks;
}

