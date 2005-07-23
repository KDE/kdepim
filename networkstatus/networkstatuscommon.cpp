/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <lists@stevello.free-online.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "networkstatuscommon.h"
#include <kdebug.h>

QDataStream & operator<< ( QDataStream & s, const NetworkStatus::Properties p )
{
	kdDebug() << k_funcinfo << "status is: " << (int)p.status << endl;
	s << (int)p.status;
	s << (int)p.onDemandPolicy;
	s << p.service;
	s << ( p.internet ? 1 : 0 );
	s << p.netmasks;
	return s;
}

QDataStream & operator>> ( QDataStream & s, NetworkStatus::Properties &p )
{
	int status, onDemandPolicy, internet;
	s >> status;
	kdDebug() << k_funcinfo << "status is: " << status << endl;
	p.status = ( NetworkStatus::EnumStatus )status;
	s >> onDemandPolicy;
	p.onDemandPolicy = ( NetworkStatus::EnumOnDemandPolicy )onDemandPolicy;
	s >> p.service;
	s >> internet;
	if ( internet )
		p.internet = true;
	else
		p.internet = false;
	s >> p.netmasks;
	kdDebug() << k_funcinfo << "enum converted status is: " << p.status << endl;
	return s;
}
