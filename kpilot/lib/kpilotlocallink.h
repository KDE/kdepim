#ifndef _KPILOT_KPILOTLOCALLINK_H
#define _KPILOT_KPILOTLOCALLINK_H
/*
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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

#include "kpilotlink.h"

/** @file
* Definition of the local link class; implemented in kpilotlink.cc .
*/


/**
* Implementation of the device link for file-system backed (ie. local, fake)
* devices. Uses a directory specified in the reset() call to serve databases.
*/
class KDE_EXPORT KPilotLocalLink : public KPilotLink
{
Q_OBJECT
public:
	KPilotLocalLink( QObject *parent=0L, const char *name=0L );
	virtual ~KPilotLocalLink();

	virtual QString statusString() const;
	virtual bool isConnected() const;
	virtual void reset( const QString & );
	virtual void close();
	virtual void reset();
	virtual bool tickle();
	virtual const KPilotCard *getCardInfo(int card);
	virtual void endSync( EndOfSyncFlags f );
	virtual int openConduit();
	virtual int getNextDatabase(int index,struct DBInfo *);
	virtual int findDatabase(const char *name, struct DBInfo*,
		int index=0, unsigned long type=0, unsigned long creator=0);
	virtual bool retrieveDatabase(const QString &path, struct DBInfo *db);
	virtual DBInfoList getDBList(int cardno=0, int flags=dlpDBListRAM);
	virtual PilotDatabase *database( const QString &name );

public slots:
	void ready();

protected:
	virtual bool installFile(const QString &, const bool deleteFile);
	virtual void addSyncLogEntryImpl( const QString &s );
	virtual int pilotSocket() const
	{
		return -1;
	}

protected:
	bool fReady;
	QString fPath;

	class Private;
	Private *d;

	/**
	* Pre-process the directory @p path to find out which databases
	* live there.
	*
	* @return Number of database in @p path.
	*/
	unsigned int findAvailableDatabases( Private &, const QString &path );
} ;


#endif

