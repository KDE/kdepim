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
#ifndef URLINFO_H
#define URLINFO_H
class KURL;

class QString;

class UrlInfo
{
public:
	/**
	 * This enum is used to determe the url type.
	 */
	enum UrlType { invalid = 0, message = 1, directory = 2 };

	/**
	 * Constructor
	 *
	 * @param url The url: this url is used to split the location data off.
	 * @param type The possible types of the url
	 */
	UrlInfo( const KURL &url, const UrlType type = (UrlType)( message | directory ) );

	/**
	 * Destructor
	 */
	~UrlInfo();

	/**
	 * Returns the type of the url
	 * @return the type of the url
	 */
	UrlType type() const { return m_type; }

	/**
	 * @return the mimetype of the url
	 */
	QString mimetype() const;
	
	/**
	 * @return The location of the mbox-file
	 */
	QString filename() const; 
	/**
	 * @return The id given in the url.
	 */
	QString id() const;

	/**
	 * @return the while url as QString
	 */
	QString url() const;
private:
	void calculateInfo( const KURL& url, const UrlType type );

	bool isDirectory( const KURL& url );
	bool isMessage( const KURL& url );

private:
	UrlType m_type;
	QString *m_filename;
	QString *m_id;
};

#endif
