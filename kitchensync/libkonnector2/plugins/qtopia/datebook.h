
#ifndef OpieHelperDateBook_H
#define OpieHelperDateBook_H

#include <qptrlist.h>
#include <libkcal/event.h>

#include "helper.h"

namespace KSync{
    class EventSyncee;
};
class QDomElement;
namespace OpieHelper {
    enum Days { Monday=1,
                Tuesday =2,
                Wednesday =4,
                Thursday = 8,
                Friday = 16,
                Saturday = 32,
                Sunday = 64 };

    class DateBook : public Base {
    public:
        DateBook( CategoryEdit* edit = 0,
                  KSync::KonnectorUIDHelper* helper = 0,
                  const QString &tz = QString::null,
                  bool meta = FALSE, Device* dev = 0);
        ~DateBook();
        KSync::EventSyncee* toKDE( const QString & fileName, ExtraMap& map );
        KTempFile* fromKDE( KSync::EventSyncee* syncee, ExtraMap& map );
    private:
        QStringList attributes()const;
        QString endDate( const QDateTime& time, bool allDay );
        QString startDate( const QDateTime& time, bool allDay );
        QString event2string( KCal::Event *event, ExtraMap& );
        KCal::Event* toEvent( QDomElement, ExtraMap&, const  QStringList& lst );
    };
};
#endif
