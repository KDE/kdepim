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
#include "mbox.h"

#include "readmbox.h"
#include "stat.h"
#include "urlinfo.h"

#include <qstring.h>
#include <qcstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kurl.h>
#include <kio/global.h>

#include <stdlib.h>

#include "kdepimmacros.h"

#include "mbox.h"

extern "C" { KDE_EXPORT int kdemain(int argc, char* argv[]); }

int kdemain( int argc, char * argv[] )
{
	KLocale::setMainCatalogue("kdelibs");
	KInstance instance("kio_mbox");
	(void) KGlobal::locale();

	if (argc != 4) {
		fprintf(stderr, "Usage: kio_mbox protocol "
	        	        "domain-socket1 domain-socket2\n");
		exit(-1);
	}

	MBoxProtocol slave(argv[2], argv[3]);
	slave.dispatchLoop();
	
	return 0;
}

MBoxProtocol::MBoxProtocol( const QCString& arg1, const QCString& arg2 )
	: KIO::SlaveBase( "mbox2", arg1, arg2 ),
	m_errorState( true )
{
	
}

MBoxProtocol::~MBoxProtocol()
{
}

void MBoxProtocol::get( const KURL& url )
{
	m_errorState = false;
	
	UrlInfo info( url, UrlInfo::message );
	QString line;
	QByteArray ba_line;

	if( info.type() == UrlInfo::invalid && !m_errorState )
	{
		error( KIO::ERR_DOES_NOT_EXIST, info.url() );
		return;
	}
	
	ReadMBox mbox( &info, this );

	while( !mbox.atEnd() && !m_errorState)
	{
		line = mbox.currentLine();
		line += '\n';
		ba_line = QCString( line.utf8() );
		ba_line.truncate( ba_line.size() - 1 ); //Removing training '\0'
		data( ba_line );
		mbox.nextLine();
	};
	
	if( !m_errorState )
	{
		data( QByteArray() );
		finished();
	}
}

void MBoxProtocol::listDir( const KURL& url )
{
	m_errorState = false;
	
	KIO::UDSEntry entry;
	UrlInfo info( url, UrlInfo::directory );
	ReadMBox mbox( &info, this, hasMetaData( "onlynew" ), hasMetaData( "savetime" ) );

	if( m_errorState )
		return;
	
	if( info.type() != UrlInfo::directory )
	{
		error( KIO::ERR_DOES_NOT_EXIST, info.url() );
		return;
	}
	
	while( !mbox.atEnd() && !m_errorState )
	{
		entry = Stat::stat( mbox, info );
		if( mbox.inListing() )
			listEntry( entry, false );
	}

	listEntry( KIO::UDSEntry(), true );
	finished();
}

void MBoxProtocol::stat( const KURL& url )
{
	UrlInfo info( url );
	if( info.type() == UrlInfo::invalid )
	{
		error( KIO::ERR_DOES_NOT_EXIST, url.path() );
		return;
	} else
	{
		statEntry( Stat::stat( info ) );
	}
	finished();
}

void MBoxProtocol::mimetype( const KURL& url )
{	
	m_errorState = false;
	
	UrlInfo info( url );

	if( m_errorState )
		return;
	
	if( info.type() == UrlInfo::invalid )
		error( KIO::ERR_DOES_NOT_EXIST, i18n( "Invalid URL" ) );
	else
		mimeType( info.mimetype() );
	finished();
}

void MBoxProtocol::emitError( int errno, const QString& arg )
{
	m_errorState = true;
	error( errno, arg );
}

