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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef NETWORKSTATUS_COMMON_H
#define NETWORKSTATUS_COMMON_H

#include <qstringlist.h>

namespace NetworkStatus
{
	enum EnumStatus { NoNetworks = 1, Unreachable, OfflineDisconnected,  OfflineFailed, ShuttingDown, Offline, Establishing, Online };
	enum EnumRequestResult { RequestAccepted = 1, Connected, UserRefused, Unavailable };
	enum EnumOnDemandPolicy { All, User, None, Permanent };
	struct Properties
	{
		QString name;
		// status of the network
		EnumStatus status;
		// policy for on-demand usage as defined by the service
		EnumOnDemandPolicy onDemandPolicy;
		// identifier for the service
		QCString service;
		// indicate that the connection is to 'the internet' - similar to default gateway in routing
		bool internet;
		// list of netmasks that the network connects to - overridden by above internet
		QStringList netmasks;
		// for future expansion consider
		// EnumChargingModel - FlatRate, TimeCharge, VolumeCharged
		// EnumLinkStatus - for WLANs - VPOOR, POOR, AVERAGE, GOOD, EXCELLENT
	};
}

QDataStream & operator>> ( QDataStream & s, NetworkStatus::Properties &p );
QDataStream & operator<< ( QDataStream & s, const NetworkStatus::Properties p );

#endif
