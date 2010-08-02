#ifndef KARM_TASK_H
#define KARM_TASK_H

// Required b/c TQPtrList is a struct, not a class.
#include <tqptrlist.h>

// Requred b/c/ TQPtrVector is a template (?)
#include <tqptrvector.h>

#include <tqdatetime.h>

// Required b/c DesktopList is a typedef not a class.
#include "desktoplist.h"

// Required b/c of static cast below?  (How else can compiler know that a
// TaskView is a subclass or QListView?)
#include "taskview.h"

class TQFile;
class TQString;
class KarmStorage;

class TQTimer;
class KCal::Incidence;
class KCal::Todo;
class TQObject;
class TQPixmap;

/// \class Task
/** \brief A class representing a task
 *
 * A "Task" object stores information about a task such as it's name,
 * total and session times.
 *
 * It can log when the task is started, stoped or deleted.
 *
 * If a task is associated with some desktop's activity it can remember that
 * too.
 *
 * It can also contain subtasks - these are managed using the
 * TQListViewItem class.
 */
class Task : public TQObject, public QListViewItem
{
  Q_OBJECT

  public:
    //@{ constructors
    Task( const TQString& taskame, long minutes, long sessionTime,
          DesktopList desktops, TaskView* parent = 0);
    Task( const TQString& taskame, long minutes, long sessionTime,
          DesktopList desktops, Task* parent = 0);
    Task( KCal::Todo* incident, TaskView* parent );
    //@}
    /* destructor */
    ~Task();

    /** return parent Task or null in case of TaskView.
     *  same as TQListViewItem::parent()
     */
    Task* firstChild() const  { return (Task*)TQListViewItem::firstChild(); }
    Task* nextSibling() const { return (Task*)TQListViewItem::nextSibling(); }
    Task* parent() const      { return (Task*)TQListViewItem::parent(); }

    /** Return task view for this task */
    TaskView* taskView() const {
        return static_cast<TaskView *>( listView() );
    }

    /** Return unique iCalendar Todo ID for this task. */
    TQString uid() const       { return _uid; }

    /**
     * Set unique id for the task.
     *
     * The uid is the key used to update the storage.
     *
     * @param uid  The new unique id.
     */
    void setUid(const TQString uid);

    /** cut Task out of parent Task or the TaskView */
    void cut();
    /** cut Task out of parent Task or the TaskView and into the
     *  destination Task */
    void move(Task* destination);
    /** insert Task into the destination Task */
    void paste(Task* destination);

    /** Sort times numerically, not alphabetically.  */
    int compare ( TQListViewItem * i, int col, bool ascending ) const;

    //@{ timing related functions

      /**
       * Change task time.  Adds minutes to both total time and session time.
       *
       *  @param minutes        minutes to add to - may be negative
       *  @param storage        Pointer to KarmStorage instance.
       *                        If zero, don't save changes.
       */
      void changeTime( long minutes, KarmStorage* storage );

      /**
       * Add minutes to time and session time, and write to storage.
       *
       *  @param minutesSession   minutes to add to task session time
       *  @param minutes          minutes to add to task time
       *  @param storage          Pointer to KarmStorage instance.
       *                          If zero, don't save changes.
       */
      void changeTimes
        ( long minutesSession, long minutes, KarmStorage* storage=0);

      /** adds minutes to total and session time
       *
       *  @param minutesSession   minutes to add to task total session time
       *  @param minutes          minutes to add to task total time
       */
      void changeTotalTimes( long minutesSession, long minutes );

      /**
       * Reset all times to 0
       */
      void resetTimes();

      /*@{ returns the times accumulated by the task
       * @return total time in minutes
       */
      long time() const { return _time; };
      long totalTime() const { return _totalTime; };
      long sessionTime() const { return _sessionTime; };
      long totalSessionTime() const { return _totalSessionTime; };

      /**
       * Return time the task was started.
       */
      TQDateTime startTime() const { return _lastStart; };

      /** sets session time to zero. */
      void startNewSession() { changeTimes( -_sessionTime, 0 ); };
    //@}

    //@{ desktop related functions

      void setDesktopList ( DesktopList dl );
      DesktopList getDesktops() const { return _desktops;}

