/* idmappingxmlsource.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "idmappingxmlsource.h"

#include "options.h"

#include <KGlobal>
#include <KStandardDirs>

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

IDMappingXmlSource::IDMappingXmlSource( const QString &userName
	, const QString &conduit )
{
	FUNCTIONSETUP;
	
	// $HOME/.kde/share/apps/kpilot/conduits/<Username>/mapping/<Conduit>-mapping.xml.
	// saveLocation will create dirs if necessary.
	QString pathName = KGlobal::dirs()->saveLocation( "data",
		CSL1("kpilot/conduits/"));
	
	fPath = pathName + CSL1( "/" ) + userName + CSL1( "/mapping/" ) + conduit 
		+ CSL1( "-mapping.xml" );
	
	// Create directories if necessary.
	QDir dir( pathName );
	if( !dir.exists( userName ) )
	{
		dir.mkpath( userName + CSL1( "/mapping/" ) );
	}
	else
	{
		dir.cd( userName );
		if( dir.exists( CSL1( "mapping" ) ) )
		{
			dir.cd( CSL1( "mapping" ) );
			if( dir.exists( conduit + CSL1( "-mapping.xml" ) ) )
			{
				// Make a backup of the existing file.
				QFile file( dir.absolutePath() + CSL1( "/" ) + conduit 
					+ CSL1( "-mapping.xml" ) );
				file.copy( file.fileName() + CSL1( "-backup" ) );
			}
		}
		else
		{
			dir.mkdir( CSL1( "mapping" ) );
		}
	}
}

IDMappingXmlSource::~IDMappingXmlSource()
{
	FUNCTIONSETUP;
}

void IDMappingXmlSource::setLastSyncedDate( const QDateTime &dateTime )
{
	FUNCTIONSETUP;
	
	fLastSyncedDateTime = dateTime;
}

void IDMappingXmlSource::setLastSyncedPC( const QString &pc )
{
	FUNCTIONSETUP;
	
	fLastSyncedPC = pc;
}

void IDMappingXmlSource::setHHCategory( const QString &hhRecordId, const QString &category )
{
	FUNCTIONSETUP;
	
	fHHCategory.insert( hhRecordId, category );
}

QString IDMappingXmlSource::hhCategory( const QString &hhRecordId ) const
{
	FUNCTIONSETUP;
	
	return fHHCategory.value( hhRecordId );
}

void IDMappingXmlSource::setPCCategories( const QString &pcRecordId, const QStringList &categories )
{
	FUNCTIONSETUP;
	
	fPCCategories.insert( pcRecordId, categories );
}

QStringList IDMappingXmlSource::pcCategories( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return fPCCategories.value( pcRecordId );
}

void IDMappingXmlSource::loadMapping()
{
	FUNCTIONSETUP;
	
	// Reset local data.
	fMappings = QMap<QString, QString>();
	fLastSyncedDateTime = QDateTime();
	fLastSyncedPC = QString();
	
	QFile file( fPath );
	
	if( !file.exists() )
	{
		DEBUGKPILOT << "File does not exist, empty map.";
	}
	else
	{
		DEBUGKPILOT << "Parsing file" << file.fileName();
		QXmlSimpleReader reader;
		reader.setContentHandler( this );
		
		// Make sure that the file is closed after parsing.
		const QXmlInputSource *source = new QXmlInputSource( &file );
		reader.parse( source );
		file.close();
		
		delete source;
	}
}

bool IDMappingXmlSource::saveMapping()
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << "Saving " << fMappings.count() << " mappings...";
	
	QDomDocument doc;
	QDomElement root = doc.createElement( CSL1("mappings") );
	QDomNode node = doc.createProcessingInstruction(CSL1("xml")
		,CSL1("version=\"1.0\" encoding=\"UTF-8\""));
	
	doc.appendChild( node );
	doc.appendChild( root );
	
	QDomElement dateElement = doc.createElement( CSL1( "lastsync" ) );
	dateElement.setAttribute( CSL1( "value" )
		, fLastSyncedDateTime.toString( Qt::ISODate ) );
	
	QDomElement pcElement = doc.createElement( CSL1( "pc" ) );
	pcElement.setAttribute( CSL1( "value" ), fLastSyncedPC );
	
	root.appendChild( dateElement );
	root.appendChild( pcElement );
	
	QMap<QString, QString>::const_iterator it;
	for( it = fMappings.begin(); it != fMappings.end(); ++it )
	{
		QDomElement mappingElement = doc.createElement( CSL1("mapping") );
		mappingElement.setAttribute( CSL1("hh"), it.key() );
		mappingElement.setAttribute( CSL1("pc"), it.value() );
		
		if( !fHHCategory.value( it.key() ).isEmpty() )
		{
			QDomElement hhCatElement = doc.createElement( CSL1("hhcategory") );
			hhCatElement.setAttribute( CSL1("value"), fHHCategory.value( it.key() ) );
			
			mappingElement.appendChild( hhCatElement );
		}
		
		if( fArchivedRecords.contains( it.value() ) )
		{
			mappingElement.setAttribute( CSL1( "archived" ), CSL1( "yes" ) );
		}
		
		root.appendChild( mappingElement );
	}
	
	QFile file( fPath );
	if( file.open( QIODevice::ReadWrite ) )
	{
		QTextStream out( &file );
		doc.save( out, 4 );
		file.close();
		return true;
	}
	
	return false;
}

bool IDMappingXmlSource::rollback()
{
	FUNCTIONSETUP;
	
	QFile backup( fPath + "-backup" );
	
	if( !backup.exists() )
	{
		// No backup, reset values.
		fMappings = QMap<QString, QString>();
		fLastSyncedDateTime = QDateTime();
		fLastSyncedPC = QString();
		return true;
	}
	
	// Rename the incorrect mapping.
	QFile fail( fPath );
	bool renamed = fail.rename( fPath + ".fail" );
	
	if( !renamed )
	{
		// Could not rename the file, rollback failed.
		DEBUGKPILOT <<"Rename failed";
		return false;
	}
	
	// Try to copy the backup back to the original location.
	bool copied = backup.copy( fPath );
	if( copied )
	{
		// Read the backup file.
		loadMapping();
		return true;
	}
	
	DEBUGKPILOT <<"Copy failed";
	// There went something wrong during copy. Rollback failed.
	return false;
}

bool IDMappingXmlSource::startElement( const QString &namespaceURI
	, const QString &localName, const QString &qName
	, const QXmlAttributes &attribs )
{
	FUNCTIONSETUP;
	
	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	
	if( qName == CSL1( "mapping" ) )
	{
		fCurrentHHId = attribs.value( CSL1( "hh" ) );
		fCurrentPCId = attribs.value( CSL1( "pc" ) );
		
		QString hh( attribs.value( CSL1( "hh" ) ) );
		QString pc( attribs.value( CSL1( "pc" ) ) );
		
		if( attribs.value( CSL1( "archived" ) ) == CSL1( "yes" ) )
		{
			fArchivedRecords.append( pc );
		}
		
		fMappings.insert( hh, pc );
	}
	else if( qName == CSL1( "hhcategory" ) )
	{
		QString category = attribs.value( CSL1("value") );
		fHHCategory.insert( fCurrentHHId, category );
	}
	else if( qName == CSL1( "lastsync" ) )
	{
		fLastSyncedDateTime = QDateTime::fromString( attribs.value( CSL1("value") )
			, Qt::ISODate );
	}
	else if( qName == CSL1("pc") )
	{
		fLastSyncedPC = attribs.value( CSL1("value") );
	}

	return true;
}
