/* 	$Id$	 */

#ifndef _CALENDARLOCAL_H
#define _CALENDARLOCAL_H

#include <qintdict.h>
#include <qmap.h>

#include "calendar.h"

#define BIGPRIME 1031 /* should allow for at least 4 appointments 365 days/yr
			 to be almost instantly fast. */

namespace KCal {

/**
    This class provides a calendar stored as a local file.
*/
class CalendarLocal : public Calendar {
    Q_OBJECT
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    CalendarLocal();
    virtual ~CalendarLocal();
  
    /**
      loads a calendar on disk in vCalendar or iCalendar format into the current calendar.
      any information already present is lost. Returns true if successful,
      else returns false.
      @param fileName the name of the calendar on disk.
    */
    bool load(const QString &fileName);
    /** writes out the calendar to disk in the specified \a format. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    bool save(const QString &fileName,CalFormat *format=0);
    /** clears out the current calendar, freeing all used memory etc. etc. */
    void close();
  
    /** Add Event to calendar. */
    void addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /** retrieves an event on the basis of the unique string ID. */
    Event *getEvent(const QString &UniqueStr);
    /** builds and then returns a list of all events that match for the
     * date specified. useful for dayView, etc. etc. */
    QList<Event> eventsForDate(const QDate &date, bool sorted = FALSE);
    /** Get events for date \a qdt. */
    QList<Event> eventsForDate(const QDateTime &qdt);
    /** Get events in a range of dates. If inclusive is set to true, only events
     * are returned, which are completely included in the range. */
    QList<Event> events(const QDate &start,const QDate &end,
                             bool inclusive=false);
    /** Return all events in calendar */
    QList<Event> getAllEvents();
  
    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    QString getHolidayForDate(const QDate &qd);
    
    /** returns the number of events that are present on the specified date. */
    int numEvents(const QDate &qd);
  
    /** add a todo to the todolist. */
    void addTodo(Todo *todo);
    /** remove a todo from the todolist. */
    void deleteTodo(Todo *);
    const QList<Todo> &getTodoList() const;
    /** searches todolist for an event with this unique string identifier,
      returns a pointer or null. */
    Todo *getTodo(const QString &UniqueStr);
    /** Returns list of todos due on the specified date */
    QList<Todo> getTodosForDate(const QDate & date);

    /** Add a Journal entry to calendar */
    virtual void addJournal(Journal *);
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &);
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID);
    /** Return list of all Journals stored in calendar */
    QList<Journal> journalList();

  signals:
    /** emitted at regular intervals to indicate that the events in the
      list have triggered an alarm. */
    //void alarmSignal(QList<Incidence> &);
    void alarmSignal(QList<Event> &);
    void alarmSignal(QList<Todo> &);
    /** emitted whenever an event in the calendar changes.  Emits a pointer
      to the changed event. */
    void calUpdated(Incidence *);
  
  public slots:
    /** checks to see if any alarms are pending, and if so, returns a list
     * of those events that have alarms. */
    void checkAlarms();
   
  protected slots:
    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    void updateEvent(Incidence *incidence);
  
  protected:
    /** inserts an event into its "proper place" in the calendar. */
    void insertEvent(const Event *anEvent);
  
    /** on the basis of a QDateTime, forms a hash key for the dictionary. */
    long int makeKey(const QDateTime &dt);
    /** on the basis of a QDate, forms a hash key for the dictionary */
    long int makeKey(const QDate &d);
    /** Return the date for which the specified key was made. */
    QDate keyToDate(long int key);
  
  private:
    QIntDict<QList<Event> > *mCalDict;    // dictionary of lists of events.
    QList<Event> mRecursList;             // list of repeating events.

    QList<Todo> mTodoList;               // list of todo items.

    QMap<QDate,Journal *> mJournalMap;
  
    QDate *mOldestDate;
    QDate *mNewestDate;
};  

}

#endif
