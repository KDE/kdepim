/* knotes-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the knotes-conduit plugin.
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

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qmap.h>
#include <qtimer.h>

#if KDE_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include <dcopclient.h>

#include <time.h> // Needed by pilot-link include

#include <pi-memo.h>

#include "pilotMemo.h"
#include "pilotSerialDatabase.h"

#include "setup_base.h"
#include "KNotesIface_stub.h"

#include "knotes-factory.moc"


extern "C"
{

void *init_libknotesconduit()
{
	return new KNotesConduitFactory;
}

} ;


KAboutData *KNotesConduitFactory::fAbout = 0L;
KNotesConduitFactory::KNotesConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("knotesconduit");
	fAbout = new KAboutData("knotesconduit",
		I18N_NOOP("KNotes Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the KNotes Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Primary Author"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addCredit("David Bishop",
		I18N_NOOP("UI"));
}

KNotesConduitFactory::~KNotesConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *KNotesConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Creating object of class "
		<< c
		<< endl;
#endif

	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new KNotesWidgetSetup(w,n,a);
		}
		else 
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to widget."
				<< endl;
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new KNotesAction(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}

	return 0L;
}

KNotesWidgetSetup::KNotesWidgetSetup(QWidget *w, const char *n, 
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	fConfigWidget = new KNotesWidget(widget());
	setTabWidget(fConfigWidget->tabWidget);
	addAboutPage(false,KNotesConduitFactory::about());

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
}

KNotesWidgetSetup::~KNotesWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void KNotesWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"KNotes-conduit");

	fConfig->writeEntry("DeleteNoteForMemo",fConfigWidget->fDeleteNoteForMemo->isChecked());
}

/* virtual */ void KNotesWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"KNotes-conduit");

	fConfigWidget->fDeleteNoteForMemo->setChecked(
		fConfig->readBoolEntry("DeleteNoteForMemo",false));
}

class KNotesAction::KNotesActionPrivate
{
public:
	KNotesActionPrivate() :
		fDCOP(0L),
		fKNotes(0L),
		fTimer(0L),
		fDatabase(0L),
		fCounter(0)
	{ } ;

	// These are  the notes that we got from KNotes
	QMap <int,QString> fNotes;
	// This iterates through that list; it's in here because
	// we use slots to process one item at a time and need
	// to keep track of where we are between slot calls.
	QMap <int,QString>::ConstIterator fIndex;
	// The DCOP client for this application, and the KNotes stub.
	DCOPClient *fDCOP;
	KNotesIface_stub *fKNotes;
	// The timer for invoking process() to do some more work.
	QTimer *fTimer;
	// The database we're working with (MemoDB)
	PilotSerialDatabase *fDatabase;
	// Some counter that needs to be preserved between calls to
	// process(). Typically used to note hom much work is done.
	int fCounter;
} ;

KNotesAction::KNotesAction(KPilotDeviceLink *o,
	const char *n, const QStringList &a) :
	ConduitAction(o,n,a),
	fP(new KNotesActionPrivate)
{
	FUNCTIONSETUP;


	fP->fDCOP = KApplication::kApplication()->dcopClient();

	if (!fP->fDCOP)
	{
		kdWarning() << k_funcinfo
			<< ": Can't get DCOP client."
			<< endl;
	}
}

/* virtual */ KNotesAction::~KNotesAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP->fTimer);
	KPILOT_DELETE(fP->fKNotes);
	KPILOT_DELETE(fP->fDatabase);
	KPILOT_DELETE(fP);
}

/* virtual */ void KNotesAction::exec()
{
	FUNCTIONSETUP;

	if (!fP->fDCOP) return;
	if (!knotesRunning()) return;

	fP->fKNotes = new KNotesIface_stub("knotes","KNotesIface");

	fP->fNotes = fP->fKNotes->notes();
	fP->fDatabase = new PilotSerialDatabase(pilotSocket(),"MemoDB",this,"MemoDB");

	if (isTest())
	{
		listNotes();
	}
	else
	{
		fP->fTimer = new QTimer(this);
		fStatus = Init;
		fP->fCounter = 0;
		fP->fIndex = fP->fNotes.begin();

		connect(fP->fTimer,SIGNAL(timeout()),SLOT(process()));

		fP->fTimer->start(0,false);
	}
}

