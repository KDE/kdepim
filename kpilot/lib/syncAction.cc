/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2001 by Waldo Bastian (code in questionYesNo)
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
static const char *syncAction_id =
	"$Id$";

#include "options.h"

#include <time.h>

#include <pi-socket.h>
#include <pi-dlp.h>

#include <qtimer.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtl.h>
#include <qstyle.h>

#include <kdialogbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "syncAction.moc"
#include "kpilotlibSettings.h"

SyncAction::SyncAction(KPilotDeviceLink  *p,
	const char *name) :
	QObject(p, name),
	fHandle(p),
	fParent(0L)
{
	FUNCTIONSETUP;
	(void) syncAction_id;
}

SyncAction::SyncAction(KPilotDeviceLink *p,
	QWidget * visibleparent,
	const char *name) :
	QObject(p, name),
	fHandle(p),
	fParent(visibleparent)
{
	FUNCTIONSETUP;
}

SyncAction::~SyncAction()
{
}

/* virtual */ QString SyncAction::statusString() const
{
	FUNCTIONSETUP;
	QString s = CSL1("status=");

	s.append(QString::number(status()));
	return s;
}

/* slot */ void SyncAction::execConduit()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Running conduit " << name() << endl;
#endif

	bool r = this->exec();

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Exec returned " << r << endl;
#endif

	if (!r)
	{
		emit logError(i18n("The conduit %1 could not be executed.")
			.arg(QString::fromLatin1(name())));
		delayDone();
	}
}

/* slot */ void SyncAction::delayedDoneSlot()
{
	emit syncDone(this);
}

bool SyncAction::delayDone()
{
	QTimer::singleShot(0,this,SLOT(delayedDoneSlot()));
	return true;
}

/* static */ QString SyncAction::syncModeName(SyncMode e)
{
	switch(e)
	{
	case eTest : return i18n("Test Sync");
	case eFastSync : return i18n("FastSync");
	case eHotSync : return i18n("HotSync");
	case eFullSync : return i18n("Full Synchronization");
	case eCopyPCToHH : return i18n("Copy PC to Handheld");
	case eCopyHHToPC : return i18n("Copy Handheld to PC");
	case eBackup : return i18n("Backup");
	case eRestore : return i18n("Restore From Backup");
	case eDefaultSync: break; /* FALLTHRU */
	}
	return i18n("Unknown sync mode");
}

void SyncAction::startTickle(unsigned timeout)
{
	FUNCTIONSETUP;
	connect(fHandle,SIGNAL(timeout()),this,SIGNAL(timeout()));
	fHandle->startTickle(timeout);
}

void SyncAction::stopTickle()
{
	FUNCTIONSETUP;
	disconnect(fHandle,SIGNAL(timeout()),this,SIGNAL(timeout()));
	fHandle->stopTickle();
}


int SyncAction::questionYesNo(const QString & text,
	const QString & caption,
	const QString & key,
	unsigned timeout,
	const QString & yes,
	const QString &no )
{
	FUNCTIONSETUP;

	bool checkboxReturn = false;
	int r;
	KMessageBox::ButtonCode result;
	if (!key.isEmpty())
	{
		if (!KMessageBox::shouldBeShownYesNo(key,result))
		{
			return result;
		}
	}

#if !KDE_IS_VERSION(3,3,0)
	return KMessageBox::Cancel;
#else
	KDialogBase *dialog =
		new KDialogBase(caption.isNull()? i18n("Question") : caption,
		KDialogBase::Yes | KDialogBase::No,
		KDialogBase::Yes, KDialogBase::No,
		fParent, "questionYesNo", true, true,
		yes.isEmpty() ? KStdGuiItem::yes() : yes,
		no.isEmpty() ? KStdGuiItem::no() : no);

	if (timeout > 0)
	{
		QObject::connect(fHandle, SIGNAL(timeout()),
			dialog, SLOT(slotCancel()));
		startTickle(timeout);
	}

	r = (KMessageBox::ButtonCode) KMessageBox::createKMessageBox(dialog,
		QMessageBox::Question,
		text,
		QStringList(),
		(key.isEmpty() ? QString::null : i18n("&Do not ask again")),
		&checkboxReturn,
		0);


	switch(r)
	{
	case KDialogBase::Yes : result=KMessageBox::Yes ; break;
	case KDialogBase::No  : result=KMessageBox::No; break;
	case KDialogBase::Cancel : result=KMessageBox::Cancel; break;
	default : break;
	}

	stopTickle();

	if (!key.isEmpty() && checkboxReturn)
	{
		KMessageBox::saveDontShowAgainYesNo(key,result);
	}

	return result;
#endif
}


int SyncAction::questionYesNoCancel(const QString & text,
	const QString & caption,
	const QString & key,
	unsigned timeout,
	const QString &yes,
	const QString &no)
{
	FUNCTIONSETUP;

	bool checkboxReturn = false;
	int r;
	KMessageBox::ButtonCode result;

	if (!key.isEmpty())
	{
		if (!KMessageBox::shouldBeShownYesNo(key,result))
		{
			if (result != KMessageBox::Cancel)
			{
				return result;
			}
		}
	}

	KDialogBase *dialog =
		new KDialogBase(caption.isNull()? i18n("Question") : caption,
		KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
		KDialogBase::Yes, KDialogBase::Cancel,
		fParent, "questionYesNoCancel", true, true,
		(yes.isEmpty() ? KStdGuiItem::yes() : yes),
		(no.isEmpty() ? KStdGuiItem::no() : no),
		KStdGuiItem::cancel());

	if (timeout > 0)
	{
		QObject::connect(fHandle, SIGNAL(timeout()),
			dialog, SLOT(slotCancel()));
		startTickle(timeout);
	}

#if KDE_IS_VERSION(3,3,0)
	r = KMessageBox::createKMessageBox(dialog,
		QMessageBox::Question,
		text,
		QStringList(),
		(key.isEmpty() ? QString::null : i18n("&Do not ask again")),
		&checkboxReturn,
		0);
#else
	r = KDialogBase::Cancel;
#endif

	KPILOT_DELETE(dialog);

	switch(r)
	{
	case KDialogBase::Yes : result=KMessageBox::Yes ; break;
	case KDialogBase::No  : result=KMessageBox::No; break;
	case KDialogBase::Cancel : result=KMessageBox::Cancel; break;
	default : break;
	}
	stopTickle();

	if (!key.isEmpty() && checkboxReturn)
	{
		KMessageBox::saveDontShowAgainYesNo(key,result);
	}

	return result;
}

