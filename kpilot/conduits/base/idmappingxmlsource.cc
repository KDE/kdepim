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

class IDMappingXmlSourcePrivate : public QSharedData
{
public:
	IDMappingXmlSourcePrivate()
	{
	}
	
	IDMappingXmlSourcePrivate( const IDMappingXmlSourcePrivate& other ) : QSharedData( other )
	{
		fPath = other.fPath;
		fMappings = other.fMappings;
		fHHCategory = other.fHHCategory;
		fPCCategories = other.fPCCategories;
		fArchivedRecords = other.fArchivedRecords;
		fLastSyncedDateTime = other.fLastSyncedDateTime;
		fLastSyncedPC = other.fLastSyncedPC;
		fCurrentHHId = other.fCurrentHHId;
		fCurrentPCId = other.fCurrentPCId;
	}

	QString fPath;
	
	/**
	 * Mappings between handheld ids (keys) and pc ids (values).
	 */
	QMap<QString, QString> fMappings;
	QMap<QString, QString> fHHCategory;       // Key: HHRecordId, Value: Category name
	QMap<QString, QStringList> fPCCategories; // Key: PCRecordId, Value: Categories
	QStringList fArchivedRecords;
	QDateTime fLastSyncedDateTime;
	QString fLastSyncedPC;
	
	QString fCurrentHHId;
	QString fCurrentPCId;
};

IDMappingXmlSource::IDMappingXmlSource() : d( new IDMappingXmlSourcePrivate )
{
	FUNCTIONSETUP;
}

IDMappingXmlSource::IDMappingXmlSource( const QString &userName
	, const QString &conduit ) : d( new IDMappingXmlSourcePrivate )
{
	FUNCTIONSETUP;
	
	// $HOME/.kde/share/apps/kpilot/conduits/<Username>/mapping/<Conduit>-mapping.xml.
	// saveLocation will create dirs if necessary.
	QString pathName = KGlobal::dirs()->saveLocation( "data",
		CSL1("kpilot/conduits/"));
	
	d->fPath = pathName + CSL1( "/" ) + userName + CSL1( "/mapping/" ) + conduit 
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

IDMappingXmlSource& IDMappingXmlSource::operator=( const IDMappingXmlSource& other )
{
	FUNCTIONSETUP;

	if( this != &other )
	{
		d = other.d;
	}
	
	return *this;
}

IDMappingXmlSource::~IDMappingXmlSource()
{
	FUNCTIONSETUP;
}

QStringList* IDMappingXmlSource::archivedRecords()
{
	return &d->fArchivedRecords;
}

const QStringList* IDMappingXmlSource::constArchivedRecords() const
{
	return &d->fArchivedRecords;
}

const QMap<QString, QString>* IDMappingXmlSource::constMappings() const
{
	return &d->fMappings;
}

QString IDMappingXmlSource::hhCategory( const QString &hhRecordId ) const
{
	FUNCTIONSETUP;
	
	return d->fHHCategory.value( hhRecordId );
}

QDateTime IDMappingXmlSource::lastSyncedDate() const
{
	return d->fLastSyncedDateTime;
}

QString IDMappingXmlSource::lastSyncedPC() const
{
	return d->fLastSyncedPC;
}

bool IDMappingXmlSource::loadMapping()
{
	FUNCTIONSETUP;
	
	// Reset local data.
	d->fMappings = QMap<QString, QString>();
	d->fLastSyncedDateTime = QDateTime();
	d->fLastSyncedPC.clear();
	bool success = false;

	QFile file( d->fPath );
	
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
		success = reader.parse( source );
		DEBUGKPILOT << "was able to parse file: " << success;
		file.close();
		
		delete source;
	}
	return success;
}

QMap<QString, QString>* IDMappingXmlSource::mappings()
{
	return &d->fMappings;
}

QStringList IDMappingXmlSource::pcCategories( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return d->fPCCategories.value( pcRecordId );
}

