#ifndef _KPILOT_IDMAPPERXML_H
#define _KPILOT_IDMAPPERXML_H
/*
** Copyright (C) 2006 Bertjan Broeksema <bbroeksema@bluebottle.com>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "idmapping.h"

#include <qxml.h>
#include <qdom.h>
#include <qstring.h>
#include <qptrcollection.h>

class IDMapperXml : public QXmlDefaultHandler
{
public:
	IDMapperXml( const QString &file );
	
	~IDMapperXml();
	
	/**
	 * Opens and parses the file or creates a new one if the file does not exist.
	 */
	bool open();
	
	/**
	 * Saves the current mappings to the xml-file. Note this function must be
	 * called after changes and before deleting the IDMapperXml object. Otherwise
	 * the changes won't be saved.
	 */
	void save();
	
	/**
	 * Adds a mapping to the collection of mappings.
	 */
	void addMapping( const IDMapping &mapping );
	
	/**
	 * Returns the collection of mappings.
	 */
	QValueList<IDMapping> &mappings();
	 
	/**
	 * Overloaded function to parse the xml file.
	 */
	bool startElement( const QString &namespaceURI, const QString &localName
		, const QString &qName, const QXmlAttributes &attribs );
		
	/**
	 * Overloaded function to parse the xml file.
	 */
	bool endElement( const QString &namespaceURI, const QString &localName
		, const QString &qName );

private:
	QFile fFile;
	QDomDocument doc;
	QDomElement root;
	IDMapping *fCurrentMapping;
	QValueList<IDMapping> fMappings;
};

#endif
