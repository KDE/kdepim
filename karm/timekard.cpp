// #include <iostream>

#include <qdatetime.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>            // i18n
#include <event.h>

#include "karmutility.h"        // formatTime()
#include "timekard.h"
#include "task.h"
#include "taskview.h"

const int reportWidth = 75;

const int taskWidth = 65;
const int timeWidth = 7; // 9999.99 max size of hours

QValueList<Week> TimeKard::weeksFromDateRange(const QDate& from, 
    const QDate& to)
{
  int dow;
  QDate start;
  QValueList<Week> weeks;

  // The QDate weekNumber() method always puts monday as the first day of the
  // week.  Also, week 1 always includes the first Thursday of the year.  For
  // example, January 1, 2000 was a Saturday, so QDate(2000,1,1).weekNumber()
  // returns 52.  QValueList<Week> weeks;
  
  dow = from.dayOfWeek();    // Monday = 1
  
  // For now, we force the first day of the week to be a Monday
  if (dow == 1)
    start = from;
  else
    // If from is a Wednesday, then dow = 3.  Subtract 2 to get Monday's date.
    start = from.addDays(-(dow - 1));

  for (QDate d = start; d <= to; d = d.addDays(7))
  {
    weeks.append(Week(d));
  }
  return weeks;
}

QString TimeKard::totalsAsText(TaskView* taskview, bool justThisTask = true)
{
  QString retval;
  QString taskhdr, totalhdr;
  QString line;
  QString buf;
  long sum;

  line.fill('-', reportWidth);
  line += '\n';

  // header
  retval += i18n("Task Totals");
  retval += QString::fromLatin1("\n\n");
  retval += QString(QString::fromLatin1("%1    %2\n"))
    .arg(i18n("Time"), timeWidth)
    .arg(i18n("Task"), -taskWidth);
  retval += line;

  // tasks
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
  retval += QString(QString::fromLatin1("%1\n")).arg(buf, timeWidth);
  retval += QString(QString::fromLatin1("%1 %2"))
    .arg(formatTime(sum),timeWidth)
    .arg(i18n("Total"), -taskWidth);
  
  return retval;
}

void TimeKard::printTask(Task *task, QString &s, int level)
{
  QString buf;

  s += buf.fill(' ', level);
  s += QString(QString::fromLatin1("%1    %2\n"))
    .arg(formatTime(task->totalTime()), timeWidth)
    .arg(task->name(), -taskWidth);

  for (Task* subTask = task->firstChild();
      subTask;
      subTask = subTask->nextSibling())
  {
    printTask(subTask, s, level+1);
  }      
}

  /*
  // Calculate the totals Note the totals are only calculated at the top most
  // levels, as the totals are increased together with its children.
  int totalTotal = 0;
  int sessionTotal = 0;
  for (Task* task = taskview->current_item()); task;
      task = static_cast<Task *>(task->nextSibling()) 
  {
    totalTotal += task->time();
    sessionTotal += task->sessionTime();
  }
  return QString::fromLatin1("stubbed taskTotalsAsText");
  */

QString TimeKard::historyAsText(TaskView* taskview, const QDate& from,
    const QDate& to)
{
  return QString(QString::fromLatin1("Stubbed taskHistoryAsText: "
      "from %1, to %2"))
      .arg(from.toString())
      .arg(to.toString());
}

