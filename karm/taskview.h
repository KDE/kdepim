#ifndef KARM_TASK_VIEW_H
#define KARM_TASK_VIEW_H

#include <qdict.h>
#include <qptrlist.h>
#include <qptrstack.h>

#include <klistview.h>

#include "calendarlocal.h"              // KCal::CalendarLocal;

#include "desktoplist.h"
//#include "desktoptracker.h"
#include "karmutility.h"

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

/**
 * Container and interface for the tasks.
 */

class TaskView : public KListView
{
  Q_OBJECT

  public:
    TaskView( QWidget *parent = 0, const char *name = 0 );
    virtual ~TaskView();

    Task* firstChild()  const { return (Task*)KListView::firstChild(); };
    Task* currentItem() const { return (Task*)KListView::currentItem(); };

    void load();
    /*
       File format:
       zero or more lines of
       1         number
       time      in minutes
       string    task name
       [string]  desktops, in which to count. e.g. "1,2,5" (optional)
    */
    void writeTaskToFile( QTextStream *, Task*, int );
    void writeTaskToCalendar( KCal::CalendarLocal&, Task*, int,
                              QPtrStack< KCal::Event >&);
    void startNewSession();
    void resetTimeForAllTasks();

  public slots:
    void save();
    void startCurrentTimer();
    void stopCurrentTimer();
    void stopAllTimers();
    void changeTimer( QListViewItem * = 0 );
    void newTask();
    void newTask( QString caption, Task* parent );
    void newSubTask();
    void editTask();
    void deleteTask();
    void addCommentToTask();
    void extractTime( int minutes );
    void loadFromKOrgTodos();
    void loadFromKOrgEvents();
    void taskTotalTimesChanged( long session, long total)
                                { emit totalTimesChanged( session, total); };
    void adaptColumns();
    /** receiving signal that a task is being deleted */
    void deletingTask(Task* deletedTask);
    void startTimerFor( Task* task );
    void stopTimerFor( Task* task );

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
    KCal::CalendarLocal _calendar;
    int previousColumnWidths[4];
    DesktopTracker* _desktopTracker;

  private:
    enum { loadEvent = 1, loadTodo = 2 };
    void updateParents( Task* task, long totalDiff, long sesssionDiff);
    void deleteChildTasks( Task *item );
    bool parseLine( QString line, long *time, QString *name, int *level,
                    DesktopList* desktopList );
    void loadFromFileFormat();
    void saveToFileFormat();
    void loadFromKCalFormat( const QString& file, int loadMask );
    void loadFromKCalFormat();
    void saveToKCalFormat();
    void buildAndPositionTasks( KCal::Event::List &eventList );
    void buildAndPositionTasks( KCal::Todo::List &todoList );
    void buildTask( KCal::Incidence* event, QDict<Task>& map );
    void positionTask( const KCal::Incidence* event, const QDict<Task>& map );
    void addTimeToActiveTasks( int minutes, bool do_logging );
    /** adjust to file format changes */
    void adjustFromLegacyFileFormat();
    void adjustFromLegacyFileFormat(Task* task);

  protected slots:
    void autoSaveChanged( bool );
    void autoSavePeriodChanged( int period );
    void minuteUpdate();
};

#endif // KARM_TASK_VIEW
