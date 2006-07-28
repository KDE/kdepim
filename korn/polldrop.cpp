/*
* polldrop.cpp -- Implementation of class KPollableDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sun Nov 30 22:41:49 EST 1997
*/


#include<kconfigbase.h>

#include"utils.h"
#include"polldrop.h"
#include"settings.h"
//Added by qt3to4:
#include <QTimerEvent>

KPollableDrop::KPollableDrop()
	: KMailDrop()
{
	_timerId = 0;
	_timerRunning = false;
}


bool KPollableDrop::startMonitor()
{
	if( !running() && _settings ) {
		recheck();
		
		_timerId = startTimer( _settings->interval() * 1000 );
		_timerRunning = true;
	}

	return false;
}

bool KPollableDrop::stopMonitor()
{
	if( running() ) {
		killTimer( _timerId );
		_timerId = 0;
		_timerRunning = false;
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

bool KPollableDrop::readConfig( AccountSettings *settings )
{
	return KMailDrop::readConfig( settings );
}

bool KPollableDrop::writeConfigGroup( AccountSettings *settings ) const
{
	return KMailDrop::writeConfigGroup( settings );
}

#include "polldrop.moc"

