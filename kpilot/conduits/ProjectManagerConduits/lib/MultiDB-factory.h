#ifndef _KPILOT_MultiDB_FACTORY_H
#define _KPILOT_MultiDB_FACTORY_H
/* MultiDB-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MultiDB-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <qlistview.h>
#include <klibloader.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <klistview.h>
#include "options.h"
#include "kpilotlink.h"
#include "plugin.h"
//#include "MultiDB-factory.h"

#ifdef KDE2
#include <qlist.h>
#define SyncTypeList_t QList<KPilotSyncType>
#define SyncTypeIterator_t QListIterator<KPilotSyncType>
#else
#include <qptrlist.h>
#define SyncTypeList_t QPtrList<KPilotSyncType>
#define SyncTypeIterator_t QPtrListIterator<KPilotSyncType>
#endif

#define RES_PALMOVERRIDES 0
#define RES_PCOVERRIDES 1
#define RES_ASK 2

#define SYNC_FIRST 0
#define SYNC_FAST 1
#define SYNC_FULL 2
#define SYNC_MAX SYNC_FULL


typedef enum synctps {
	st_ask, st_ignore, st_backup, st_pdb, st_vcal, st_ldap, st_sql, st_csv
};

class DBSyncInfo{
public:
	DBSyncInfo(QListViewItem*item) {dbname=item->text(0); syncaction=item->text(1).toInt(); filename=item->text(2);};
	DBSyncInfo(){dbname=QString::null; syncaction=0; filename=QString::null; synctype=SYNC_FAST;}
	DBSyncInfo(char*db, int act, char*fn, int st=SYNC_FAST) {dbname=QString(db); syncaction=act; filename=QString(fn); synctype=st;};
	DBSyncInfo(QString db, int act, QString fn, int st=SYNC_FAST) {dbname=db; syncaction=act; filename=fn; synctype=st;};
	bool set(QString name, int act, QString fn, int st=SYNC_FAST) {dbname=name; syncaction=act; filename=fn; synctype=st; return true;}
	~DBSyncInfo() {};
public:
	QString dbname;
	int syncaction;
	int synctype;
	QString filename;
};



#define SYNC_NEEDSFILE 1
#define SYNC_MAXFLAG SYNC_NEEDSFILE

class KPilotSyncType {
public:
	KPilotSyncType();
	KPilotSyncType(QString sn="", QString ln="", int i=st_ask, int flg=0);
	KPilotSyncType(KPilotSyncType*tp);
	virtual ~KPilotSyncType() {};
	virtual bool getFlag(int flg) const { return (flags & flg); }
	virtual int setFlag(int flg, bool on) { if (on) {flags|=flg;} else {flags&=(!flg);} return flags;}

public:
	QString ShortName;
	QString LongName;
	int id;
	int flags;
};

class MultiDBConduitFactory : public KLibFactory {
Q_OBJECT

public:
	MultiDBConduitFactory(QObject * = 0L,const char * = 0L) ;
	virtual ~MultiDBConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	virtual QObject*createSetupWidget(QWidget*, const char*, const QStringList &)=0;
	virtual QObject*createConduit(KPilotDeviceLink *, const char *n=0L, const QStringList &l=QStringList())=0;
	virtual void buildConduitInfo();
	virtual void customConduitInfo() {};	

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() ) ;
private:
	KInstance *fInstance;
public:
	static KAboutData *fAbout;
	static SyncTypeList_t *synctypes;
	static QString conflictResolution;
	static QString archive;
	static QString fullSyncOnPCChange;
} ;


#endif
