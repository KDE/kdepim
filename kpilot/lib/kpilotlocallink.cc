/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006-2007 Adriaan de Groot <groot@kde.org>
** Copyright (C) 2007 Jason 'vanRijn' Kasper <vR@movingparts.net>
**
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

#include "options.h"



#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>

#include <pi-source.h>
#include <pi-socket.h>
#include <pi-dlp.h>
#include <pi-file.h>
#include <pi-buffer.h>

#include <qdir.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qthread.h>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kio/netaccess.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "kpilotlink.h"
#include "kpilotlocallink.moc"


typedef QPair<QString, struct DBInfo> DatabaseDescriptor;
typedef QValueList<DatabaseDescriptor> DatabaseDescriptorList;

class KPilotLocalLink::Private
{
public:
	DatabaseDescriptorList fDBs;
} ;

unsigned int KPilotLocalLink::findAvailableDatabases( KPilotLocalLink::Private &info, const QString &path )
{
	FUNCTIONSETUP;

	info.fDBs.clear();

	QDir d(path);
	if (!d.exists())
	{
		// Perhaps return an error?
		return 0;
	}

	// Use this to fake indexes in the list of DBInfo structs
	unsigned int counter = 0;

	QStringList dbs = d.entryList( CSL1("*.pdb"), QDir::Files | QDir::NoSymLinks | QDir::Readable );
	for ( QStringList::ConstIterator i = dbs.begin(); i != dbs.end() ; ++i)
	{
		struct DBInfo dbi;

		// Remove the trailing 4 characters
		QString dbname = (*i);
		dbname.remove(dbname.length()-4,4);

		QString dbnamecheck = (*i).left((*i).findRev(CSL1(".pdb")));
		Q_ASSERT(dbname == dbnamecheck);

		if (PilotLocalDatabase::infoFromFile( path + CSL1("/") + (*i), &dbi))
		{
			DEBUGKPILOT << fname << ": Loaded "
				<< dbname << endl;
			dbi.index = counter;
			info.fDBs.append( DatabaseDescriptor(dbname,dbi) );
			++counter;
		}
	}

	DEBUGKPILOT << fname << ": Total " << info.fDBs.count()
		<< " databases." << endl;
	return info.fDBs.count();
}


KPilotLocalLink::KPilotLocalLink( QObject *parent, const char *name ) :
	KPilotLink(parent,name),
	fReady(false),
	d( new Private )
{
	FUNCTIONSETUP;
}

KPilotLocalLink::~KPilotLocalLink()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(d);
}

/* virtual */ QString KPilotLocalLink::statusString() const
{
	return fReady ? CSL1("Ready") : CSL1("Waiting") ;
}

/* virtual */ bool KPilotLocalLink::isConnected() const
{
	return fReady;
}

/* virtual */ void KPilotLocalLink::reset( const QString &p )
{
	FUNCTIONSETUP;
	fPath = p;
	reset();
}

/* virtual */ void KPilotLocalLink::reset()
{
	FUNCTIONSETUP;
	QFileInfo info( fPath );
	fReady = !fPath.isEmpty() && info.exists() && info.isDir() ;
	if (fReady)
	{
		findAvailableDatabases(*d, fPath);
		QTimer::singleShot(500,this,SLOT(ready()));
	}
	else
	{
		WARNINGKPILOT << "The local link path <"
			<< fPath
			<< "> does not exist or is not a directory. No sync can be done."
			<< endl;
	}
}

/* virtual */ void KPilotLocalLink::close()
{
	fReady = false;
}

/* virtual */ bool KPilotLocalLink::tickle()
{
	return true;
}

/* virtual */ const KPilotCard *KPilotLocalLink::getCardInfo(int)
{
	return 0;
}

/* virtual */ void KPilotLocalLink::endSync( EndOfSyncFlags f )
{
	Q_UNUSED(f);
	fReady = false;
}

/* virtual */ int KPilotLocalLink::openConduit()
{
	FUNCTIONSETUP;
	return 0;
}


/* virtual */ int KPilotLocalLink::getNextDatabase( int index, struct DBInfo *info )
{
	FUNCTIONSETUP;

	if ( (index<0) || (index>=(int)d->fDBs.count()) )
	{
		WARNINGKPILOT << "Index out of range." << endl;
		return -1;
	}

	DatabaseDescriptor dd = d->fDBs[index];

	DEBUGKPILOT << fname << ": Getting database " << dd.first << endl;

	if (info)
	{
		*info = dd.second;
	}

	return index+1;
}

