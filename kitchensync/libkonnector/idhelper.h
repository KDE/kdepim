
#ifndef idhelper_h
#define idhelper_h

#include <qmap.h>
#include <qvaluelist.h>

#include <kontainer.h> // from libksyncentry

class KConfig;
/**
 * the KonnectorUIDHelper helps to manage the
 * relation between the KDE uids and the KonnectorPlugin
 * uids. This makes finding ids later more easy
 */
namespace KSync {
    class KonnectorUIDHelper {
    public:
        // the full path to the dir where the file is stored
        KonnectorUIDHelper( const QString &dir );
        ~KonnectorUIDHelper();
        QString konnectorId( const QString &appName,
                             const QString &kdeId,
                             const QString &defaultId = QString::null );

        QString kdeId( const QString &appName,
                       const QString &konnectorId,
                       const QString &defaultId = QString::null );

        void addId(const QString& appName,
                   const QString& konnectorId,
                   const QString& kdeId);
        void replaceIds( const QString& appName,
                         Kontainer::ValueList );
        void removeId(const QString &app,  const QString &id);
        void clear();
        void save();

    private:
        class KonnectorUIDHelperPrivate;
        KonnectorUIDHelperPrivate *d;
        KConfig *m_config;
        QMap< QString,  Kontainer::ValueList > m_ids;
    };
};
#endif
