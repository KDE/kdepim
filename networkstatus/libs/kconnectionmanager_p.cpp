#include "clientinterface.h"

#include "kconnectionmanager_p.h"

KConnectionManagerPrivate::KConnectionManagerPrivate(QObject * parent ) : QObject( parent ), service( new OrgKdeSolidNetworkingClientInterface( "org.kde.kded", "/modules/networkstatus", QDBusConnection::sessionBus(), this ) ),
  connectPolicy( KConnectionManager::Managed ),
  disconnectPolicy( KConnectionManager::Managed ),
  connectReceiver( 0 ), connectSlot( 0 ),
  disconnectReceiver( 0 ), disconnectSlot( 0 )
{
}

KConnectionManagerPrivate::~KConnectionManagerPrivate()
{
}

#include "kconnectionmanager_p.moc"
