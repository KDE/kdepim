
#ifndef OpieHelperBase_H
#define OpieHelperBase_H

#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <ktempfile.h>

#include <kontainer.h>
#include <syncer.h>

#include <idhelper.h>


#include "categoryedit.h"

namespace OpieHelper {

    class Base {
    public:
        Base( CategoryEdit* edit =0,
              KSync::KonnectorUIDHelper* helper = 0,
              const QString &tz = QString::null,
              bool metaSyncing = FALSE);
        virtual ~Base();
    protected:
        // from tt GPLed
        time_t toUTC( const QDateTime& dt );
        QDateTime fromUTC( time_t time );
        // off tt code
        KTempFile* file();
        int newId();
        CategoryEdit* edit() { return m_edit; };
        KSync::KonnectorUIDHelper* helper() { return m_helper; };
        bool isMetaSyncingEnabled()const;
        void setMetaSyncingEnabled(bool meta);
        // returns a ; separated list of real ids
        // will also add the value m_kde2opie
        QString categoriesToNumber( const QStringList &categories, const QString &app= QString::null );
        // convience method
        QStringList categoriesToNumberList( const QStringList &categories, const QString &app = QString::null );
        QString konnectorId( const QString &appName,  const QString &uid );
        QString kdeId( const QString &appName, const QString &uid );
        CategoryEdit *m_edit;
        KSync::KonnectorUIDHelper *m_helper;
        Kontainer::ValueList m_kde2opie;
        bool m_metaSyncing : 1;
        QString m_tz;
    private:
        class BasePrivate;
        BasePrivate *baseD;
    };
};


#endif
