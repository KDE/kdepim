/*
* news.cpp -- Implementation of class KNewsDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Thu May  7 05:29:29 EST 1998
*/

#include<assert.h>
#include<stdio.h>


#include"utils.h"
#include"news.h"

#include"newscfg.h"
#include"dropdlg.h"

#include<kconfigbase.h>
#include<mimelib/nntp.h>

const char *KNewsDrop::ServerConfigKey = "server";
const char *KNewsDrop::PortConfigKey = "port";
const char *KNewsDrop::GroupConfigKey = "group";
const char *KNewsDrop::PersistenceConfigKey = "stayconnected";

const int KNewsDrop::DefaultPort = 119;
const bool KNewsDrop::DefaultPersistence = true;

const char *KNewsDrop::DefaultServer = "news";
const char *KNewsDrop::DefaultGroup = "misc.news.newusers";

const int KNewsDrop::OkCode = 211;

KNewsDrop::KNewsDrop()
	: KPollableDrop(),
	_port( DefaultPort ), 
	_server (fu(DefaultServer)), 
	_group (fu(DefaultGroup)), 
	_persistence( false ),

	_lastStart( 0 ),
	_lastStop( 0 ),

	_socket( 0 )
{
}

KNewsDrop::~KNewsDrop()
{
	delete _socket;
}

void KNewsDrop::recheck()
{
	if( _socket == 0 ) {
		_socket = new DwNntpClient;
	}

	assert( _socket != 0 );

	if( ! _socket->IsOpen() ) {
		// connect

		_socket->Open( server().ascii(), port() );

		if( !_socket->IsOpen() ) {
			// FIXME: no connect error
			return;
		}
	}

	if( _socket->Group( group().ascii() ) == OkCode ) {
		// group ok, decode values
		int articles = decodeString( 
				_socket->StatusResponse().c_str() );

		if( articles != count() ) {
			emit changed( articles );
		}
	}

	if( persistence() == false ) {
		// disconnect on non-persistence
		_socket->Close();
	}
}

bool KNewsDrop::valid()
{
	return true;
}

KMailDrop* KNewsDrop::clone() const
{
	KNewsDrop * newdrop = new KNewsDrop;

	newdrop->setServer( server() );
	newdrop->setPort( port() );
	newdrop->setGroup( group() );
	newdrop->setPersistence( persistence() );

	return newdrop;
}

bool KNewsDrop::readConfigGroup( const KConfigBase& cfg )
{
	bool p = KPollableDrop::readConfigGroup( cfg );

	setServer( cfg.readEntry(fu(ServerConfigKey), fu(DefaultServer)) );
	setPort( cfg.readNumEntry(fu(PortConfigKey), DefaultPort) );
	setGroup( cfg.readEntry(fu(GroupConfigKey), fu(DefaultGroup)) );
	setPersistence( cfg.readNumEntry(fu(PersistenceConfigKey), 
			DefaultPersistence) );

	return p;
}

bool KNewsDrop::writeConfigGroup( KConfigBase& cfg ) const
{
	cfg.writeEntry(fu(ServerConfigKey), server() );
	cfg.writeEntry(fu(PortConfigKey), port() );
	cfg.writeEntry(fu(GroupConfigKey), group() );
	cfg.writeEntry(fu(PersistenceConfigKey), persistence() );

	return KPollableDrop::writeConfigGroup( cfg );
}

int KNewsDrop::decodeString( const char *response )
{
	int status, count, first, last;
	
	sscanf( response, "%d %d %d %d\n", 
			&status, &count, &first, &last );

	if ( _lastStart < first )  {
		_lastStart = first;
	}

	_lastStop = last;

	return (_lastStop - _lastStart);
}

void KNewsDrop::setPersistence( bool p )
{
	if( !p && (_socket != 0) && _socket->IsOpen() ) {
		_socket->Close();
	}

	_persistence = p;
}


void KNewsDrop::addConfigPage( KDropCfgDialog *dlg )
{
	assert( dlg );

	dlg->addConfigPage( new KNewsCfg( this ) );
	KPollableDrop::addConfigPage( dlg );
}

void KNewsDrop::forceCountZero()
{
	_lastStart = _lastStop;

	emit changed( _lastStop - _lastStart );
}
#include "news.moc"
