
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
              bool meta = FALSE);
        ~ToDo();

        KSync::TodoSyncee* toKDE( const QString &fileName );
        void fromKDE( KSync::TodoSyncee* entry, const QString&  );
    private:
        void setUid( KCal::Todo*,  const QString &uid );
        KCal::Todo* dom2todo( QDomElement );
        QString todo2String( KCal::Todo*  );
    };
};


#endif
