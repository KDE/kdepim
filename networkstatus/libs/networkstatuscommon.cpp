/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "networkstatuscommon.h"

QDataStream & operator<< ( QDataStream & s, const NetworkStatus::Properties p )
{
  s << p.name;
	s << (int)p.status;
	s << p.service;
	return s;
}

QDataStream & operator>> ( QDataStream & s, NetworkStatus::Properties &p )
{
	int status;
	s >> p.name;
	s >> status;
	p.status = (NetworkStatus::Status)status;
	s >> p.service;
	return s;
}

namespace NetworkStatus
{
  QString toString( NetworkStatus::Status st )
  {
    QString str;
    switch ( st ) {
      case NetworkStatus::NoNetworks:
        str = "NoNetworks";
        break;
      case NetworkStatus::Unreachable:
        str = "Unreachable";
        break;
      case NetworkStatus::OfflineDisconnected:
        str = "OfflineDisconnected";
        break;
      case NetworkStatus::OfflineFailed:
        str = "OfflineFailed";
        break;
      case NetworkStatus::ShuttingDown:
        str = "ShuttingDown";
        break;
      case NetworkStatus::Offline:
        str = "Offline";
        break;
      case NetworkStatus::Establishing:
        str = "Establishing";
        break;
      case NetworkStatus::Online:
        str = "Online";
        break;
    }
    return str;
  }
} // namespace NetworkStatus
