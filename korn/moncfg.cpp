/*
* moncfg.cpp -- Implementation of class KMonitorCfg.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Wed Jul 29 03:47:49 EST 1998
*/

#include <assert.h>
//#include <qwidget.h>

#include "moncfg.h"
#include "maildrop.h"
//#include"typolayout.h"

KMonitorCfg::KMonitorCfg( KMailDrop *drop )
	: _drop ( drop ) 
{ 
	assert( drop );

	connect(_drop, SIGNAL(notifyDisconnect()),
		this, SLOT(monitorDead())); 
}

#include "moncfg.moc"
