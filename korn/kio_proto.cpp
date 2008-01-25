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

#include <qmap.h>

QMap< QString, QString >* KIO_Protocol::createConfig( KConfigGroup * group, const QString& password ) const
{
	QMap< QString, QString > *result = new QMap< QString, QString >;

	result->insert( "name", group->readEntry( "name", "" ) );
	result->insert( "server", group->readEntry( "server", "" ) );
	result->insert( "port", group->readEntry( "port", "" ) );
	result->insert( "username", group->readEntry( "username", "" ) );
	result->insert( "password", password );
	result->insert( "mailbox", group->readEntry( "mailbox", "" ) );
	result->insert( "savepassword", group->readEntry( "savepassword", "false" ) );
	result->insert( "ssl", group->readEntry( "ssl", "false" ) );
	result->insert( "metadata", group->readEntry( "metadata", "" ) );

	return result;
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
		QStringList list = QStringList::split( ",", *map->find( "metadata" ) );
		QStringList::Iterator it;
		for( it = list.begin(); it != list.end(); ++it )
		{
			int split = (*it).find( '=' );

			metadata->insert( (*it).left( split ), (*it).right( (*it).length() - split - 1 ) );
		}
	}

	this->readEntries( map, metadata );

	delete metadata;
}
