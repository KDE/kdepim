#ifndef ICALFORMAT_H
#define ICALFORMAT_H
// $Id$

#include <qstring.h>

#include "scheduler.h"

#include "calformat.h"

namespace KCal {

class ICalFormatImpl;

/**
  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal KOrganizer
  representation as Calendar and Events.

  @short iCalendar format implementation
*/
class ICalFormat : public CalFormat {
  public:
    /** Create new iCal format for calendar object */
    ICalFormat(Calendar *);
    virtual ~ICalFormat();

    /**
      loads a calendar on disk in iCalendar format  into current calendar.
      Returns TRUE if successful, else returns FALSE. Provides more error
      information by exception().
      @param fileName the name of the calendar on disk.
    */
    bool load(const QString &fileName);
    /** writes out the calendar to disk in iCalendar format. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    bool save(const QString &fileName);
  
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDrag(Event *selectedEv, QWidget *owner);
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDragTodo(Todo *selectedEv, QWidget *owner);
    /** Create Todo object from drop event */
    Todo *createDropTodo(QDropEvent *de);
    /** Create Event object from drop event */
    Event *createDrop(QDropEvent *de);
  
    /** cut, copy, and paste operations follow. */
    bool copyEvent(Event *);
    /** pastes the event and returns a pointer to the new event pasted. */
    Event *pasteEvent(const QDate *, const QTime *newTime = 0L);
    
    /** Create a scheduling message for event \a e using method \m */
    QString createScheduleMessage(Incidence *e,Scheduler::Method m);
    /** Parse scheduling message provided as string \s */
    ScheduleMessage *parseScheduleMessage(const QString &s);
    
  private:
    ICalFormatImpl *mImpl;
};

}

#endif
