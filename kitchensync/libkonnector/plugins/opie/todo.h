
#ifndef ToDoHelper_H
#define ToDoHelper_H

#include <qmap.h>
#include <qvaluelist.h>
#include <libkcal/todo.h>

#include <kontainer.h>

#include "helper.h"

class KSyncEntry;
class OpieCategories;
class KonnectorUIDHelper;
class KAlendarSyncEntry;
namespace OpieHelper {

    class ToDo  : public Base {
    public:
        ToDo( CategoryEdit* edit = 0,
              KonnectorUIDHelper* helper= 0,
              bool meta = FALSE);
        ~ToDo();

        QPtrList<KCal::Todo> toKDE( const QString &fileName );
        QByteArray fromKDE( KAlendarSyncEntry* entry  );
    private:
        void setUid( KCal::Todo*,  const QString &uid );
        QString todo2String( KCal::Todo*  );
    };
};


#endif
