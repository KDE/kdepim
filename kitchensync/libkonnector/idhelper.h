
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
        /**
         * c'tor
         * @param dir The directory name where the relations
         *            between KDE and the Konnector UIDs
         *            are saved
         */
        KonnectorUIDHelper( const QString &dir );
        ~KonnectorUIDHelper();

        /**
         * @param appName The Application Name. For example 'datebook'
         * @param kdeId the UID assigned to in the KDE world
         * @param defaultId in case of failure what should be returned
         *
         * @return returns a to the Konnector known uid
         */
        QString konnectorId( const QString &appName,
                             const QString &kdeId,
                             const QString &defaultId = QString::null );

        /**
         * fetches a KDE UID for an appName
         * and a konnectorUID
         */
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
