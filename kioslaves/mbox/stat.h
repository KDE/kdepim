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
#ifndef STAT_H
#define STAT_H

#include <kio/global.h>

class ReadMBox;
class UrlInfo;

class KURL;

class QString;

/**
 * This class is used to get the stats of a mbox-email or mbox-file.
 * This class only uses static members.
 */
class Stat
{
public:
	/**
	 * Empty constructor
	 */
	Stat()  {}

	/**
	 * Emtpy destructor
	 */
	~Stat() {}
	
	/**
	 * This functions gives information with a given UrlInfo.
	 * @param info The file information
	 * @return The information of the file as destribed in UrlInfo.
	 */
	static KIO::UDSEntry stat( const UrlInfo& info );
	/**
	 * This function gives information with a given ReadMBox and UrlInfo.
	 * Through this, it is possible to ask the stats of the next message,
	 * without reopening the mbox-file.
	 * @param mbox The ReadMBox instance, used to search the mbox-email in.
	 * @param info The url information.
	 * @return The requesteds information.
	 */
	static KIO::UDSEntry stat( ReadMBox& mbox, const UrlInfo& info );
	
	/**
	 * This function gets the stats of a given mbox-file in an UDSEntry.
	 * @param info The location of the mbox-file.
	 * @return A list of Atoms.
	 */
	static KIO::UDSEntry statDirectory( const UrlInfo& info );

	/**
	 * This function gets the stats of a geven mbox-message in a UDSEntry.
	 * @param info The url of the mbox-message.
	 * @return Information shipped in an UDSEntry.
	 */
	static KIO::UDSEntry statMessage( const UrlInfo& info );
private:
	static void addAtom( KIO::UDSEntry& entry, unsigned int key, const QString& value );
	static void addAtom( KIO::UDSEntry& entry, unsigned int key, const long value );
};

#endif
