
#ifndef ToDoHelper_H
#define ToDoHelper_H

#include <qmap.h>
#include <qptrlist.h>
#include <libkcal/todo.h>

class KSyncEntry;
class OpieCategories;
class KonnectorUIDHelper;
class KAlendarSyncEntry;
namespace OpieHelper {

    class ToDo {
    public:
        ToDo(KonnectorUIDHelper* helper= 0,  bool meta = FALSE);
        ~ToDo();
        QPtrList<KCal::Todo> toKDE( const QString &fileName, const QValueList<OpieCategories> & );
        QByteArray fromKDE( KAlendarSyncEntry* entry, const QValueList<OpieCategories> & );
    private:
        void setUid( KCal::Todo*,  const QString &uid );
        int uid( const QString &udi );
        QString todo2String( KCal::Todo*,  const QValueList<OpieCategories> & );
        KonnectorUIDHelper *m_helper;
        bool m_meta:1;
        // this map mapps kde uid to opie uid
        QMap<QString,  QString> kde2opie;
    };
};


#endif
