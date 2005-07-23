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

#include "clientifaceimpl.h"

ClientIfaceImpl::ClientIfaceImpl( NetworkStatusModule * module ) : m_module ( module )
{
}

int ClientIfaceImpl::status( QString host )
{
	return m_module->status( host );
}

int ClientIfaceImpl::request( QString host, bool userInitiated )
{
	return m_module->request( host, userInitiated );
}

void ClientIfaceImpl::relinquish( QString host )
{
	m_module->relinquish( host );
}

bool ClientIfaceImpl::reportFailure( QString host )
{
	return m_module->reportFailure( host );
}

// QString ClientIfaceImpl::statusAsString()
// {
// 	
// }
