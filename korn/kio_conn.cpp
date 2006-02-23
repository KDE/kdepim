/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kio_conn.h"

#include "kio_proto.h"

#include <kdebug.h>
#include <kio/scheduler.h>
#include <kio/slave.h>

#include <qmap.h>

QMap< KIO::Slave*, int >* KIO_Connection::m_slaves = 0;

KIO::Slave* KIO_Connection::getSlave( const KUrl& kurl, const KIO::MetaData& metadata, const KIO_Protocol* protocol )
{
	KIO::Slave *slave;
	if( m_slaves == 0 )
		initData();

	if( !protocol->connectionBased() )
		return 0; /* No slave needed */

	/* Look if slave already exist for this connection; is so, use it */
	QMap< KIO::Slave*, int >::Iterator it;
	for( it = m_slaves->begin(); it != m_slaves->end(); ++it )
	{
		if( it.key()->protocol() == kurl.protocol() && it.key()->host() == kurl.host() &&
		    it.key()->user() == kurl.user() && it.key()->passwd() == kurl.pass() )
		{
			++(*it); //Increse usage number
			return it.key();
		}
	}

	//No connection already exists.
	if ( !(slave = KIO::Scheduler::getConnectedSlave( kurl, metadata ) ) )
		return 0; // Error while opening slave
	
	m_slaves->insert( slave, 1 ); //Add slave to mapping.
	return slave;
}

void KIO_Connection::removeSlave( KIO::Slave* slave )
{
	if( m_slaves == 0 )
		initData();

	if( !m_slaves->contains( slave ) )
		kDebug() << "(KIO_Connection): Not able to remove a non-existing slave." << endl;

	QMap< KIO::Slave*, int >::Iterator it = m_slaves->find( slave );
	if( it.value() == 1 )
	{
		KIO::Scheduler::disconnectSlave( slave ); //Disconnect slave
		m_slaves->erase( it );  //Remove it from list
	}
	else
		--(*it); //Decrease usage number
		
}

void KIO_Connection::initData()
{
	m_slaves = new QMap< KIO::Slave*, int >;
}

