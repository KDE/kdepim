/*
* polldrop.cpp -- Implementation of class KPollableDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sun Nov 30 22:41:49 EST 1997
*/


#include<kconfigbase.h>

#include"utils.h"
#include"polldrop.h"

KPollableDrop::KPollableDrop()
	: KMailDrop()
{
	_timerId = 0;
	_timerRunning = false;
	_freq = 300;
}


bool KPollableDrop::startMonitor()
{
	if( !running() ) {
		recheck();
		
		_timerId = startTimer( _freq * 1000 );
		_timerRunning = true;

		return startProcess();
	}

	return false;
}

bool KPollableDrop::stopMonitor()
{
	if( running() ) {
		killTimer( _timerId );
		_timerId = 0;
		_timerRunning = false;

		return stopProcess();
	}

	return false;
}


void KPollableDrop::timerEvent( QTimerEvent *ev )
{
	if( _timerRunning && (ev->timerId() == _timerId) ) {
		// this event is ours.
		recheck(); // should be reimplemented by children.
	}
	else {
		QObject::timerEvent( ev );
	}
}

bool KPollableDrop::readConfigGroup( const KConfigBase& cfg )
{
	KMailDrop::readConfigGroup( cfg );

	setFreq( cfg.readNumEntry(fu(PollConfigKey), DefaultPoll ) );

	return true;
}

bool KPollableDrop::writeConfigGroup( KConfigBase& cfg ) const
{
	KMailDrop::writeConfigGroup( cfg );

	cfg.writeEntry(fu(PollConfigKey), freq() );

	return true;
}

//void KPollableDrop::addConfigPage( KDropCfgDialog *dlg )
//{
//	dlg->addConfigPage( new KPollCfg( this ) );
//
//	KMailDrop::addConfigPage( dlg );
//}

const char *KPollableDrop::PollConfigKey = "interval";
const int KPollableDrop::DefaultPoll = 300; // 5 minutes

#include "polldrop.moc"
