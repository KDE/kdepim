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


#include "kmail_proto.h"

#include "account_input.h"
#include "kio.h"
#include "password.h"
#include "protocols.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <qmap.h>
#include <qobject.h>
#include <qptrvector.h>
#include <qstringlist.h>

const char* KMail_Protocol::kmailGroupName = "Account %1";
const char* KMail_Protocol::kmailKeyType = "Type";
const char* KMail_Protocol::kmailKeyName = "Name";
const char* KMail_Protocol::kmailKeyId = "Id";
const char* KMail_Protocol::kmailKeyMBox = "Location";
const char* KMail_Protocol::kmailKeyQMail = "Location";
const int KMail_Protocol::kmailFirstGroup = 1;

class KMailDrop;

KMail_Protocol::KMail_Protocol()
{
}

KMail_Protocol::~KMail_Protocol()
{
}

const Protocol* KMail_Protocol::getProtocol( KConfigGroup* config ) const
{
	KConfig kmailconfig( "kmailrc", true, false );
	int id;
	QString type = getTypeAndConfig( config->readEntry( "kmailname" ), kmailconfig, id );

	if( type == "imap" )
		return Protocols::getProto( "imap" );
        if( type == "cachedimap" )
                return Protocols::getProto( "imap" );
	if( type == "pop" )
		return Protocols::getProto( "pop3" );
	if( type == "local" )
		return Protocols::getProto( "mbox" );
	if( type == "maildir" )
		return Protocols::getProto( "qmail" );

	//Type not recognized, or does not exist in KOrn
	kdWarning() << "KMail configuration not found" << endl;
	return 0;
}

KMailDrop* KMail_Protocol::createMaildrop( KConfigGroup *config ) const
{
	int id;
	KConfig kmailconfig( "kmailrc", true, false );
	QString type = getTypeAndConfig( config->readEntry( "kmailname" ), kmailconfig, id );

	if( type == "imap" || type == "cachedimap" || type == "pop" || type == "local" || type == "maildir" )
		return new KKioDrop();
	
	kdWarning() << "KMail configuration not found" << endl;
	return 0;
}

QMap< QString, QString > * KMail_Protocol::createConfig( KConfigGroup* config, const QString& ) const
{
	QMap< QString, QString > *result = new QMap<QString, QString>;
	int id;
	KConfig kmailconfig( "kmailrc", true, false );
	//First: find the account in the configuration and get the type and id out of it.
	QString type = getTypeAndConfig( config->readEntry( "kmailname" ), kmailconfig, id );
	QString metadata;

	if( type == "imap" || type == "cachedimap" )
	{
		//Construct metadata
		if( kmailconfig.hasKey( "auth" ) )
			metadata += QString( "auth=%1," ).arg( kmailconfig.readEntry( "auth" ) );
		if( !kmailconfig.hasKey( "use-tls" ) )
			metadata += "tls=auto";
		else
		{
			if( kmailconfig.readBoolEntry( "use-tls", false ) )
				metadata += "tls=on";
			else
				metadata += "tls=off";
		}
		//Add the fields into the mapping.
		result->insert( "name", config->readEntry( "name", "" ) );
		result->insert( "server", kmailconfig.readEntry( "host", "localhost" ) );
		result->insert( "port", kmailconfig.readEntry( "port", "143" ) );
		result->insert( "ssl", kmailconfig.readEntry( "use-ssl", "false" ) );
		result->insert( "metadata", metadata );
		result->insert( "username", kmailconfig.readEntry( "login", "" ) );
		result->insert( "mailbox", "INBOX" ); //Didn't find a good way to get this out of the configuration yet.
		result->insert( "password", readPassword( kmailconfig.readBoolEntry( "store-passwd", false ), kmailconfig, id ) );
		result->insert( "savepassword", kmailconfig.readEntry( "store-passwd", "false" ) );
	}
	if( type == "pop" )
	{
		//Constructing metadata
		if( kmailconfig.hasKey( "auth" ) )
			metadata += QString( "auth=%1," ).arg( kmailconfig.readEntry( "auth" ) );
		if( !kmailconfig.hasKey( "use-tls" ) )
			metadata += "tls=auto";
		else
		{
			if( kmailconfig.readBoolEntry( "use-tls", false ) )
				metadata += "tls=on";
			else
				metadata += "tls=off";
		}
		result->insert( "name", config->readEntry( "name", "" ) );
		result->insert( "server", kmailconfig.readEntry( "host", "localhost" ) );
		result->insert( "port", kmailconfig.readEntry( "port", "110" ) );
		result->insert( "ssl", kmailconfig.readEntry( "use-ssl", "false" ) );
		result->insert( "metadata", metadata );
		result->insert( "username", kmailconfig.readEntry( "login", "" ) );
		result->insert( "mailbox", "" );
		result->insert( "password", readPassword( kmailconfig.readBoolEntry( "store-passwd", false ), kmailconfig, id ) );
		result->insert( "savepassword", kmailconfig.readEntry( "store-password", "false" ) );
	}
	if( type == "local" ) //mbox
	{
		result->insert( "name", config->readEntry( "name", "" ) );
		result->insert( "server", "" );
		result->insert( "port", "0" );
		result->insert( "ssl", "false" );
		result->insert( "metadata", "" );
		result->insert( "username", "" );
		result->insert( "mailbox", kmailconfig.readPathEntry( kmailKeyMBox, "" ) );
		result->insert( "password", "" );
		result->insert( "savepassword", "false" );
	}
	if( type == "maildir" )
	{
		result->insert( "name", config->readEntry( "name", "" ) );
		result->insert( "server", "" );
		result->insert( "port", "0" );
		result->insert( "ssl", "false" );
		result->insert( "metadata", "" );
		result->insert( "username", "" );
		result->insert( "mailbox", kmailconfig.readPathEntry( kmailKeyQMail, "" ) );
		result->insert( "password", "" );
		result->insert( "savepassword", "false" );
	}

	return result;
}