void KNotesAction::listNotes()
{
	FUNCTIONSETUP;

	QMap<int,QString>::ConstIterator i = fP->fNotes.begin();
	while (i != fP->fNotes.end())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": "
			<< i.key()
			<< "->"
			<< i.data()
			<< (fP->fKNotes->isNew("kpilot",i.key()) ?
				" (new)" : "" )
			<< endl;
#endif
		i++;
	}

	emit syncDone(this);
}

/* slot */ void KNotesAction::process()
{
	switch(fStatus)
	{
	case Init: getAppInfo(); break;
	case NewNotesToPilot : addNewNoteToPilot(); break;
	case Cleanup : cleanupMemos(); break;
	default : 
		fP->fTimer->stop();
		emit syncDone(this);
	}
}

void KNotesAction::getAppInfo()
{
	FUNCTIONSETUP;

#define APP_INFO_BLOCK_SIZE  (8192)

	unsigned char buffer[APP_INFO_BLOCK_SIZE];
	int appInfoSize = fP->fDatabase->readAppBlock(buffer,APP_INFO_BLOCK_SIZE);
	struct MemoAppInfo memoInfo;

	if (appInfoSize<0)
	{
		fStatus=Error;
		return;
	}

	unpack_MemoAppInfo(&memoInfo,buffer,appInfoSize);
	PilotDatabase::listAppInfo(&memoInfo.category);

	fStatus=NewNotesToPilot;
}

void KNotesAction::addNewNoteToPilot()
{
	FUNCTIONSETUP;

	if (fP->fIndex == fP->fNotes.end())
	{
		if (fP->fCounter)
		{
			addSyncLogEntry(i18n("Added one new memo.",
				"Added %n new memos.",
				fP->fCounter));
		}

		fP->fCounter = 0;
		fStatus = Cleanup;
		return;
	}

	if (fP->fKNotes->isNew("kpilot",fP->fIndex.key()))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": The note #"
			<< fP->fIndex.key()
			<< " with name "
			<< fP->fIndex.data()
			<< " is new to the Pilot."
			<< endl;
#endif

		QString text = fP->fIndex.data() + "\n" ;
		text.append(fP->fKNotes->text(fP->fIndex.key()));

		const char *c = text.latin1();
		PilotMemo *a = new PilotMemo((void *)(const_cast<char *>(c)));
		PilotRecord *r = a->pack();

		fP->fDatabase->writeRecord(r);

		delete r;
		delete a;
		
		fP->fCounter++;
	}

	++(fP->fIndex);
}

void KNotesAction::cleanupMemos()
{
	FUNCTIONSETUP;

	// Tell KNotes we're up-to-date
	fP->fKNotes->sync("kpilot");

	fStatus=Done;
}

bool KNotesAction::knotesRunning() const
{
	FUNCTIONSETUP;

	QCStringList apps = fP->fDCOP->registeredApplications();
	return apps.contains("knotes");
}

/* virtual */ QString KNotesAction::statusString() const
{
	switch(fStatus)
	{
	case Init : return QString("Init");
	case NewNotesToPilot :
		return QString("NewNotesToPilot key=%1")
			.arg(fP->fIndex.key());
	case Done :
		return QString("Done");
	default :
		return QString("Unknown (%1)").arg(fStatus);
	}
}


// $Log$
// Revision 1.4  2001/10/10 22:39:49  adridg
// Some UI/Credits/About page patches
//
// Revision 1.3  2001/10/10 21:42:09  adridg
// Actually do part of a sync now
//
// Revision 1.2  2001/10/10 13:40:07  cschumac
// Compile fixes.
//
// Revision 1.1  2001/10/08 22:27:42  adridg
// New ui, moved to lib-based conduit
//
//

