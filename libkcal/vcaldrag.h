/* 
 * VCalendar Xdnd Drag Object.
 * (c) 1998 Preston Brown
 * Created for the KOrganizer project
 */

#ifndef VCALDRAG_H
#define VCALDRAG_H

#include <qdragobject.h>

class VObject;

namespace KCal {

/** vCalendar drag&drop class. */
class VCalDrag : public QStoredDrag {
  public:
    /** Create a drag&drop object for vCalendar component \a vcal. */
    VCalDrag(VObject *vcal, QWidget *parent=0, const char *name=0);
    ~VCalDrag() {};

    /** Return, if drag&drop object can be decode to vCalendar. */
    static bool canDecode(QMimeSource *);
    /** Decode drag&drop object to vCalendar component \a vcal. */
    static bool decode(QMimeSource *e, VObject **vcal);
};

}

#endif
