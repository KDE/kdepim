
#ifndef idhelper_h
#define idhelper_h

#include <qmap.h>
#include <qvaluelist.h>

#include <kontainer.h> // from libksyncentry

class KConfig;
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
                     QValueList<Kontainer> );
    void removeId(const QString &app,  const QString &id);
    void clear();
    void save();

private:
    class KonnectorUIDHelperPrivate;
    KonnectorUIDHelperPrivate *d;
    KConfig *m_config;
    QMap< QString,  QValueList<Kontainer> > m_ids;
};

#endif
