/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
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

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <kconfig.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "kpilotlink.h"
#include "kpilotlocallink.moc"


typedef QPair<QString, struct DBInfo> DatabaseDescriptor;
typedef QList<DatabaseDescriptor> DatabaseDescriptorList;

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

	QStringList dbs = d.entryList( QStringList() << CSL1("*.pdb"), QDir::Files | QDir::NoSymLinks | QDir::Readable );
	for ( QStringList::ConstIterator i = dbs.begin(); i != dbs.end() ; ++i)
	{
		struct DBInfo dbi;

		// Remove the trailing 4 characters
		QString dbname = (*i);
		dbname.remove(dbname.length()-4,4);

		QString dbnamecheck = (*i).left((*i).lastIndexOf(CSL1(".pdb")));
		Q_ASSERT(dbname == dbnamecheck);

		if (PilotLocalDatabase::infoFromFile( path + CSL1("/") + (*i), &dbi))
		{
			DEBUGKPILOT << "Loaded [" << dbname << ']';
			dbi.index = counter;
			info.fDBs.append( DatabaseDescriptor(dbname,dbi) );
			++counter;
		}
	}

	DEBUGKPILOT << "Total" << info.fDBs.count() << " databases.";
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
		WARNINGKPILOT << "The local link path ["
			<< fPath
			<< "] does not exist or is not a directory. No sync can be done.";
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
		WARNINGKPILOT << "Index out of range.";
		return -1;
	}

	DatabaseDescriptor dd = d->fDBs[index];

	DEBUGKPILOT << "Getting database " << dd.first;

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
		WARNINGKPILOT << "Index out of range.";
		return -1;
	}

	if (!name)
	{
		WARNINGKPILOT << "NULL name.";
		return -1;
	}

	QString desiredName = Pilot::fromPilot(name);
	DEBUGKPILOT << "Looking for DB [" << desiredName << ']';
	
	for( int i = index; i < d->fDBs.size(); ++i )
	{
		const DatabaseDescriptor &dd = d->fDBs.at( i );
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

/* virtual */ void KPilotLocalLink::addSyncLogEntryImpl(QString const &)
{
	FUNCTIONSETUP;
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

	KUrl src( canonicalSrcPath );
	KUrl dst( canonicalDstPath );

	KIO::Job *my_job = KIO::file_copy(src,dst, -1, KIO::Overwrite);
	KIO::NetAccess::synchronousRun(my_job, 0L);

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

	DEBUGKPILOT << "src=[" << sourcefile << ']'
		<< "dst=[" << destfile << ']';

	QFile in( sourcefile );
	if ( !in.exists() )
	{
		WARNINGKPILOT << "Source file [" << sourcefile << "] does not exist.";
		return false;
	}
	if ( !in.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
	{
		WARNINGKPILOT << "Cannot read source file [" << sourcefile << ']';
		return false;
	}

	QFile out( destfile );
	if ( !out.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Unbuffered ) )
	{
		WARNINGKPILOT << "Cannot write destination file" << destfile;
		return false;
	}

	const qint64 BUF_SIZ = 8192 ;
	char buf[BUF_SIZ];
	qint64 r;

	while ( (r=in.read(buf,BUF_SIZ))>0 )
	{
		out.write(buf,r);
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

