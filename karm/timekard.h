#ifndef KARM_TIMEKARD_H
#define KARM_TIMEKARD_H

#undef Color // X11 headers
#undef GrayScale // X11 headers
#include <kprinter.h>
//#include <qdate.h>

#include "karmstorage.h"

class QString;
class QDate;

class TaskView;


/**
 *  Seven consecutive days.
 *
 *  The timecard report prints out one table for each week of data.  The first
 *  day of the week should be read from the KControlPanel.  Currently, it is
 *  hardcoded to Sunday.
 */
class Week
{
  public:
    /** Need an empty constructor to use in a QValueList. */
    Week();
    Week(QDate from);
    QDate start() const;
    QDate end() const;
    void load(QDate from);

  private:
    QValueList<QDate> _days;
};

/**
 *  Routines to output timecard data.
 */
class TimeKard
{
  public:
    TimeKard() {};

    /**
     * Generates ascii text of task totals, for current task on down.
     *
     * Formatted for pasting into clipboard.
     *
     * @param justThisTask Only useful when user has picked a root task.  We
     * use this parameter to distinguish between when a user just wants to
     * print the task subtree for a root task and when they want to print 
     * all tasks.
     */
    QString totalsAsText(TaskView* taskview, bool justThisTask = true);

    /**
     * Generates ascii text of weekly task history, for current task on down.
     *
     * Formatted for pasting into clipboard.
     */
    QString historyAsText(TaskView* taskview, const QDate& from, 
        const QDate& to);
    QValueList<Week> weeksFromDateRange(const QDate& from, const QDate& to);
    void printTask(Task *t, QString &s, int level);
  
  /*
  private:
    void header(QPainter& painter);
    void footer(QPainter& painter);
    void tableheader(QPainter& painter);
    void tablefooter(QPainter& painter);
  */
};
#endif // KARM_TIMEKARD_H
