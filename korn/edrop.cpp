/*
* edrop.cpp -- Implementation of class KExternDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat Nov 21 18:50:02 EST 1998
*/

#include<assert.h>

#include<ctype.h>
#include<stdlib.h>

#include<kconfig.h>
#include<kprocess.h>

#include"utils.h"
#include"dropdlg.h"
#include"edrop.h"
#include"ecfg.h"

bool KExternDrop::startMonitor()
{
	if( !valid() || _process != 0 ) {
		return false;
	}

//	debug( "proc start: %s", _command.data() );

	_process = new KProcess;
	_process->setUseShell( true );
	// only reading stdin yet
	connect( _process,SIGNAL(receivedStdout( KProcess *, char *, int)),
			this, 
			SLOT(receiveInput( KProcess *,char *,int)) );
	connect( _process, SIGNAL(processExited(KProcess*)),
			this, SLOT(processExit(KProcess*)) );

	*_process << _command;

	_process->start( KProcess::NotifyOnExit, KProcess::Stdout );

	return true;
}

bool KExternDrop::stopMonitor()
{
	if( _process != 0 ) {
//		debug( "proc stop" );
		_process->kill( SIGHUP );
		delete _process;
		_process = 0;
	}

	return true;
}

bool KExternDrop::running()
{
	return ( _process != 0 );
}

KMailDrop* KExternDrop::clone() const
{
	KExternDrop *drop = new KExternDrop;

	drop->_command = _command;

	return drop;
}

void KExternDrop::setCommand( const QString& cmd )
{
	stopMonitor();
	
	_command = cmd;

	startMonitor();
}

bool KExternDrop::readConfigGroup( const KConfigBase& cfg )
{
	KMailDrop::readConfigGroup( cfg );

	_command = cfg.readPathEntry(fu(CmdCfgKey));

	return true;
}

bool KExternDrop::writeConfigGroup( KConfigBase& cfg ) const 
{
	KMailDrop::writeConfigGroup( cfg );

	cfg.writePathEntry(fu(CmdCfgKey), _command);

	return true;
}

void KExternDrop::addConfigPage( KDropCfgDialog *dlg )
{

	dlg->addConfigPage( new KExternCfg( this ) );
	KMailDrop::addConfigPage( dlg );

}

const char *KExternDrop::CmdCfgKey = "command";


void KExternDrop::receiveInput( KProcess *proc, char *buffer, int len )
{
	assert(static_cast<void *>(proc) == static_cast<void *>(_process));

	char *buf = new char[ len + 1 ];
	memcpy( buf, buffer, len );
	buf[ len ] = '\0';

	char *ptr = buf, *start = buf;
	int num = 0;

	while( *ptr ) {
		// find number
		while( *ptr && !isdigit( *ptr ) ) {
			ptr++;
		}
		start = ptr;
		if( *ptr == 0 ) {
			break;
		}

		// find end
		while( *ptr && isdigit( *ptr ) ) {
			ptr++;
		}

		// atoi number
		char back = *ptr;
		*ptr = 0;
		num = atoi( start );
		*ptr = back;
	}

	emit changed( num );
	delete [] buf;
}

void KExternDrop::processExit( KProcess *proc )
{
	assert(static_cast<void *>(proc) == static_cast<void *>(_process));

	_process = 0;

//	debug( "proc exited" );
}
#include "edrop.moc"