/* virtual */ int KPilotLocalLink::findDatabase(const char *name, struct DBInfo*info,
		int index, unsigned long type, unsigned long creator)
{
	FUNCTIONSETUP;

	if ( (index<0) || (index>=(int)d->fDBs.count()) )
	{
		WARNINGKPILOT << "Index out of range." << endl;
		return -1;
	}

	if (!name)
	{
		WARNINGKPILOT << "NULL name." << endl;
		return -1;
	}

	QString desiredName = Pilot::fromPilot(name);
	DEBUGKPILOT << fname << ": Looking for DB " << desiredName << endl;
	for ( DatabaseDescriptorList::ConstIterator i = d->fDBs.at(index);
		i != d->fDBs.end(); ++i)
	{
		const DatabaseDescriptor &dd = *i;
		if (dd.first == desiredName)
		{
			if ( (!type || (type == dd.second.type)) &&
				(!creator || (creator == dd.second.creator)) )
			{
				if (info)
				{
					*info = dd.second;
				}
				return index;
			}
		}

		++index;
	}

	return -1;
}

/* virtual */ void KPilotLocalLink::addSyncLogEntryImpl(QString const &s)
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname << ": " << s << endl ;
}

/* virtual */ bool KPilotLocalLink::installFile(QString const &path, bool deletefile)
{
	FUNCTIONSETUP;

	QFileInfo srcInfo(path);
	QString canonicalSrcPath = srcInfo.dir().canonicalPath() + CSL1("/") + srcInfo.fileName() ;
	QString canonicalDstPath = fPath + CSL1("/") + srcInfo.fileName();

	if (canonicalSrcPath == canonicalDstPath)
	{
		// That's a cheap copy operation
		return true;
	}

	KURL src = KURL::fromPathOrURL( canonicalSrcPath );
	KURL dst = KURL::fromPathOrURL( canonicalDstPath );

	KIO::NetAccess::file_copy(src,dst,-1,true);

	if (deletefile)
	{
		KIO::NetAccess::del(src, 0L);
	}

	return true;
}

/* virtual */ bool KPilotLocalLink::retrieveDatabase( const QString &path, struct DBInfo *db )
{
	FUNCTIONSETUP;

	QString dbname = Pilot::fromPilot(db->name) + CSL1(".pdb") ;
	QString sourcefile = fPath + CSL1("/") + dbname ;
	QString destfile = path ;

	DEBUGKPILOT << fname << ": src=" << sourcefile << endl;
	DEBUGKPILOT << fname << ": dst=" << destfile << endl;

	QFile in( sourcefile );
	if ( !in.exists() )
	{
		WARNINGKPILOT << "Source file " << sourcefile << " doesn't exist." << endl;
		return false;
	}
	if ( !in.open( IO_ReadOnly | IO_Raw ) )
	{
		WARNINGKPILOT << "Can't read source file " << sourcefile << endl;
		return false;
	}

	QFile out( destfile );
	if ( !out.open( IO_WriteOnly | IO_Truncate | IO_Raw ) )
	{
		WARNINGKPILOT << "Can't write destination file " << destfile << endl;
		return false;
	}

	const Q_ULONG BUF_SIZ = 8192 ;
	char buf[BUF_SIZ];
	Q_LONG r;

	while ( (r=in.readBlock(buf,BUF_SIZ))>0 )
	{
		out.writeBlock(buf,r);
	}
	out.flush();
	in.close();

	return out.exists();
}

KPilotLink::DBInfoList KPilotLocalLink::getDBList( int, int )
{
	FUNCTIONSETUP;
	DBInfoList l;
	for ( DatabaseDescriptorList::ConstIterator i=d->fDBs.begin();
		i != d->fDBs.end(); ++i)
	{
		l.append( (*i).second );
	}
	return l;
}


/* virtual */ PilotDatabase *KPilotLocalLink::database( const QString &name )
{
	FUNCTIONSETUP;
	return new PilotLocalDatabase( fPath, name );
}



/* slot */ void KPilotLocalLink::ready()
{
	if (fReady)
	{
		emit deviceReady(this);
	}
}

