#ifndef KARM_TASK_H
#define KARM_TASK_H

#include <qlistview.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qptrvector.h>
#include <qstring.h>

#include "incidence.h"          // KCal::Incidence
#include "event.h"              // KCal::Event

#include "desktoplist.h"

class QFile;
class QTimer;

class Logging;
class TaskView;

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
 * QListViewItem class.
 */
class Task : public QObject, public QListViewItem
{
  Q_OBJECT

  public:
    //@{ constructors
    Task( const QString& taskame, long minutes, long sessionTime, 
          DesktopList desktops, TaskView* parent = 0);
    Task( const QString& taskame, long minutes, long sessionTime, 
          DesktopList desktops, Task* parent = 0);
    Task( KCal::Incidence* event, TaskView* parent );
    //@}
    /* destructor */
    ~Task();

    /** return parent Task or null in case of TaskView.
     *  same as QListViewItem::parent()
     */
    Task* firstChild() const   { return (Task *) QListViewItem::firstChild(); }
    Task* nextSibling() const  { return (Task *) QListViewItem::nextSibling(); }
    Task* parent() const       { return (Task *) QListViewItem::parent(); }

    /** cut Task out of parent Task or the TaskView */
    void cut();
    /** cut Task out of parent Task or the TaskView and into the
     *  destination Task */
    void move(Task* destination);
    /** insert Task into the destination Task */
    void paste(Task* destination);

    //@{ timing related functions
 
      /** adds minutes to time and session time
       *
       *  @param minutes        minutes to add to - may be negative
       *  @param do_loggin      should the change be logged?
       */
      void changeTime( long minutes, bool do_logging )
                { changeTimes( minutes, minutes, do_logging); };

      /** adds minutes to time and session time
       *
       *  @param minutesSession   minutes to add to task session time
       *  @param minutes          minutes to add to task time
       *  @param do_loggin        should the change be logged?
       */
      void changeTimes( long minutesSession, long minutes,
                        bool do_logging);

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
        long time() const        { return _time; };
        long sessionTime() const { return _sessionTime; };

      /** sets session time to zero. */
      // TODO: log
      void startNewSession() { changeTimes( -_sessionTime, 0, false); };
    //@}

    //@{ desktop related functions

      void setDesktopList ( DesktopList dl );
      DesktopList getDesktops() const { return _desktops;}

      QString getDesktopStr() const;
    //@}

    //@{ name related functions

      /** sets the name of the task
       *  @param name    a pointer to the name. A deep copy will be made.
       */
      void setName( const QString& name );

      /** returns the name of this task.
       *  @return a pointer to the name.
       */
      QString name() const  { return _name; };
    //@}

    /** updates the content of the QListViewItem with respect to _name and
     *  all the times */
    void update();

    //@{ the state of a Task - stopped, running

      /** starts or stops a task
       *  @param on       true or false for starting or stopping a task */
      void setRunning(bool on);

      /** return the state of a task - if it's running or not
       *  @return         true or false depending on whether the task is running
       */
      bool isRunning() const;
    //@}

    static bool parseIncidence( KCal::Incidence*, long&, QString&, int&,
                                DesktopList& );

    KCal::Event* asEvent( int level );

    /** adds a comment to the task
     *  currently only being passed through to the logger
     */
    void addComment( QString comment );

    /** tells you whether this task is the root of the task tree */
    bool isRoot() const                 { return parent() == 0; }

    /** remove Task with all it's children
     * @param   activeTasks - list of aktive tasks
     */
    void remove( QPtrList<Task>& activeTasks );

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
    long totalTimeInSeconds() const      { return _totalTime * 60; }
    /** if the time or session time is negative set them to zero */
    void noNegativeTimes();
    /** initialize a task */
    void init( const QString& taskame, long minutes, long sessionTime, 
               DesktopList desktops);


    QString _name;
    //@{ totals of the whole subtree including self
      long _totalTime;
      long _totalSessionTime;
    //@}
    //@{ times spend on the task itself
      long _time;
      long _sessionTime;
    //@}
    DesktopList _desktops;
    QTimer *_timer;
    int _currentPic;
    static QPtrVector<QPixmap> *icons;
    Logging *_logging;

};

#endif // KARM_TASK_H
