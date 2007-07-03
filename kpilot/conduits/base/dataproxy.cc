/* dataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "dataproxy.h"
#include "cudcounter.h"

DataProxy::DataProxy()
{
}

DataProxy::~DataProxy()
{
}

QVariant DataProxy::create( Record *record )
{
	Q_UNUSED(record)
	#warning Not implemented!
	return QVariant();
}

void DataProxy::remove( const QVariant &id )
{
	Q_UNUSED(id)
	#warning Not implemented!
}

void DataProxy::update( const QVariant &id, const Record *record )
{
	Q_UNUSED(record)
	Q_UNUSED(id)
	#warning Not implemented!
}

void DataProxy::syncFinished()
{
	#warning Not implemented!
}

void DataProxy::setIterateMode( const Mode m )
{
	fMode = m;
}
