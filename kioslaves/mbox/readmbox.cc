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
#include "readmbox.h"

#include "mbox.h"
#include "urlinfo.h"

#include <kdebug.h>
#include <kio/global.h>

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

ReadMBox::ReadMBox( const UrlInfo* info, MBoxProtocol* parent )
	: MBoxFile( info, parent ),
	m_file( 0 ),
	m_stream( 0 ),
	m_current_line( new QString( QString::null ) ),
	m_current_id( new QString( QString::null ) )
{
	if( m_info->type() == UrlInfo::invalid )
		m_mbox->emitError( KIO::ERR_DOES_NOT_EXIST, info->url() );
		
	if( !open() )
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
		
	if( m_stream->atEnd() )
	{
		*m_current_line = QString::null;
		*m_current_id = QString::null;
		return true;
	}

	*m_current_line = m_stream->readLine();

	//New message
	if( m_current_line->left( 5 ) == "From " )
	{
		*m_current_id = *m_current_line;
		return true;
	}

	return false;
}

bool ReadMBox::searchMessage( const QString& id )
{
	if( !m_stream )
		return false;
		
	while( !m_stream->atEnd() && *m_current_id != id )
		nextLine();

	return *m_current_id == id;
}

unsigned int ReadMBox::skipMessage()
{
	unsigned int result = m_current_line->length();

	if( !m_stream )
		return 0;

	while( !m_stream->atEnd() && !nextLine() )
		result += m_current_line->length();

	return result;
}

void ReadMBox::rewind()
{
	if( m_stream )
		m_stream->device()->reset();
}

bool ReadMBox::atEnd() const
{
	if( !m_stream )
		return true;
	
	return m_stream->atEnd() || ( m_info->type() == UrlInfo::message && *m_current_id != m_info->id() );
}

bool ReadMBox::open()
{
	if( m_file )
		return false; //File already open

	m_file = new QFile( m_info->filename() );
	if( !m_file->open( IO_ReadOnly ) )
	{
		delete m_file;
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
}

