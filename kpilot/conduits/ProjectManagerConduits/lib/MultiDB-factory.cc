/* MultiDB-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MultiDB-conduit plugin.
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

#include "options.h"

#include <kinstance.h>
#include <kaboutdata.h>
#include "kpilotlink.h"
#include "MultiDB-factory.h"
#include "MultiDB-factory.moc"


KPilotSyncType::KPilotSyncType(){
	ShortName="";
	LongName="";
	id=st_ask;
	flags=0;
}
KPilotSyncType::KPilotSyncType(QString sn="", QString ln="", int i=st_ask, int flg=0) {
	ShortName=sn;
	LongName=ln;
	id=i;
	flags=flg;
}
KPilotSyncType::KPilotSyncType(KPilotSyncType*tp) {
	ShortName=tp->ShortName;
	LongName=tp->LongName;
	id=tp->id;
	flags=tp->flags;
}

KAboutData *MultiDBConduitFactory::fAbout = 0L;
SyncTypeList_t *MultiDBConduitFactory::synctypes = 0L;
QString MultiDBConduitFactory::conflictResolution = "ConflictResolution";
QString MultiDBConduitFactory::archive = "ArchiveDeletedEntries";
QString MultiDBConduitFactory::fullSyncOnPCChange = "FullSyncOnPCChange";

MultiDBConduitFactory::MultiDBConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n) {
	FUNCTIONSETUP;

	fInstance = new KInstance(n);
	synctypes=new SyncTypeList_t();
	synctypes->setAutoDelete( TRUE ); // the list owns the objects

/*	fAbout = new KAboutData(n,
		I18N_NOOP("MultiDB Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the MultiDB Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold F. Kainhofer");
	fAbout->addAuthor("Dan Pilone", I18N_NOOP("Original Author of KPilot and the VCal conduit"));
	fAbout->addAuthor("Preston Brown", I18N_NOOP("Original Author of the VCal conduit"));
	fAbout->addAuthor("Herwin-Jan Steehouwer", I18N_NOOP("Original Author of the VCal conduit"));
	fAbout->addAuthor("Adriaan de Groot", I18N_NOOP("Maintainer or KPilot"), "groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Reinhold Kainhofer", I18N_NOOP("Original author and maintainer of this conduit"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com");*/
}

void MultiDBConduitFactory::buildConduitInfo() {
	synctypes->clear();
//	synctypes->append( new KPilotSyncType(i18n("ask"), i18n("Ask for an action"), st_ask, 0) );
	synctypes->append( new KPilotSyncType(i18n("ignore"), i18n("Completely ignore database"), st_ignore, 0) );
	synctypes->append( new KPilotSyncType(i18n("backup"), i18n("Only do a backup"), st_backup, 0) );
	synctypes->append( new KPilotSyncType(i18n("pdb"), i18n("Save as .pdb database"), st_pdb, SYNC_NEEDSFILE) );
	customConduitInfo();	
}


MultiDBConduitFactory::~MultiDBConduitFactory() {
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *MultiDBConduitFactory::createObject( QObject *p,
	const char *n, const char *c, const QStringList &a) {
	FUNCTIONSETUP;

		#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Creating object of class "	<< c << endl;
		#endif

	if (!synctypes || synctypes->isEmpty()) { // synctypes not yet initialized
		synctypes=new SyncTypeList_t();
		buildConduitInfo();
	}



	if (qstrcmp(c,"ConduitConfig")==0) {
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w) {
			return createSetupWidget(w,n,a);
		} else {
				#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Couldn't cast parent to widget." << endl;
				#endif
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0) {
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d) {
			return createConduit(d,n,a);
		} else {
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink."
				<< endl;
		}
	}

	return 0L;
}

// $Log$
// Revision 1.1  2002/04/07 12:09:42  kainhofe
// Initial checkin of the conduit. The gui works mostly, but syncing crashes KPilot...
//
// Revision 1.2  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.6  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.5  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
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