      TQString getDesktopStr() const;
    //@}

    //@{ name related functions

      /** sets the name of the task
       *  @param name    a pointer to the name. A deep copy will be made.
       *  @param storage a pointer to a KarmStorage object.
       */
      void setName( const TQString& name, KarmStorage* storage );

      /** returns the name of this task.
       *  @return a pointer to the name.
       */
      TQString name() const  { return _name; };

      /**
       * Returns that task name, prefixed by parent tree up to root.
       *
       * Task names are seperated by a forward slash:  /
       */
      TQString fullName() const;
    //@}

    /** Update the display of the task (all columns) in the UI. */
    void update();

    //@{ the state of a Task - stopped, running

      /** starts or stops a task
       *  @param on       true or false for starting or stopping a task
       *  @param storage a pointer to a KarmStorage object.
       *  @param whenStarted time when the task was started. Normally
				    TQDateTime::currentDateTime, but if calendar has 
				    been changed by another program and being reloaded
 				    the task is set to running with another start date
       */
      void setRunning( bool on, KarmStorage* storage, TQDateTime whenStarted=TQDateTime::currentDateTime(), TQDateTime whenStopped=TQDateTime::currentDateTime());

      /** return the state of a task - if it's running or not
       *  @return         true or false depending on whether the task is running
       */
      bool isRunning() const;
    //@}

    bool parseIncidence(KCal::Incidence*, long& minutes,
        long& sessionMinutes, TQString& name, DesktopList& desktops,
        int& percent_complete);

    /**
     *  Load the todo passed in with this tasks info.
     */
    KCal::Todo* asTodo(KCal::Todo* calendar) const;

    /** Add a comment to this task. */
    void addComment( TQString comment, KarmStorage* storage );

    /** Retrieve the entire comment for the task. */
    TQString comment() const;

    /** tells you whether this task is the root of the task tree */
    bool isRoot() const                 { return parent() == 0; }

    /** remove Task with all it's children
     * @param   activeTasks - list of aktive tasks
     * @param storage a pointer to a KarmStorage object.
     */
    bool remove( TQPtrList<Task>& activeTasks, KarmStorage* storage );

    /**
     * Update percent complete for this task.
     *
     * Tasks that are complete (i.e., percent = 100) do not show up in
     * taskview.  If percent NULL, set to zero.  If greater than 100, set to
     * 100.  If less than zero, set to zero.
     */
    void setPercentComplete(const int percent, KarmStorage *storage);


    /** Sets an appropriate icon for this task based on its level of
     * completion */
    void setPixmapProgress();

    /** Return true if task is complete (percent complete equals 100).  */
    bool isComplete();

    /** Remove current task and all it's children from the view.  */
    void removeFromView();

    /** delivers when the task was started last */
    TQDateTime lastStart() { return _lastStart; }

  protected:
    void changeParentTotalTimes( long minutesSession, long minutes );

  signals:
    void totalTimesChanged( long minutesSession, long minutes);
    /** signal that we're about to delete a task */
    void deletingTask(Task* thisTask);

  protected slots:
    /** animate the active icon */
    void updateActiveIcon();

  private:

    /** The iCal unique ID of the Todo for this task. */
    TQString _uid;

    /** The comment associated with this Task. */
    TQString _comment;

    int _percentcomplete;

    long totalTimeInSeconds() const      { return _totalTime * 60; }

    /** if the time or session time is negative set them to zero */
    void noNegativeTimes();

    /** initialize a task */
    void init( const TQString& taskame, long minutes, long sessionTime,
               DesktopList desktops, int percent_complete);


    /** task name */
    TQString _name;

    /** Last time this task was started. */
    TQDateTime _lastStart;

    //@{ totals of the whole subtree including self
      long _totalTime;
      long _totalSessionTime;
    //@}

    //@{ times spend on the task itself
      long _time;
      long _sessionTime;
    //@}
    DesktopList _desktops;
    TQTimer *_timer;
    int _currentPic;
    static TQPtrVector<TQPixmap> *icons;

    /** Don't need to update storage when deleting task from list. */
    bool _removing;

};

#endif // KARM_TASK_H
