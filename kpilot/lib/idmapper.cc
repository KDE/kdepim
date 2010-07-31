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

#include <tqsqldatabase.h>
#include <tqfile.h>

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
	
	TQString dbPath = KGlobal::dirs()->
		saveLocation("data", CSL1("kpilot/") );
	TQString dbFile = dbPath + CSL1("mapping.xml");
	
	if( !openDatasource( dbFile ) )
	{
		DEBUGKPILOT << fname << "Could not open or create xml file." << endl;
	}
}

IDMapper::IDMapper( const TQString &file)
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

bool IDMapper::openDatasource( const TQString &file )
{
	FUNCTIONSETUP;
	
	fP->fXmlSource = new IDMapperXml( file );
	return fP->fXmlSource->open();
}

void IDMapper::registerPCObjectId( const TQString &conduit, const TQString &uid )
{
	FUNCTIONSETUP;
	
	IDMapping mapping = IDMapping( conduit );
	mapping.setUid( uid );
	
	fP->fXmlSource->addMapping( mapping );
	fP->fXmlSource->save();
}

void IDMapper::registerHHObjectId( const TQString &conduit, recordid_t pid )
{
	FUNCTIONSETUP;
	
	IDMapping mapping = IDMapping( conduit );
	mapping.setPid( pid );
	
	fP->fXmlSource->addMapping( mapping );
	fP->fXmlSource->save();
}

TQValueList<TQString> IDMapper::getPCObjectIds( const TQString &conduit )
{
	FUNCTIONSETUP;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	TQValueList<TQString> uids;
	
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

TQValueList<recordid_t> IDMapper::getHHObjectIds( const TQString &conduit )
{
	FUNCTIONSETUP;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	TQValueList<recordid_t> pids;
	
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

bool IDMapper::hasPCId( const TQString &conduit, recordid_t pid )
{
	FUNCTIONSETUP;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	
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

bool IDMapper::hasHHId( const TQString &conduit, const TQString &uid )
{
	FUNCTIONSETUP;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	
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

void IDMapper::setHHObjectId( const TQString &conduit, const TQString &uid
		, recordid_t pid )
{
	FUNCTIONSETUP;
	
	bool modified = false;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	
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

void IDMapper::setPCObjectId( const TQString &conduit, recordid_t pid
	, const TQString &uid )
{
	FUNCTIONSETUP;
	
	bool modified = false;
	
	TQValueList<IDMapping> &mappings = fP->fXmlSource->mappings();
	TQValueList<IDMapping>::iterator it;
	
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
