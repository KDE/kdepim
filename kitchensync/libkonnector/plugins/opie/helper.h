
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <kontainer.h>
#include <idhelper.h>
#include "categoryedit.h"

namespace OpieHelper {

    class Base {
    public:
        Base( CategoryEdit* edit =0,
              KonnectorUIDHelper* helper = 0,
              bool metaSyncing = FALSE);
        virtual ~Base();
    protected:
        CategoryEdit* edit() { return m_edit; };
        KonnectorUIDHelper* helper() { return m_helper; };
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
        KonnectorUIDHelper *m_helper;
        QValueList<Kontainer> m_kde2opie;
        bool m_metaSyncing : 1;
    private:
        class BasePrivate;
        BasePrivate *baseD;
    };
};
