
#ifndef OpieHelperDateBook_H
#define OpieHelperDateBook_H

#include <qptrlist.h>
#include <libkcal/event.h>

#include "helper.h"

class KAlendarSyncEntry;
namespace OpieHelper {

    class DateBook : public Base {
    public:
        DateBook( CategoryEdit* edit = 0,
                  KonnectorUIDHelper* helper = 0,
                  bool meta = FALSE );
        ~DateBook();
        QPtrList<KCal::Event> toKDE( const QString & fileName );
        QByteArray fromKDE( KAlendarSyncEntry* entry );
    };
};
#endif
