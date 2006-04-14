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
#include <config.h>

#include "readmbox.h"

#include "mbox.h"
#include "urlinfo.h"

#include <kdebug.h>
#include <kio/global.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qtextstream.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <utime.h>

ReadMBox::ReadMBox( const UrlInfo* info, MBoxProtocol* parent, bool onlynew, bool savetime )
	: MBoxFile( info, parent ),
	m_file( 0 ),
	m_stream( 0 ),
	m_current_line( new QString( QString::null ) ),
	m_current_id( new QString( QString::null ) ),
	m_atend( true ),
	m_prev_time( 0 ),
	m_only_new( onlynew ),
	m_savetime( savetime ),
	m_status( false ),
	m_prev_status( false ),
	m_header( true )
	
{
	if( m_info->type() == UrlInfo::invalid )
		m_mbox->emitError( KIO::ERR_DOES_NOT_EXIST, info->url() );
		
	if( !open( savetime ) )
		m_mbox->emitError( KIO::ERR_CANNOT_OPEN_FOR_READING, info->url() );

	if( m_info->type() == UrlInfo::message )
		if( !searchMessage( m_info->id() ) )
			m_mbox->emitError( KIO::ERR_DOES_NOT_EXIST, info->url() );
}

ReadMBox::~ReadMBox()
{
	delete m_current_line;
	close();
}

QString ReadMBox::currentLine() const
{
	return *m_current_line;
}

QString ReadMBox::currentID() const
{
	return *m_current_id;
}

bool ReadMBox::nextLine()
{
	if( !m_stream )
		return true;
		
	*m_current_line = m_stream->readLine();
	m_atend = m_current_line->isNull();
	if( m_atend ) // Cursor was at EOF
	{
		*m_current_id = QString::null;
		m_prev_status = m_status;
		return true;
	}

	//New message
	if( m_current_line->left( 5 ) == "From " )
	{
		*m_current_id = *m_current_line;
		m_prev_status = m_status;
		m_status = true;
		m_header = true;
		return true;
	} else if( m_only_new )
	{
		if( m_header && m_current_line->left( 7 ) == "Status:" &&
		    ! m_current_line->contains( "U" ) && ! m_current_line->contains( "N" ) )
		{
			m_status = false;
		}
	}

	if( m_current_line->stripWhiteSpace().isEmpty() )
		m_header = false;

	return false;
}

bool ReadMBox::searchMessage( const QString& id )
{
	if( !m_stream )
		return false;
		
	while( !m_atend && *m_current_id != id )
		nextLine();

	return *m_current_id == id;
}

unsigned int ReadMBox::skipMessage()
{
	unsigned int result = m_current_line->length();

	if( !m_stream )
		return 0;

	while( !nextLine() )
		result += m_current_line->length();

	return result;
}

void ReadMBox::rewind()
{
	if( !m_stream )
		return; //Rewinding not possible
	
	m_stream->device()->reset();
	m_atend = m_stream->atEnd();
}

bool ReadMBox::atEnd() const
{
	if( !m_stream )
		return true;
	
	return m_atend || ( m_info->type() == UrlInfo::message && *m_current_id != m_info->id() );
}

bool ReadMBox::inListing() const
{
	return !m_only_new || m_prev_status;
}

bool ReadMBox::open( bool savetime )
{
	if( savetime )
	{
		QFileInfo info( m_info->filename() );
	
		m_prev_time = new utimbuf;
		m_prev_time->actime = info.lastRead().toTime_t();
		m_prev_time->modtime = info.lastModified().toTime_t();
	}
	
	if( m_file )
		return false; //File already open

	m_file = new QFile( m_info->filename() );
	if( !m_file->open( IO_ReadOnly ) )
	{
		delete m_file;
		m_file = 0;
		return false;
	}
	m_stream = new QTextStream( m_file );
	skipMessage();

	return true;
}

void ReadMBox::close()
{
	if( !m_stream )
		return;

	delete m_stream; m_stream = 0;
	m_file->close();
	delete m_file; m_file = 0;

	if( m_prev_time )
		utime( QFile::encodeName( m_info->filename() ), m_prev_time );
}

