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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "mbox.h"

#include <libkdepim/kdepimmacros.h>
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
		data( line.utf8() );
		mbox.nextLine();
	};
	
	if( !m_errorState )
	{
		data( QCString() );
		finished();
	}
}

void MBoxProtocol::listDir( const KURL& url )
{
	m_errorState = false;
	
	KIO::UDSEntryList list;
	UrlInfo info( url, UrlInfo::directory );
	ReadMBox mbox( &info, this );

	if( m_errorState )
		return;
	
	if( info.type() != UrlInfo::directory )
	{
		error( KIO::ERR_DOES_NOT_EXIST, info.url() );
		return;
	}
	
	int counter = 0;
	while( !mbox.atEnd() && ++counter < 100 && !m_errorState )
		listEntry( Stat::stat( mbox, info ), false );
		
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
		error( KIO::ERR_DOES_NOT_EXIST, i18n( "Invalid url" ) );
	else
		mimeType( info.mimetype() );
	finished();
}

void MBoxProtocol::emitError( int errno, const QString& arg )
{
	m_errorState = true;
	error( errno, arg );
}

