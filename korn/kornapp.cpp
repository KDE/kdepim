/*
* kornapp.cpp -- Implementation of class KornApp.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sun Apr 22 23:50:49 EST 2001
*/

#include"kornapp.h"
#include<kdebug.h>
#include"kornshell.h"

int KornApp::newInstance()
{
	if( _instanceCount ) {
		_shell->optionDlg();
	}
	_instanceCount++;

	return 0;
}
#include "kornapp.moc"
