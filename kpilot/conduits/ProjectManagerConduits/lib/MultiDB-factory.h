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


typedef enum synctps {
	st_ask, st_ignore, st_backup, st_pdb, st_vcal, st_ldap, st_sql, st_csv
};

class DBSyncInfo{
public:
	DBSyncInfo(QListViewItem*item) {dbname=item->text(0); syncaction=item->text(1).toInt(); filename=item->text(2);};
	DBSyncInfo(){dbname=QString::null; syncaction=0; filename=QString::null;}
	DBSyncInfo(char*db, int act, char*fn) {dbname=QString(db); syncaction=act; filename=QString(fn);};
	DBSyncInfo(QString db, int act, QString fn) {dbname=db; syncaction=act; filename=fn;};
	bool set(QString name, int act, QString fn) {dbname=name; syncaction=act; filename=fn; return true;}
	~DBSyncInfo() {};
public:
	QString dbname;
	int syncaction;
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
} ;


// $Log$
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.8  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.7  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.5  2002/03/23 21:46:42  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.4  2002/03/23 18:21:14  reinhold
// Cleaned up the structure. Works with QTimer instead of loops.
//
// Revision 1.3  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.2  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//


#endif
