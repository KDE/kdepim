#ifndef SSK_KARM_H
#define SSK_KARM_H

#include <stdio.h>
#include <stdlib.h>

#include <qptrstack.h>
#include <qsplitter.h>
#include <qdict.h>
#include <klistview.h>
#include <qptrlist.h>
#include <qtextstream.h>
#include "kwinmodule.h"
#include <vector>
#include "calendarlocal.h"

class KMenuBar;
class KToolBar;
class QListBox;
class AddTaskDialog;
class IdleTimeDetector;
class QTimer;
class Preferences;
class Task;

typedef std::vector<int> DesktopListType;
typedef std::vector<Task*> TaskVector;

/**
 * Container and interface for the tasks.
 */

class TaskView : public KListView
{
    Q_OBJECT

private: // member variables
    IdleTimeDetector *_idleTimeDetector;
    QTimer *_minuteTimer;
    QTimer *_autoSaveTimer;

    Preferences *_preferences;

    QPtrList<Task> activeTasks;
    KWinModule kWinModule;

    // define vectors for at most 16 virtual desktops
    // E.g.: desktopTrackerStop[3] contains a vector with
    // all tasks to be stopped, when switching to desk 3.
    TaskVector desktopTracker[16];
    int lastDesktop;
    int desktopCount;

    KCal::CalendarLocal _calendar;

public:
    TaskView( QWidget *parent = 0, const char *name = 0 );
    virtual ~TaskView();
    static QString formatTime(long minutes);
    void printTrackers();

private:
    enum { loadEvent = 1, loadTodo = 2 };
    void updateParents( QListViewItem* task, long totalDiff, long sesssionDiff );
    void startTimerFor(Task* item);
    void stopTimerFor(Task* item);
    void applyTrackers();
    void updateTrackers(Task *task, DesktopListType dl);
    bool parseLine(QString line, long *time, QString *name, int *level, DesktopListType* desktops);
    void loadFromFileFormat();
    void saveToFileFormat();
    void loadFromKCalFormat( const QString& file, int loadMask );
    void loadFromKCalFormat();
    void saveToKCalFormat();
    void buildAndPositionTasks( QPtrList<KCal::Event>& eventList );
    void buildAndPositionTasks( QPtrList<KCal::Todo>& todoList );
    void buildTask( KCal::Incidence* event, QDict<Task>& map );
    void positionTask( const KCal::Incidence* event, const QDict<Task>& map );

public slots:
    /*
    File format:
    zero or more lines of
    1 		number
    time	in minutes
    string	task name
    [string]    desktops, in which to count. e.g. "1,2,5" (optional)
    */
    void load();
    void save();
    void writeTaskToFile(QTextStream *, QListViewItem *, int);
    void writeTaskToCalendar(KCal::CalendarLocal&, Task*, int, QPtrStack< KCal::Event >&);
    void startCurrentTimer();
    void stopCurrentTimer();
    void stopAllTimers();
    void changeTimer(QListViewItem * = 0);
    void newTask();
    void newTask(QString caption, QListViewItem *parent);
    void newSubTask();
    void editTask();
    void deleteTask();
    void extractTime(int minutes);
    void resetSessionTimeForAllTasks();
    void resetTimeForAllTasks();
    void handleDesktopChange(int desktop);
    void loadFromKOrgTodos();
    void loadFromKOrgEvents();

protected slots:
    void autoSaveChanged(bool);
    void autoSavePeriodChanged(int period);
    void minuteUpdate();

signals:
    void sessionTimeChanged( long, long );
    void updateButtons();
    void timerActive();
    void timerInactive();
    void tasksChanged( QPtrList<Task> activeTasks);


protected slots:
    void stopChildCounters(Task *item);
    void addTimeToActiveTasks(int minutes);
};

inline QString TaskView::formatTime( long minutes )
{
    QString time;
    time.sprintf("%ld:%02ld", minutes / 60, labs(minutes % 60));
    return time;
}

#endif
