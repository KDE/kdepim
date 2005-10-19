/*
 * This is a simple kioslave to handle mbox-files.
 * Copyright (C) 2004 Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "stat.h"

#include "readmbox.h"
#include "urlinfo.h"

#include <kdebug.h>
#include <kio/global.h>

#include <sys/stat.h>

KIO::UDSEntry Stat::stat( const UrlInfo& info )
{
	if( info.type() == UrlInfo::message )
		return Stat::statMessage( info );
	else if( info.type() == UrlInfo::directory )
		return Stat::statDirectory( info );
	else
		return KIO::UDSEntry();
}

KIO::UDSEntry Stat::stat( ReadMBox& mbox, const UrlInfo& info )
{
	kdDebug() << "Stat::stat()" << endl;
	KIO::UDSEntry entry;
	QString url;
	
	if( info.type() == UrlInfo::invalid )
		return entry;
	else if( info.type() == UrlInfo::message )
		mbox.searchMessage( info.id() );

	entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
        entry.insert( KIO::UDS_MIME_TYPE, QString( "message/rfc822" ) );
		        
	url = QString( "mbox:%1/%2" ).arg( info.filename(), mbox.currentID() );
	entry.insert( KIO::UDS_URL, url );
	if( mbox.currentID().isEmpty() )
		entry.insert( KIO::UDS_NAME, QString( "" ) );
	else
	        entry.insert( KIO::UDS_NAME, mbox.currentID() );
	

	entry.insert( KIO::UDS_SIZE, mbox.skipMessage() );

	return entry;
}

KIO::UDSEntry Stat::statDirectory( const UrlInfo& info )
{
	kdDebug() << "statDirectory()" << endl;
	KIO::UDSEntry entry;

	//Specific things for a directory
	entry.insert( KIO::UDS_FILE_TYPE, S_IFDIR );
	entry.insert( KIO::UDS_NAME, info.filename() );

	return entry;
}

KIO::UDSEntry Stat::statMessage( const UrlInfo& info )
{
	kdDebug() << "statMessage( " << info.url()  << " )" << endl;
	KIO::UDSEntry entry;
	QString url = QString( "mbox:%1" ).arg( info.url() );

	//Specific things for a message
	entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
	entry.insert( KIO::UDS_MIME_TYPE, QString( "message/rfc822" ) );
	
	entry.insert( KIO::UDS_URL, url );
	url = url.right( url.length() - url.findRev( "/" ) - 1 );
	entry.insert( KIO::UDS_NAME, url );

	return entry;
}

