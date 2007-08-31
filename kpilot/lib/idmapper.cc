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

#include "idmapper.h"
#include "idmapperxml.h"
#include "options.h"

#include <qsqldatabase.h>
#include <qfile.h>

#include <kglobal.h>
#include <kstandarddirs.h>

class IDMapperPrivate
{
public:
	IDMapperPrivate()
	{
		fXmlSource = 0L;
	}
	
	~IDMapperPrivate()
	{
		FUNCTIONSETUP;
		
		KPILOT_DELETE(fXmlSource);
	}
	
	IDMapperXml *fXmlSource;
};

IDMapper::IDMapper()
{
	FUNCTIONSETUP;
	
	fP = new IDMapperPrivate();
	
	QString dbPath = KGlobal::dirs()->
		saveLocation("data", CSL1("kpilot/") );
	QString dbFile = dbPath + CSL1("mapping.xml");
	
	if( !openDatasource( dbFile ) )
	{
		DEBUGKPILOT << fname << "Could not open or create xml file." << endl;
	}
}

IDMapper::IDMapper( const QString &file)
{
	FUNCTIONSETUP;
	
	fP = new IDMapperPrivate();
	
	if( !openDatasource( file ) )
	{
		DEBUGKPILOT << fname << "Could not open or create xml file." << endl;
	}
}

IDMapper::~IDMapper()
{
	KPILOT_DELETE(fP);
}

bool IDMapper::openDatasource( const QString &file )
{
	FUNCTIONSETUP;
	
	fP->fXmlSource = new IDMapperXml( file );
	return fP->fXmlSource->open();
}

void IDMapper::registerPCObjectId( const QString &conduit, const QString &uid )
{
	FUNCTIONSETUP;
	
	IDMapping mapping = IDMapping( conduit );
	mapping.setUid( uid );
	
	fP->fXmlSource->addMapping( mapping );
	fP->fXmlSource->save();
}

void IDMapper::registerHHObjectId( const QString &conduit, recordid_t pid )
{
	FUNCTIONSETUP;
	
	IDMapping mapping = IDMapping( conduit );
	mapping.setPid( pid );
	
	fP->fXmlSource->addMapping( mapping );
	fP->fXmlSource->save();
}

QValueList<QString> IDMapper::getPCObjectIds( const QString &conduit )
{
	FUNCTIONSETUP;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	QValueList<QString> uids;
	
	DEBUGKPILOT << fname << ": total " << mappings.count() << endl;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = (*it);
		
		DEBUGKPILOT << fname << ": mapping.conduit() = " << mapping.conduit() << endl;
		DEBUGKPILOT << fname << ": conduit = " << conduit << endl;
		
		if( (mapping.conduit() == conduit) && !mapping.uid().isNull() )
		{
			DEBUGKPILOT << fname << ": mapping.conduit() == conduit" << endl;
			uids.append( mapping.uid() );
		}
	}
	
	return uids;
}

QValueList<recordid_t> IDMapper::getHHObjectIds( const QString &conduit )
{
	FUNCTIONSETUP;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	QValueList<recordid_t> pids;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = *it;
		DEBUGKPILOT << fname << ": mapping.conduit() = " << mapping.conduit() << endl;
		DEBUGKPILOT << fname << ": " << mapping.pid() << endl;
		if( mapping.conduit() == conduit && mapping.pid() != 0 )
		{
			DEBUGKPILOT << fname << ": mapping.conduit() == conduit" << endl;
			pids.append( mapping.pid() );
		}
	}
	
	return pids;
}

bool IDMapper::hasPCId( const QString &conduit, recordid_t pid )
{
	FUNCTIONSETUP;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = *it;
		if( mapping.conduit() == conduit && mapping.pid() == pid )
		{
			return !mapping.uid().isNull();
		}
	}
	
	return false;
}

bool IDMapper::hasHHId( const QString &conduit, const QString &uid )
{
	FUNCTIONSETUP;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = *it;
		if( mapping.conduit() == conduit && mapping.uid() == uid )
		{
			return mapping.pid() != 0;
		}
	}
	
	return false;
}

void IDMapper::setHHObjectId( const QString &conduit, const QString &uid
		, recordid_t pid )
{
	FUNCTIONSETUP;
	
	bool modified = false;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = *it;
		if( mapping.conduit() == conduit && mapping.uid() == uid )
		{
			mapping.setPid( pid );
			fP->fXmlSource->save();
			modified = true;
		}
	}
}

void IDMapper::setPCObjectId( const QString &conduit, recordid_t pid
	, const QString &uid )
{
	FUNCTIONSETUP;
	
	bool modified = false;
	
	QValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	QValueList<IDMapping>::iterator it;
	
	for ( it = mappings.begin(); it != mappings.end(); ++it )
	{
		IDMapping &mapping = *it;
		if( mapping.conduit() == conduit && mapping.pid() == pid )
		{
			mapping.setUid( uid );
			fP->fXmlSource->save();
			modified = true;
		}
	}
}