bool IDMappingXmlSource::saveMapping()
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << "Saving " << d->fMappings.count() << " mappings...";
	
	QDomDocument doc;
	QDomElement root = doc.createElement( CSL1("mappings") );
	QDomNode node = doc.createProcessingInstruction(CSL1("xml")
		,CSL1("version=\"1.0\" encoding=\"UTF-8\""));
	
	doc.appendChild( node );
	doc.appendChild( root );
	
	QDomElement dateElement = doc.createElement( CSL1( "lastsync" ) );
	dateElement.setAttribute( CSL1( "value" )
		, d->fLastSyncedDateTime.toString( Qt::ISODate ) );
	
	QDomElement pcElement = doc.createElement( CSL1( "pc" ) );
	pcElement.setAttribute( CSL1( "value" ), d->fLastSyncedPC );
	
	root.appendChild( dateElement );
	root.appendChild( pcElement );
	
	QMap<QString, QString>::const_iterator it;
	for( it = d->fMappings.begin(); it != d->fMappings.end(); ++it )
	{
		QDomElement mappingElement = doc.createElement( CSL1("mapping") );
		mappingElement.setAttribute( CSL1("hh"), it.key() );
		mappingElement.setAttribute( CSL1("pc"), it.value() );
		
		if( !d->fHHCategory.value( it.key() ).isEmpty() )
		{
			QDomElement hhCatElement = doc.createElement( CSL1("hhcategory") );
			hhCatElement.setAttribute( CSL1("value"), d->fHHCategory.value( it.key() ) );
			
			mappingElement.appendChild( hhCatElement );
		}
		
		foreach( const QString& pcCat, d->fPCCategories.value( it.value() ) )
		{
			QDomElement pcCatElement = doc.createElement( CSL1("pccategory") );
			pcCatElement.setAttribute( CSL1("value"), pcCat );
			
			mappingElement.appendChild( pcCatElement );
		}
		
		if( d->fArchivedRecords.contains( it.value() ) )
		{
			mappingElement.setAttribute( CSL1( "archived" ), CSL1( "yes" ) );
		}
		
		root.appendChild( mappingElement );
	}
	
	QFile file( d->fPath );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate) )
	{
		QTextStream out( &file );
		doc.save( out, 4 );
		file.close();
		return true;
	}
	
	return false;
}

void IDMappingXmlSource::setHHCategory( const QString &hhRecordId, const QString &category )
{
	FUNCTIONSETUP;
	
	d->fHHCategory.insert( hhRecordId, category );
}

void IDMappingXmlSource::setLastSyncedDate( const QDateTime &dateTime )
{
	FUNCTIONSETUP;
	
	d->fLastSyncedDateTime = dateTime;
}

void IDMappingXmlSource::setLastSyncedPC( const QString &pc )
{
	FUNCTIONSETUP;
	
	d->fLastSyncedPC = pc;
}


void IDMappingXmlSource::setPCCategories( const QString &pcRecordId, const QStringList &categories )
{
	FUNCTIONSETUP;
	
	d->fPCCategories.insert( pcRecordId, categories );
}

bool IDMappingXmlSource::rollback()
{
	FUNCTIONSETUP;
	
	QFile backup( d->fPath + "-backup" );
	
	if( !backup.exists() )
	{
		// No backup, reset values.
		d->fMappings = QMap<QString, QString>();
		d->fLastSyncedDateTime = QDateTime();
		d->fLastSyncedPC.clear();
		return true;
	}
	
	// Rename the incorrect mapping.
	QFile fail( d->fPath );
	bool renamed = fail.rename( d->fPath + ".fail" );
	
	if( !renamed )
	{
		// Could not rename the file, rollback failed.
		DEBUGKPILOT <<"Rename failed";
		return false;
	}
	
	// Try to copy the backup back to the original location.
	bool copied = backup.copy( d->fPath );
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
	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	
	if( qName == CSL1( "mapping" ) )
	{
		d->fCurrentHHId = attribs.value( CSL1( "hh" ) );
		d->fCurrentPCId = attribs.value( CSL1( "pc" ) );
		
		QString hh( attribs.value( CSL1( "hh" ) ) );
		QString pc( attribs.value( CSL1( "pc" ) ) );
		
		if( attribs.value( CSL1( "archived" ) ) == CSL1( "yes" ) )
		{
			d->fArchivedRecords.append( pc );
		}
		
		d->fMappings.insert( hh, pc );
	}
	else if( qName == CSL1( "hhcategory" ) )
	{
		QString category = attribs.value( CSL1("value") );
		d->fHHCategory.insert( d->fCurrentHHId, category );
	}
	else if( qName == CSL1( "pccategory" ) )
	{
		QString category = attribs.value( CSL1("value") );
		
		QStringList cats = d->fPCCategories.value( d->fCurrentPCId );
		cats << category;
		
		d->fPCCategories.insert( d->fCurrentPCId, cats );
	}
	else if( qName == CSL1( "lastsync" ) )
	{
		d->fLastSyncedDateTime = QDateTime::fromString( attribs.value( CSL1("value") )
			, Qt::ISODate );
	}
	else if( qName == CSL1("pc") )
	{
		d->fLastSyncedPC = attribs.value( CSL1("value") );
	}

	return true;
}

bool IDMappingXmlSource::remove()
{
	FUNCTIONSETUP;
	
	// $HOME/.kde/share/apps/kpilot/conduits/<Username>/mapping/<Conduit>-mapping.xml.
	DEBUGKPILOT << "removing file: " << d->fPath;
	QFile file( d->fPath );
	bool success;
	
	if( !file.exists() )
	{
		DEBUGKPILOT << "File does not exist. Can't remove.";
	}
	else
	{
		success = file.remove();
		DEBUGKPILOT << (success ? "Successfully removed " : "Failed to remove ")
			    << "file.";
	}
	return success;
}
