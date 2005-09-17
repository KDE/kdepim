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
#include "mboxfile.h"

#include <assert.h>

MBoxFile::MBoxFile( const UrlInfo* info, MBoxProtocol* parent )
	: m_info( info ),
	m_mbox( parent )
{
	assert( m_info );
}

MBoxFile::~MBoxFile()
{
}

bool MBoxFile::lock()
{
	//Not implemented
	return true;
}

void MBoxFile::unlock()
{
	//Not implemented
}


