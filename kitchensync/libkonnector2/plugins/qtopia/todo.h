
#ifndef ToDoHelper_H
#define ToDoHelper_H


#include <qdom.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <libkcal/todo.h>

#include <kontainer.h>

#include "helper.h"

class OpieCategories;

namespace KSync {
    class KonnectorUIDHelper;
    class Syncee;
    class TodoSyncee;
}
namespace OpieHelper {

    class ToDo  : public Base {
    public:
        ToDo( CategoryEdit* edit = 0,
              KSync::KonnectorUIDHelper* helper= 0,
              const QString& tz = QString::null,
              bool meta = FALSE, Device* dev = 0);
        ~ToDo();

        KSync::TodoSyncee* toKDE( const QString &fileName, ExtraMap& map );
        KTempFile* fromKDE( KSync::TodoSyncee* entry, ExtraMap& map  );
    private:
        QStringList attributes()const;
        void setUid( KCal::Todo*,  const QString &uid );
        KCal::Todo* dom2todo( QDomElement, ExtraMap&, const QStringList& );
        QString todo2String( KCal::Todo*, ExtraMap&  );
    };
};


#endif
