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

#ifndef NETWORKSTATUS_CLIENTIFACEIMPL_H
#define NETWORKSTATUS_CLIENTIFACEIMPL_H

#include <qstring.h>

#include "clientiface.h"
#include "networkstatus.h"

class ClientIfaceImpl : virtual public ClientIface
{
public:
	ClientIfaceImpl( NetworkStatusModule * module );
	int status( QString host );
	int request( QString host, bool userInitiated );
	void relinquish( QString host );
	bool reportFailure( QString host );
/*	QString statusAsString();*/
private:
	NetworkStatusModule * m_module;
};

#endif