/*
void TimeKard::print()
{
  QValueList<Week> weeks;
  QValueList<Week>::iterator week;
  QValueList<HistoryEvent> events;

  if (setup(0L, i18n("Print Time Card"))) 
  {
    QPainter painter(this);
    QPaintDeviceMetrics deviceMetrics(this);
    QFontMetrics metrics = painter.fontMetrics();
    pageHeight = deviceMetrics.height();
    int pageWidth = deviceMetrics.width();
    xMargin = margins().width();
    yMargin = margins().height();
    yoff = yMargin;
    lineHeight = metrics.height();
  
    // FIXME: stubbed
    realPageWidth = pageWidth;

    weeks = weeksFromDateRange(_from, _to);
    for (week = weeks.begin(); week != weeks.end(); ++week)
    {
      events = _taskview->getHistory((*week).start(), (*week).end());

      if (noRoomForNewTable())
      {
        footer(painter);
        newpage();
        header(painter);
      }

      kdDebug() << "TimeKard::print: 3" << endl;
      tableheader(painter);

      if (events.empty())
      {
        painter.drawText(xMargin, yoff, i18n("No hours logged this week."));
        yoff += lineHeight;
      }
      else
      {
        kdDebug() << "TimeKard::print: 4" << endl;
        for (Task* task = _taskview->first_child(); task;
            task = static_cast<Task *>(task->nextSibling()) )
        {
          printTask(task, painter, events, 0);
        }
      }
      if (noRoomForTableFooter())
      {
        footer(painter);
        newpage();
        header(painter);
      }
      tablefooter(painter);
    }

  }
}

void TimeKard::printTask(Task *task, QPainter &painter, 
    QValueList<HistoryEvent> &events, int level)
{
  QString time = formatTime(task->time());
  QString sessionTime = formatTime(task->sessionTime());
  QString name = task->name();
  if (noRoomForNewTask())
  {
    footer(painter);
    newpage();
    header(painter);
    tableheader(painter);
  }
  printLine(time, sessionTime, name, painter, level);

  for ( Task* subTask = task->firstChild();
              subTask;
              subTask = subTask->nextSibling())
  {
    printTask(subTask, painter, events, level+1);
  }      
}

void TimeKard::printLine( QString total, QString session, QString name, 
                           QPainter &painter, int level )
{
  int xoff = xMargin + 10 * level;
  kdDebug() << "TimeKard::printLine: " << name << endl;
  
  painter.drawText(xoff, yoff, nameFieldWidth, lineHeight,
      QPainter::AlignLeft, name);

  xoff = xMargin + nameFieldWidth;
  
  painter.drawText(xoff, yoff, sessionTimeWidth, lineHeight,
      QPainter::AlignRight, session);

  xoff += sessionTimeWidth + 5;
  
  painter.drawText(xoff, yoff, timeWidth, lineHeight,
      QPainter::AlignRight, total);

  xoff += timeWidth + 5;

  yoff += lineHeight;
  
  if (yoff + 2 * lineHeight > pageHeight) {
    newPage();
    yoff = yMargin;
  }
}

bool TimeKard::noRoomForNewTable() const
{
  // 2 line for blank space before table,
  // 2 lines for table header
  // 2 tasks
  return ( (yoff + 6 * lineHeight) > pageHeight );
}
bool TimeKard::noRoomForNewTask() const
{
  return ( (yoff + 2 * lineHeight) > pageHeight );
}
bool TimeKard::noRoomForTableFooter() const
{
  return ( (yoff + 3 * lineHeight) > pageHeight );
}
void TimeKard::newpage()
{
  newPage();
  yoff = yMargin;
}
void TimeKard::footer(QPainter& painter)
{
  yoff += lineHeight;
  painter.drawLine(xMargin, yoff, xMargin + realPageWidth, yoff);
  yoff += lineHeight;
  painter.drawText(xMargin, yoff, i18n("Page %").arg(page_no));
}
void TimeKard::tableheader(QPainter& painter)
{
  painter.drawText(xMargin, yoff,QString::fromLatin1("Stubbed table header"));
  yoff += lineHeight;
  painter.drawLine(xMargin, yoff, xMargin + realPageWidth, yoff);
  yoff += 2 * lineHeight;
}
void TimeKard::tablefooter(QPainter& painter)
{
  painter.drawText(xMargin,yoff,QString::fromLatin1("Stubbed table footer"));
  yoff += lineHeight;
  painter.drawLine(xMargin, yoff, xMargin + realPageWidth, yoff);
  yoff += 2 * lineHeight;
}
*/
Week::Week() 
{
  load(QDate::currentDate());
}

Week::Week(QDate from)
{
  load(from);
}

void Week::load(QDate from)
{
  _days.clear();
  _days.append(from);
  for (int i = 1; i < 7; i++)
    _days.append(from.addDays(i));
}

QDate Week::start() const 
{ 
  return _days.first();
}

QDate Week::end() const
{
  return _days.last();
}

