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

#include "idmapperxml.h"

#include "options.h"

IDMapperXml::IDMapperXml( const QString &file ) : fFile(file)
	, fCurrentMapping( 0l )
{
}

IDMapperXml::~IDMapperXml()
{
	FUNCTIONSETUP;
}

bool IDMapperXml::open()
{
	FUNCTIONSETUP;
	
	root = doc.createElement( CSL1("mappings") );
	QDomNode node = doc.createProcessingInstruction(CSL1("xml")
		,CSL1("version=\"1.0\" encoding=\"UTF-8\""));
	
	doc.appendChild( node );
	doc.appendChild( root );
	
	if( !fFile.exists() )
	{
		DEBUGKPILOT << fname << ": Creating " << fFile.name() << endl;
		
		if( fFile.open( IO_ReadWrite ) )
		{
			QTextStream out( &fFile );
			doc.save( out, 4 );
			fFile.close();
			return true;
		}
		else
		{
			DEBUGKPILOT << fname << ": Could not create " << fFile.name() << endl;
			return false;
		}
	}
	else
	{
		DEBUGKPILOT << fname << ": Parsing file " << fFile.name() << endl;
		QXmlSimpleReader reader;
		reader.setContentHandler( this );
		
		// Make sure that the file is closed after parsing.
		bool result = reader.parse( fFile );
		fFile.close();
		
		return result;
	}
}

void IDMapperXml::save()
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << fname << ": Saving " << fMappings.count()
		<< " mappings..." << endl;
	DEBUGKPILOT << fname << ": ";
	
	QValueList<IDMapping>::const_iterator it;
	for ( it = fMappings.begin(); it != fMappings.end(); ++it )
	{
		DEBUGKPILOT << ".";
		
		IDMapping mapping = (*it);
		
		DEBUGKPILOT << fname << ": " << mapping.conduit();
		
		QDomElement mappingElement = doc.createElement( CSL1("mapping") );
		mappingElement.setAttribute( CSL1("conduit"), mapping.conduit() );
		
		if( !mapping.uid().isNull() )
		{
			QDomElement uidElement = doc.createElement( CSL1("uid") );
			uidElement.setAttribute( CSL1("value"), mapping.uid() );
			mappingElement.appendChild( uidElement );
		}
		
		if( mapping.pid() != 0 )
		{
			QDomElement uidElement = doc.createElement( CSL1("pid") );
			uidElement.setAttribute( CSL1("value"), mapping.pid() );
			mappingElement.appendChild( uidElement );
		}
		
		if( !mapping.lastSyncTime().isNull() )
		{
			QDomElement uidElement = doc.createElement( CSL1("pid") );
			uidElement.setAttribute( CSL1("value"), QString::number( mapping.pid() ) );
			mappingElement.appendChild( uidElement );
		}
		
		root.appendChild( mappingElement );
	}
	
	if( fFile.open( IO_ReadWrite ) )
	{
		QTextStream out( &fFile );
		doc.save( out, 4 );
		fFile.close();
		
		DEBUGKPILOT << endl << fname << ": finished saving." << endl;
	}
}

void IDMapperXml::addMapping( const IDMapping &mapping )
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << fname << ": " << mapping.conduit() << endl;
	
	fMappings.append( mapping );
	
	DEBUGKPILOT << fname << ": " << fMappings.first().conduit() << endl;
}

QValueList<IDMapping>& IDMapperXml::mappings()
{
	return fMappings;
}

bool IDMapperXml::startElement( const QString &namespaceURI
	, const QString &localName, const QString &qName
	, const QXmlAttributes &attribs )
{
	FUNCTIONSETUP;
	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	
	if( qName == CSL1("mapping") )
	{
		QString conduit( attribs.value( CSL1("conduit") ) );
		
		fCurrentMapping = new IDMapping( conduit );
	}
	else if( qName == CSL1("uid") )
	{
		fCurrentMapping->setUid( attribs.value( CSL1("value") ) );
	}
	else if( qName == CSL1("pid") )
	{
		fCurrentMapping->setPid( attribs.value( CSL1("value") ).toULong() );
	}
	else if( qName == CSL1("lastsync") )
	{
		// NOTE: this isn't very robuust!
		// Parses only dates in the form: dd-mm-yyyy hh:mm:ss
		QString date = attribs.value( CSL1("value") );
		int day = date.left(2).toInt();
		int month = date.mid(3,2).toInt();
		int year = date.mid(6, 4).toInt();
		
		int hour = date.mid(11,2).toInt();
		int minute = date.mid(14,2).toInt();
		int second = date.mid(17,2).toInt();
		
		QDate tmpDate = QDate( year, month, day );
		QTime tmpTime = QTime( hour, minute, second );
		
		fCurrentMapping->setLastSyncTime( QDateTime( tmpDate, tmpTime ) );
	}

	return true;
}

bool IDMapperXml::endElement( const QString &namespaceURI
	, const QString &localName, const QString &qName )
{
	FUNCTIONSETUP;

	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	Q_UNUSED(qName);
	
	if( qName == CSL1("mapping") )
	{
		addMapping( *fCurrentMapping );
		delete fCurrentMapping;
		fCurrentMapping = 0l;
	}
	
	return true;
}
