
#ifndef OpieHelperDateBook_H
#define OpieHelperDateBook_H

#include <qptrlist.h>
#include <libkcal/event.h>

#include "helper.h"

class KAlendarSyncEntry;
namespace OpieHelper {
    enum Days { Monday=1,  Tuesday =2,  Wednesday =4,  Thursday = 8,  Friday = 16,  Saturday = 32,  Sunday = 64 };
    class DateBook : public Base {
    public:
        DateBook( CategoryEdit* edit = 0,
                  KonnectorUIDHelper* helper = 0,
                  const QString &tz = QString::null,
                  bool meta = FALSE );
        ~DateBook();
        QPtrList<KCal::Event> toKDE( const QString & fileName );
        QByteArray fromKDE( KAlendarSyncEntry* entry );
    private:
        QString event2string( KCal::Event *event );
    };
};
#endif
