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

class VCalDrag : public QStoredDrag {
  public:
    VCalDrag(VObject *vcal, QWidget *parent=0, const char *name=0);
    ~VCalDrag() {};

    static bool canDecode(QMimeSource *);
    static bool decode(QMimeSource *e, VObject **vcal);
};

}

#endif
