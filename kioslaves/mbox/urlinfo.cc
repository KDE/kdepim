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
#include "urlinfo.h"

#include <kdebug.h>
#include <kurl.h>

#include <qfileinfo.h>
#include <qstring.h>

UrlInfo::UrlInfo( const KURL& url, const UrlType type )
	: m_type( invalid ),
	m_filename( new QString ),
	m_id( new QString )
{
	calculateInfo( url, type );
}

UrlInfo::~UrlInfo()
{
	delete m_filename;
	delete m_id;
}

QString UrlInfo::mimetype() const
{
	switch( m_type )
	{
	case message:
		return "message/rfc822";
	case directory:
		return "inode/directory";
	case invalid:
	default:
		return "invalid";
	}
}

QString UrlInfo::filename() const
{
	return *m_filename;
}

QString UrlInfo::id() const
{
	return *m_id;
}

QString UrlInfo::url() const
{
	return *m_filename + "/" + *m_id;
}


void UrlInfo::calculateInfo( const KURL& url, const UrlType type )
{
	bool found = false;

	if( !found && type & UrlInfo::message )
		found = isMessage( url );
	if( !found && type & UrlInfo::directory )
		found = isDirectory( url );
	if( !found )
	{
		m_type = invalid;
		*m_filename = "";
		*m_id = "";
	}
}

bool UrlInfo::isDirectory( const KURL& url )
{
	//Check is url is in the form mbox://{filename}
	QString filename = url.path();
	QFileInfo info;

	//Remove ending /
	while( filename.length() > 1 && filename.right( 1 ) == "/" )
		filename.remove( filename.length()-2, 1 );

	//Is this a directory?
	info.setFile( filename );
	if( !info.isFile() )
		return false;

	//Setting paramaters
	*m_filename = filename;
	*m_id = QString::null;
	m_type = directory;
	kdDebug() << "urlInfo::isDirectory( " << url << " )" << endl;
	return true;
}

bool UrlInfo::isMessage( const KURL& url )
{
	QString path = url.path();
	QFileInfo info;
	int cutindex = path.findRev( '/' );
	
	//Does it contain at least one /?
	if( cutindex < 0 )
		return false;

	//Does the mbox-file exists?
	info.setFile( path.left( cutindex ) );
	if( !info.isFile() )
		return false;
	
	//Settings parameters
	kdDebug() << "urlInfo::isMessage( " << url << " )" << endl;
	m_type = message;
	*m_id = path.right( path.length() - cutindex - 1 );
	*m_filename = path.left( cutindex );
	
	return true;
}

