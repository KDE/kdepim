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

#include <QXmlDefaultHandler>
#include <QDateTime>
#include <QMap>


class KPILOT_EXPORT IDMappingXmlSource : public QXmlDefaultHandler
{
public:
	IDMappingXmlSource( const QString &userName, const QString &conduit );
	
	~IDMappingXmlSource();
	
	QDateTime lastSyncedDate() const { return fLastSyncedDateTime; }
	
	void setLastSyncedDate( const QDateTime &dateTime );
	
	QString lastSyncedPC() const { return fLastSyncedPC; }
	
	void setLastSyncedPC( const QString &pc );
	
	QMap<QString, QString>* mappings() { return &fMappings; }
	
	const QMap<QString, QString>* constMappings() const { return &fMappings; }

	void setHHCategory( const QString &hhRecordId, const QString &category );
	
	QString hhCategory( const QString &hhRecordId ) const;

	void setPCCategories( const QString &pcRecordId, const QStringList &categories );
	
	QStringList pcCategories( const QString &pcRecordId ) const;
	
	QStringList* archivedRecords() { return &fArchivedRecords; }
	
	const QStringList* constArchivedRecords() const { return &fArchivedRecords; }
	
	void loadMapping();
	
	bool saveMapping();
	
	bool rollback();
	 
protected:
	/**
	 * Overloaded function to parse the xml file.
	 */
	bool startElement( const QString &namespaceURI, const QString &localName
		, const QString &qName, const QXmlAttributes &attribs );

private:
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

#endif /*IDMAPPERXMLSOURCE_H */
