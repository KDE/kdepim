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

#ifndef NETWORKSTATUS_PROVIDERIFACE_H
#define NETWORKSTATUS_PROVIDERIFACE_H

#include <dcopobject.h>
class ProviderIface : virtual public DCOPObject
{
K_DCOP
k_dcop:
	/** @return NetworkStatus::EnumOnlineStatus */
	virtual int status( const QString & network ) = 0;
	/** @return NetworkStatus::EnumRequestResult */
	virtual int establish( const QString & network ) = 0;
	/** @return NetworkStatus::EnumRequestResult */
	virtual int shutdown( const QString & network ) = 0;
	/** fake a failure - go directly to failed */
	virtual void simulateFailure() = 0;
	/** fake a network disconnect - go directly to offlinedisconnected */
	virtual void simulateDisconnect() = 0;
};

#endif
