#ifndef KARM_STORAGE_H
#define KARM_STORAGE_H

#include <qdict.h>
#include <qdatetime.h>

#include "incidence.h"
#include "calendarlocal.h"
#include "journal.h"

#include "taskview.h"

class Preferences;
class Task;
class TaskView;

/**
 * Singleton to store/retrieve KArm data to/from persistent storage.
 *
 * The storage is an iCalendar file.  Also included are methods to
 * import KArm data from the two legacy file formats.
 *
 * All logic that deals with getting and saving data should go here.  The
 * storage logic has changed at least twice already in KArm's history, and
 * chances are good it will change again (for example, allowing KOrganizer and
 * KArm to access the same iCalendar file simultaneously).
 *
 * Prior to KDE 3.2, KArm just stored totals for each task--a session total
 * and a task total.  The session total was reset to zero each time KArm
 * started up or after the user reset the session times to zero.  With the
 * release of KDE 3.2, KArm now stores these task totals as well as logging
 * the history of each start/stop event; that is, every time you start a timer
 * and then stop a timer on a task, KArm records this as an iCalendar event.
 *
 * @short Logic that gets and stores KArm data to disk.
 * @author Mark Bucciarelli <mark@hubcapconsulting.com>
 * @version $Id$
 */

class KarmStorage
{
  public:
    /*
     * Return reference to storage singleton.
     *
     * The constructors are made private, so in order to create this class 
     * you must use this function.
     */
    static KarmStorage *instance();

    /*
     * Load the list view with tasks read from iCalendar file.
     *
     * Parses iCalendar file, builds list view items in the proper
     * hierarchy, and loads them into the list view widget.
     *
     * If the file name passed in is the same as the last file name that was
     * loaded, this method does nothing.
     *
     * This method considers any of the following conditions errors:
     *
     *    @li the iCalendar file does not exist
     *    @li the iCalendar file is not readable
     *    @li the list group currently has list items
     *    @li an iCalendar todo has no related to attribute
     *    @li a todo is related to another todo which does not exist
     *
     * @param taskview     The list group used in the TaskView
     * @param preferences  The current KArm preferences.
     *
     * @return empty string if success, error message if error.
     *
     */
    QString load(TaskView* taskview, const Preferences* preferences);

    /*
     * Save all tasks and their totals to an iCalendar file.
     *
     * All tasks must have an associated VTODO object already created in the
     * calendar file; that is, the task->uid() must refer to a valid VTODO in
     * the calender.
     *
     * @param taskview    The list group used in the TaskView
     */
    void save(TaskView* taskview);

    /**
     * Read tasks and their total times from a text file (legacy storage).
     *
     * This reads from one of the two legacy file formats.  In this version,
     * the parent task times do not include the sum of all their children's
     * times.
     *
     * The format of the file is zero or more lines of:
     *    1         task id (a number)
     *    time      in minutes
     *    string    task name
     *    [string]  desktops, in which to count. e.g. "1,2,5" (optional)
     */
    QString loadFromFlatFile(TaskView* taskview, const QString& filename);

    /**
     *  Reads tasks and their total times from text file (legacy).
     *
     *  This is the older legacy format, where the task totals included the
     *  children totals.
     * 
     *  @see loadFromFlatFile
     */
    QString loadFromFlatFileCumulative(TaskView* taskview,
        const QString& filename);

    /*
     * Log the change in a task's time.
     *
     * We create an iCalendar event to store each change.  The event start
     * date is set to the current datetime.  If time is added to the task, the
     * task end date is set to start time + delta.  If the time is negative,
     * the end date is set to the start time.
     *
     * In both cases (postive or negative delta), we create a custom iCalendar
     * property that stores the delta (in seconds).  This property is called 
     * X-KDE-karm-duration.
     *
     * Note that the KArm UI allows the user to change both the session and
     * the total task time, and this routine does not account for all posibile
     * cases.  For example, it is possible for the user to do something crazy
     * like add 10 minutes to the session time and subtract 50 minutes from
     * the total time.  Although this change violates a basic law of physics,
     * it is allowed.
     *
     * For now, you should pass in the change to the total task time.
     * Eventually, the UI should be changed.
     *
     * @param task   The task the change is for.  
     * @param delta  Change in task time, in seconds.  Can be negative.
     */
    void changeTime(const Task* task, const long deltaSeconds);

    /**
     * Log a change to a task name.
     *
     * For iCalendar storage, there is no need to log an Event for this event,
     * since unique id's are used to link Events to Todos.  No matter how many
     * times you change a task's name, the uid stays the same.
     *
     * @param task     The task
     * @param oldname  The old name of the task.  The new name is in the task
     *   object already.
     */
    void setName(const Task* task, const QString& oldname) {};


    /**
     * Log the event that a timer has started for a task.
     *
     * For the iCalendar storage, there is no need to log anything for this
     * event.  We log an event when the timer is stopped.
     *
     * @param task    The task the timer was started for.
     */
    void startTimer(const Task* task) {} ;

    /**
     * Log the event that the timer has stopped for this task.
     *
     * The task stores the last time a timer was started, so we log a new iCal
     * Event with the start and end times for this task.
     * @see KarmStorage::changeTime
     *
     * @param task   The task the timer was stopped for.
     */
    void stopTimer(const Task* task);

    /**
     * Log a new comment for this task.
     *
     * iCal allows multiple comment tags.  So we just add a new comment to the
     * todo for this task and write the calendar.
     *
     * @param task     The task that gets the comment 
     * @param comment  The comment
     */
    void addComment(const Task* task, const QString& comment);


    /**
     * Remove this task from iCalendar file.
     *
     * Removes task as well as all event history for this task.
     *
     * @param task   The task to be removed.
     * @return true if change was saved, false otherwise
     */
    bool removeTask(const Task* task);

    /**
     * Add this task from iCalendar file.
     *
     * Create a new KCal::Todo object and load with task information.  If
     * parent is not zero, then set the RELATED-TO attribute for this Todo.
     *
     * @param task   The task to be removed.
     * @param parent The parent of this task.  Must have a uid() that is in
     * the existing calendar.  If zero, this task is considered a root task.
     * @return The unique ID for the new VTODO.  Return an null QString if
     * there was an error creating the new calendar object.
     */
    QString addTask(const Task* task, const Task* parent);

    /**
     *  Check if the iCalendar file currently loaded has any Todos in it.
     *
     *  @return true if the CalendarLocal::rawTodos() list is not empty.
     */
    bool isEmpty();
    
    /**
     * Check if iCalendar file name in the preferences has changed since the
     * last call to load.  If there is no calendar file currently loaded,
     * return false.
     *
     * @param preferences  Set of KArm preferences.
     *
     * @return true if a previous file has been loaded and the iCalendar file
     * specified in the preferences is different.
     */
    bool isNewStorage(const Preferences* preferences) const;

  private:
    static KarmStorage *_instance;
    KCal::CalendarLocal _calendar;
    QString _icalfile;

    KarmStorage();
    void adjustFromLegacyFileFormat(Task* task);
    bool parseLine(QString line, long *time, QString *name, int *level,
        DesktopList* desktopList);
    void writeTaskAsTodo(Task* task, const int level,
        QPtrStack< KCal::Todo >& parents);

    KCal::Event* baseEvent(const Task*);

};

#endif // KARM_STORAGE_H
