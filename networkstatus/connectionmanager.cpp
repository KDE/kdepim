#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>

#include "clientiface_stub.h"
#include "networkstatuscommon.h"

#include "connectionmanager.h"

// ConnectionManager's private parts
class ConnectionManagerPrivate
{
	public:
		// this holds the currently active state
		ConnectionManager::State m_state;
		ClientIface_stub * m_stub;
		bool m_userInitiatedOnly;
};

// Connection manager itself
ConnectionManager::ConnectionManager( QObject * parent, const char * name ) : DCOPObject( "ConnectionManager" ),QObject( parent, name )
{
	d = new ConnectionManagerPrivate;
	
	d->m_stub = new ClientIface_stub( kapp->dcopClient(), "kded", "networkstatus" );
	
	connectDCOPSignal( "kded", "networkstatus", "statusChange(QString,int)", "slotStatusChanged(QString,int)", false );
	d->m_userInitiatedOnly = false;
	initialise();
}

ConnectionManager *ConnectionManager::s_self = 0L;

ConnectionManager *ConnectionManager::self()
{
	static KStaticDeleter<ConnectionManager> deleter;
	if(!s_self)
		deleter.setObject( s_self, new ConnectionManager( 0, "connection_manager" ) );
	return s_self;	
}

void ConnectionManager::initialise()
{
	// determine initial state and set the state object accordingly.
	d->m_state = Offline;
	updateStatus();
}

void ConnectionManager::updateStatus()
{
	/*NetworkStatus::EnumStatus daemonStatus = (NetworkStatus::EnumStatus)d->m_stub->status( QString::null );
	switch ( daemonStatus )
	{
		case Offline:
		case OfflineFailed:
		case OfflineDisconnected:
		case ShuttingDown:
			if ( d->m_state == Online )
				d->m_state = Pending;
			else
				d->m_state = Offline;
			break;
		case Establishing:
		case Online:
			d->m_state = Online;
			break;
		case NoNetworks:
		case Unreachable:
			d->m_state = Inactive;
			break;
	}*/
}

ConnectionManager::~ConnectionManager()
{
	delete d;
}

NetworkStatus::EnumStatus ConnectionManager::status( const QString & host )
{
	if ( d->m_state == Inactive )
		return NetworkStatus::NoNetworks;
	else
		return NetworkStatus::Offline;
}
NetworkStatus::EnumRequestResult ConnectionManager::requestConnection( QWidget * mainWidget, const QString & host, bool userInitiated )
{
	NetworkStatus::EnumRequestResult result;
	// if offline and the user has previously indicated they didn't want any new connections, suppress it
	if ( d->m_state == Offline && !userInitiated && d->m_userInitiatedOnly )
		result = NetworkStatus::UserRefused;
	// if offline, ask the user whether this connection should be allowed
	if ( d->m_state == Offline )
	{
		if ( askToConnect( mainWidget ) )
			result = (NetworkStatus::EnumRequestResult)d->m_stub->request( host, userInitiated );
		else
			result = NetworkStatus::UserRefused;
	}
	// otherwise, just ask for the connection
	else
		result = (NetworkStatus::EnumRequestResult)d->m_stub->request( host, userInitiated );
	
	return result;
}

void ConnectionManager::relinquishConnection( const QString & host )
{
	d->m_stub->relinquish( host );
}

void ConnectionManager::slotStatusChanged( QString host, int status )
{
	updateStatus();
	// reset user initiated only flag if we are now online
	if ( d->m_state == Online )
		d->m_userInitiatedOnly = false;

	emit statusChanged( host, (NetworkStatus::EnumStatus)status );
}

bool ConnectionManager::askToConnect( QWidget * mainWidget )
{
	i18n( "A network connection was disconnected.  The application is now in offline mode.  Do you want the application to resume network operations when the network is available again?" );
	i18n( "This application is currently in offline mode.  Do you want to connect?" );
	i18n( "Message shown when a network connection failed.  The placeholder contains the concrete description of the operation eg 'while performing this operation", "A network connection failed %1.  Do you want to place the application in offline mode?" );
	return ( KMessageBox::questionYesNo( mainWidget,
			 i18n("This application is currently in offline mode.  Do you want to connect in order to carry out this operation?"),
																		i18n("Leave offline mode?"),
																		KStdGuiItem::yes(), KStdGuiItem::no(),
																		QString::fromLatin1("OfflineModeAlwaysGoOnline") ) == KMessageBox::Yes );
}

#include "connectionmanager.moc"