void KMail_Protocol::configFillGroupBoxes( QStringList* lijst ) const
{
	lijst->append( "KMail" );
}

void KMail_Protocol::configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput >* result ) const
{
	QMap< QString, QString > accountList;
	QString type;
	QString name;
	int nummer = kmailFirstGroup - 1;
	
	KConfig kmailconfig( "kmailrc", true, false );
	while( kmailconfig.hasGroup( QString( kmailGroupName ).arg( ++nummer ) ) )
	{
		kmailconfig.setGroup( QString( kmailGroupName ).arg( nummer ) );
		type = kmailconfig.readEntry( kmailKeyType, QString::null );
		name = kmailconfig.readEntry( kmailKeyName, QString::null );
		if( type == "imap" || type == "cachedimap" || type == "pop" || type == "local" )
		{
			accountList.insert( name, name );
		}
	}

	result->append( new ComboInput( (QWidget*)vector->at( 0 ), i18n( "KMail name" ), accountList, *accountList.begin(), "kmailname" ) );
}

void KMail_Protocol::readEntries( QMap< QString, QString >* ) const
{
	//The configuartion is read out on the right way
}

void KMail_Protocol::writeEntries( QMap< QString, QString >* ) const
{
	//The configuartion is read out on the right way
}

QString KMail_Protocol::readPassword( bool store, const KConfigBase& config, int id ) const
{
	if( !store )
		return "";

	return KOrnPassword::readKMailPassword( id, config );
}

QString KMail_Protocol::getTypeAndConfig( const QString& kmailname, KConfig &kmailconfig, int &id ) const
{
	int nummer = kmailFirstGroup - 1;
	bool found = false;

	id = -1;
	
	while( kmailconfig.hasGroup( QString( kmailGroupName ).arg( ++nummer ) ) )
	{
		kmailconfig.setGroup( QString( kmailGroupName ).arg( nummer ) );
		if( kmailconfig.readEntry( kmailKeyName, QString::null ) == kmailname )
		{
			id = kmailconfig.readNumEntry( kmailKeyId, 0 );	
			found = true;
			break;
		}
	}
	if( !found )
	{
		nummer = -1;
		return QString::null;
	}

	//The correct group is found, and kmailconfig.setGroup() is already called for the right group.
	return kmailconfig.readEntry( kmailKeyType, QString::null );
}

