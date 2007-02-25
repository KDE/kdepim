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


#include "kio_proto.h"

#include <kconfig.h>
#include <kdebug.h>

#include <QMap>

QMap< QString, QString >* KIO_Protocol::createConfig( AccountSettings *settings ) const
{
	return new QMap< QString, QString >( settings->readEntries() );
}

void KIO_Protocol::clearFields( QMap<QString, QString> *map, const KIO_Protocol::Fields fields ) const
{
	if( fields & server )
		map->insert( "server", "" );
	if( fields & port )
		map->insert( "port", "" );
	if( fields & username )
		map->insert( "username", "" );
	if( fields & password )
		map->insert( "password", "" );
	if( fields & mailbox )
		map->insert( "mailbox", "" );
	if( fields & save_password  )
		map->insert( "savepassword", "" );
	if( fields & metadata )
		map->insert( "metadata", "" );
}

void KIO_Protocol::readEntries( QMap< QString, QString >* map ) const
{
	QMap< QString, QString> *metadata = new QMap< QString, QString >;

	if( map->contains( "metadata" ) )
	{
		QStringList list = (*map->find( "metadata" )).split( ",", QString::SkipEmptyParts );
		QStringList::Iterator it;
		for( it = list.begin(); it != list.end(); ++it )
		{
			int split = (*it).indexOf( '=', 0, Qt::CaseSensitive );

			metadata->insert( (*it).left( split ), (*it).right( (*it).length() - split - 1 ) );
		}
	}

	this->readEntries( map, metadata );

	delete metadata;
}
