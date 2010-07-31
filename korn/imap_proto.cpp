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


#include "imap_proto.h"

#include "account_input.h"

#include <tqwidget.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqptrvector.h>
#include <tqptrlist.h>

void Imap_Protocol::configFillGroupBoxes( TQStringList* groupBoxes ) const
{
	groupBoxes->append( "Server" );
	groupBoxes->append( "Identity" );
}

void Imap_Protocol::configFields( TQPtrVector< TQWidget >* vector, const TQObject* configDialog, TQPtrList< AccountInput > * result ) const
{
	TQMap< TQString, TQString > encrList;
	encrList.insert( "ssl", i18n( "SSL" ) );
	encrList.insert( "tls=auto", i18n( "TLS if possible" ) );
	encrList.insert( "tls=on", i18n( "Always TLS" ) );
	encrList.insert( "tls=off", i18n( "Never TLS" ) );
	
	TQMap< TQString, TQString > authList;
	authList.insert( "auth=*", i18n( "Default" ) );
	authList.insert( "auth=LOGIN", i18n( "LOGIN" ) ); //Note: LOGIN is an authentication method
	authList.insert( "auth=ANONTMOUS", i18n( "Anonymous" ) ); //Note: ANONYMOUS is an authentication method
	authList.insert( "auth=CRAM-MD5", i18n( "CRAM-MD5" ) ); //Note: CRAM-MD5 is an authentication method

	result->append( new TextInput( (TQWidget*)vector->at( 0 ), i18n( "Server" ), TextInput::text, "", "server" ) );
	result->append( new TextInput( (TQWidget*)vector->at( 0 ), i18n( "Port" ), 0, 65535, "143", "port" ) );
	result->append( new ComboInput( (TQWidget*)vector->at( 0 ), i18n( "Encryption" ), encrList, "tls=auto", "encryption" ) );
	TQObject::connect( (TQObject*)result->last()->rightWidget(), TQT_SIGNAL( activated( int) ),
	                  configDialog, TQT_SLOT( slotSSLChanged() ) );
	
	result->append( new TextInput( (TQWidget*)vector->at( 1 ), i18n( "Username" ), TextInput::text, "", "username" ) );
	result->append( new TextInput( (TQWidget*)vector->at( 1 ), i18n( "Mailbox" ), TextInput::text, "INBOX", "mailbox" ) );
	result->append( new TextInput( (TQWidget*)vector->at( 1 ), i18n( "Password" ), TextInput::password, "", "password" ) );
	result->append( new CheckboxInput( (TQWidget*)vector->at( 1 ), i18n( "Save password" ), "true", "savepassword" ) );
	TQObject::connect( (TQObject*)result->last()->rightWidget(), TQT_SIGNAL( toggled( bool ) ),
			  (TQObject*)result->prev()->rightWidget(), TQT_SLOT( setEnabled( bool ) ) );
	result->last()->setValue( "false" );
	result->append( new ComboInput( (TQWidget*)vector->at( 1 ), i18n( "Authentication" ), authList, "auth=*", "auth" ) );
}

void Imap_Protocol::readEntries( TQMap< TQString, TQString >* map, TQMap< TQString, TQString > *metadata ) const
{
	if( map->contains( "ssl" ) && *map->find( "ssl" ) == "true" )
		map->insert( "encryption", "ssl" );
	if( metadata->contains( "tls" ) )
		map->insert( "encryption", TQString( "tls=%1" ).arg( *metadata->find( "tls" ) ) );
	if( metadata->contains( "auth" ) )
		map->insert( "auth", TQString( "auth=%1" ).arg( *metadata->find( "auth" ) ) );
}

void Imap_Protocol::writeEntries( TQMap< TQString, TQString >* map ) const
{
	TQString metadata;
	if( map->contains( "encryption" ) )
	{
		if( *map->find( "encryption" ) == "ssl" )
			map->insert( "ssl", "true" );
		else
		{
			map->insert( "ssl", "false" );
			metadata += *map->find( "encryption" );
		}
		map->erase( "encryption" );
	}

	if( map->contains( "auth" ) )
	{
		if( !metadata.isEmpty() )
			metadata += ",";
		metadata += *map->find( "auth" );
		map->erase( "auth" );
	}

	map->insert( "metadata", metadata );
}

