#ifndef IDMAPPINGXMLSOURCE_H
#define IDMAPPINGXMLSOURCE_H
/* idmappingxmlsource.h			KPilot
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

#include "kpilot_export.h"

#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QSharedDataPointer>
#include <QtXml/QXmlDefaultHandler>

class IDMappingXmlSourcePrivate;

class KPILOT_EXPORT IDMappingXmlSource : public QXmlDefaultHandler
{
public:
	IDMappingXmlSource();

	IDMappingXmlSource( const QString &userName, const QString &conduit );
	
	IDMappingXmlSource& operator=( const IDMappingXmlSource& other );
	
	~IDMappingXmlSource();
	
	QStringList* archivedRecords();
	
	const QStringList* constArchivedRecords() const;
	
	const QMap<QString, QString>* constMappings() const;
	
	QString hhCategory( const QString &hhRecordId ) const;
	
	QDateTime lastSyncedDate() const;
	
	QString lastSyncedPC() const;
	
	bool loadMapping();
	
	QMap<QString, QString>* mappings();
	
	QStringList pcCategories( const QString &pcRecordId ) const;
	
	bool saveMapping();
	
	void setHHCategory( const QString &hhRecordId, const QString &category );
	
	void setLastSyncedDate( const QDateTime &dateTime );
	
	void setLastSyncedPC( const QString &pc );

	void setPCCategories( const QString &pcRecordId, const QStringList &categories );
	
	bool rollback();

	bool remove();
	 
protected:
	/**
	 * Overloaded function to parse the xml file.
	 */
	bool startElement( const QString &namespaceURI, const QString &localName
		, const QString &qName, const QXmlAttributes &attribs );

private:
	QSharedDataPointer<IDMappingXmlSourcePrivate> d;
};

#endif /*IDMAPPERXMLSOURCE_H */
