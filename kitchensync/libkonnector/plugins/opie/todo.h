
#ifndef ToDoHelper_H
#define ToDoHelper_H

#include <qmap.h>
#include <qvaluelist.h>
#include <libkcal/todo.h>

#include <kontainer.h>

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
        QString konnectorId(const QString &appName,  const QString &uid );
        QString todo2String( KCal::Todo*,  const QValueList<OpieCategories> & );
        KonnectorUIDHelper *m_helper;
        bool m_meta:1;
        // this map mapps kde uid to opie uid
        QValueList<Kontainer> kde2opie;
    };
};


#endif
