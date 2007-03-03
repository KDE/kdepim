#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include <qobject.h>

#include <dbus/qdbusobject.h>

class QDBusMessage;
class QDBusConnection;

class OpenSyncService : public QDBusObjectBase
{
  public:
    OpenSyncService();

    void setConnection( QDBusConnection *connection );

  protected:
    virtual bool handleMethodCall( const QDBusMessage &message );

    QDBusMessage hello( const QDBusMessage & );
    QDBusMessage randomNumber( const QDBusMessage & );

    QDBusMessage listGroups( const QDBusMessage &message );
    QDBusMessage listPlugins( const QDBusMessage &message );
    QDBusMessage showGroup( const QDBusMessage &message );
    QDBusMessage showMember( const QDBusMessage &message );

    QDBusMessage error( const QDBusMessage &, const QString &errorCode,
      const QString &errorMessage );

  private:
    QDBusConnection *mConnection;
};

#endif
