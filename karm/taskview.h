#ifndef KARM_TASK_VIEW_H
#define KARM_TASK_VIEW_H

#include <qdict.h>
#include <qptrlist.h>
#include <qptrstack.h>

#include <klistview.h>

#include "desktoplist.h"
#include "karmstorage.h"
//#include "desktoptracker.h"

//#include "karmutility.h"

class QListBox;
class QTextStream;
class QTimer;

class KMenuBar;
class KToolBar;

class DesktopTracker;
class EditTaskDialog;
class IdleTimeDetector;
class Preferences;
class Task;
class KarmStorage;
class HistoryEvent;

/**
 * Container and interface for the tasks.
 */

class TaskView : public KListView
{
  Q_OBJECT

  public:
    TaskView( QWidget *parent = 0, const char *name = 0 );
    virtual ~TaskView();

    /**  Return the first item in the view, cast to a Task pointer.  */
    Task* first_child() const;

    /**  Return the current item in the view, cast to a Task pointer.  */
    Task* current_item() const;

    /**  Return the i'th item (zero-based), cast to a Task pointer.  */
    Task* item_at_index(int i);

    /** Load the view from storage.  */
    void load();

    /** Reset session time to zero for all tasks.   */
    void startNewSession();

    /** Reset session and total time to zero for all tasks.  */
    void resetTimeForAllTasks();

    /** Return the total number if items in the view.  */
    long count();

    /** Return list of start/stop events for given date range. */
    QValueList<HistoryEvent> getHistory(const QDate& from, const QDate& to)
      const;

  public slots:
    /** Save to persistent storage. */
    void save();

    /** Start the timer on the current item (task) in view.  */
    void startCurrentTimer();

    /** Stop the timer for the current item in the view.  */
    void stopCurrentTimer();

    /** Stop all running timers.  */
    void stopAllTimers();

    /** Stop all running timers, and start timer on current item.  */
    void changeTimer( QListViewItem * = 0 );

    /** Calls newTask with caption "New Task".  */
    void newTask();

    /** Display edit task dialog and create a new task with results.  */
    void newTask( QString caption, Task* parent );

    /** Used to import a legacy file format. */
    void loadFromFlatFile();

    /** Calls newTask with caption "New Sub Task". */
    void newSubTask();
    void editTask();
    void deleteTask();
    void addCommentToTask();

    /** Subtracts time from all active tasks, and does not log event. */ 
    void extractTime( int minutes );
    void taskTotalTimesChanged( long session, long total)
                                { emit totalTimesChanged( session, total); };
    void adaptColumns();
    /** receiving signal that a task is being deleted */
    void deletingTask(Task* deletedTask);
    void startTimerFor( Task* task );
    void stopTimerFor( Task* task );

    /** User has picked a new iCalendar file on preferences screen. */
    void iCalFileChanged(QString file);

    /** Copy totals for current and all sub tasks to clipboard. */
    void clipTotals();

    /** Copy history for current and all sub tasks to clipboard. */
    void clipHistory();

  signals:
    void totalTimesChanged( long session, long total );
    void updateButtons();
    void timersActive();
    void timersInactive();
    void tasksChanged( QPtrList<Task> activeTasks );

  private: // member variables
    IdleTimeDetector *_idleTimeDetector;
    QTimer *_minuteTimer;
    QTimer *_autoSaveTimer;
    Preferences *_preferences;
    QPtrList<Task> activeTasks;
    int previousColumnWidths[4];
    DesktopTracker* _desktopTracker;

    //KCal::CalendarLocal _calendar;
    KarmStorage * _storage;

  private:
    void updateParents( Task* task, long totalDiff, long sesssionDiff);
    void deleteChildTasks( Task *item );
    void addTimeToActiveTasks( int minutes, bool do_logging); 

  protected slots:
    void autoSaveChanged( bool );
    void autoSavePeriodChanged( int period );
    void minuteUpdate();
};

#endif // KARM_TASK_VIEW
