/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2001 by Waldo Bastian (code in questionYesNo) <waldo@kpilot.org>
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

#include <time.h>

#include <pi-socket.h>
#include <pi-dlp.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QStyle>

#include <kdialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "syncAction.moc"
#include "kpilotlibSettings.h"

SyncAction::SyncAction(KPilotLink  *p,
	const char * name)
	: QObject(p),
	fHandle(p),
	fParent(0L)
{
	FUNCTIONSETUP;
	setObjectName(name);
}

SyncAction::SyncAction(KPilotLink *p,
	QWidget * visibleparent,
	const char *name) :
	QObject(p),
	fHandle(p),
	fParent(visibleparent)
{
	FUNCTIONSETUP;
	setObjectName(name);
}

SyncAction::~SyncAction()
{
   FUNCTIONSETUPL(5);
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

	bool r = this->exec();

	DEBUGKPILOT << "Exec, " << objectName()
		<< (r ? " is running" : " failed to start");

	if (!r)
	{
		emit logError(i18n("The conduit %1 could not be executed.",
			objectName()));
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

static struct
{
	SyncAction::SyncMode::Mode mode;
	const char *name;
} maps[] =
{
	{ SyncAction::SyncMode::eHotSync, "--hotsync" },
	{ SyncAction::SyncMode::eFullSync, "--full" },
	{ SyncAction::SyncMode::eCopyPCToHH, "--copyPCToHH" },
	{ SyncAction::SyncMode::eCopyHHToPC, "--copyHHToPC" },
	{ SyncAction::SyncMode::eBackup, "--backup" },
	{ SyncAction::SyncMode::eRestore, "--restore" },
	{ SyncAction::SyncMode::eFullSync, "--fullsync" },
	{ SyncAction::SyncMode::eHotSync, (const char *)0 }
}
;

SyncAction::SyncMode::SyncMode(const QStringList &args) :
	fMode(eHotSync),
	fTest(args.contains("--test")),
	fLocal(args.contains("--local"))
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "args passed in: [" << args.join(",") << "]";
	
	int i = 0;
	while(maps[i].name)
	{
		if (args.contains(QString::fromLatin1(maps[i].name)))
		{
			fMode = maps[i].mode;
			break;
		}
		i++;
	}

	DEBUGKPILOT << "using 'i' of: [" << i << "]";
	
	if (!maps[i].name)
	{
		WARNINGKPILOT << "No mode set by arguments ("
			<< args.join(",") << ") defaulting to HotSync.";
	}
}

SyncAction::SyncMode::SyncMode(Mode m, bool test, bool local) :
	fMode(m),
	fTest(test),
	fLocal(local)
{
	if ( ((int)m<(int)eHotSync) || ((int)m>(int)eRestore) )
	{
		WARNINGKPILOT << "Mode value" << (int)m << " is illegal"
			", defaulting to HotSync.";
		fMode = eHotSync;
	}
}

QStringList SyncAction::SyncMode::list() const
{
	FUNCTIONSETUPL(3);

	QStringList l;
	int i=0;

	while(maps[i].name)
	{
		if ( fMode == maps[i].mode )
		{
			l.append(QString::fromLatin1(maps[i].name));
			break;
		}
		i++;
	}
	if ( !maps[i].name )
	{
		WARNINGKPILOT << "Mode" << fMode << " does not have a name.";
		l.append(QString::fromLatin1(maps[0].name));
	}

	if (isTest()) l.append(CSL1("--test"));
	if (isLocal()) l.append(CSL1("--local"));
	return l;
}

QVariantList SyncAction::SyncMode::variantList() const
{
	FUNCTIONSETUPL(3);

	QVariantList l;
	int i=0;

	while(maps[i].name)
	{
		if ( fMode == maps[i].mode )
		{
			l.append(QString::fromLatin1(maps[i].name));
			break;
		}
		i++;
	}
	if ( !maps[i].name )
	{
		WARNINGKPILOT << "Mode" << fMode << " does not have a name.";
		l.append(QString::fromLatin1(maps[0].name));
	}

	if (isTest()) l.append(CSL1("--test"));
	if (isLocal()) l.append(CSL1("--local"));
	return l;
}

/* static */ QString SyncAction::SyncMode::name(SyncAction::SyncMode::Mode e)
{
	switch(e)
	{
	case eHotSync : return i18n("HotSync");
	case eFullSync : return i18n("Full Synchronization");
	case eCopyPCToHH : return i18n("Copy PC to Handheld");
	case eCopyHHToPC : return i18n("Copy Handheld to PC");
	case eBackup : return i18n("Backup");
	case eRestore : return i18n("Restore From Backup");
	}
	return CSL1("<unknown>");
}

QString SyncAction::SyncMode::name() const
{
	QString s = name(fMode);
	if (isTest())
	{

		s.append(CSL1(" [%1]").arg(i18n("Test Sync")));
	}
	if (isLocal())
	{
		s.append(CSL1(" [%1]").arg(i18n("Local Sync")));
	}
	return s;
}

bool SyncAction::SyncMode::setMode(int mode)
{
	// Resets test and local flags too
	fTest = fLocal = false;

	if ( (mode>0) && (mode<=eRestore) )
	{
		fMode = (SyncAction::SyncMode::Mode) mode;
		return true;
	}
	else
	{
		WARNINGKPILOT << "Bad sync mode" << mode << " requested.";
		fMode = eHotSync;
		return false;
	}
}

bool SyncAction::SyncMode::setMode(SyncAction::SyncMode::Mode m)
{
	int i=0;
	while ( maps[i].name )
	{
		if ( maps[i].mode == m )
		{
			fMode = m;
			return true;
		}
		i++;
	}

	WARNINGKPILOT << "Bad sync mode" << m << " requested.";
	fMode = eHotSync;
	return false;
}

void SyncAction::startTickle(unsigned timeout)
{
	FUNCTIONSETUP;

	if (!deviceLink())
	{
		WARNINGKPILOT << "Trying to tickle without a device.";
	}
	else
	{
		connect(deviceLink(),SIGNAL(timeout()),this,SIGNAL(timeout()));
		deviceLink()->startTickle(timeout);
	}
}

void SyncAction::stopTickle()
{
	FUNCTIONSETUP;
	if (!deviceLink())
	{
		WARNINGKPILOT << "Trying to tickle without a device.";
	}
	else
	{
		disconnect(deviceLink(),SIGNAL(timeout()),this,SIGNAL(timeout()));
		deviceLink()->stopTickle();
	}
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

	KDialog *dialog = new KDialog(fParent);
	dialog->setCaption(caption.isNull()? i18n("Question") : caption);
	dialog->setButtons(KDialog::Yes | KDialog::No);
	dialog->setDefaultButton(KDialog::Yes);
	dialog->setEscapeButton(KDialog::Cancel);
	dialog->setObjectName("questionYesNo");
	dialog->showButtonSeparator(true);
	dialog->setButtonText(KDialog::Yes, yes.isEmpty() ?
		KStandardGuiItem::yes().text() : yes);
	dialog->setButtonText(KDialog::No,  no.isEmpty()  ?
		KStandardGuiItem::no().text()  : no);

	if ( (timeout > 0) && ( deviceLink() ) )
	{
		QObject::connect(deviceLink(), SIGNAL(timeout()),
			dialog, SLOT(slotCancel()));
		startTickle(timeout);
	}

	r = (KMessageBox::ButtonCode) KMessageBox::createKMessageBox(dialog,
		QMessageBox::Question,
		text,
		QStringList(),
		(key.isEmpty() ? QString::null : i18n("&Do not ask again")),	//krazy:exclude=nullstrassign for old broken gcc
		&checkboxReturn,
		0);


	switch(r)
	{
	case KDialog::Yes : result=KMessageBox::Yes ; break;
	case KDialog::No  : result=KMessageBox::No; break;
	case KDialog::Cancel : result=KMessageBox::Cancel; break;
	default : break;
	}

	stopTickle();

	if (!key.isEmpty() && checkboxReturn)
	{
		KMessageBox::saveDontShowAgainYesNo(key,result);
	}

	return result;
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

	KDialog *dialog = new KDialog(fParent);
	dialog->setCaption(caption.isNull()? i18n("Question") : caption);
	dialog->setButtons(KDialog::Yes | KDialog::No | KDialog::Cancel);
	dialog->setDefaultButton(KDialog::Yes);
	dialog->setEscapeButton(KDialog::Cancel);
	dialog->setObjectName("questionYesNoCancel");
	dialog->showButtonSeparator(true);
	dialog->setButtonText(KDialog::Yes, yes.isEmpty() ?
		KStandardGuiItem::yes().text() : yes);
	dialog->setButtonText(KDialog::No,  no.isEmpty()  ?
		KStandardGuiItem::no().text()  : no);

	if ( (timeout > 0) && (deviceLink()) )
	{
		QObject::connect(deviceLink(), SIGNAL(timeout()),
			dialog, SLOT(slotCancel()));
		startTickle(timeout);
	}

	r = KMessageBox::createKMessageBox(dialog,
		QMessageBox::Question,
		text,
		QStringList(),
		(key.isEmpty() ? QString::null : i18n("&Do not ask again")),	//krazy:exclude=nullstrassign for old broken gcc
		&checkboxReturn,
		0);

	switch(r)
	{
	case KDialog::Yes : result=KMessageBox::Yes ; break;
	case KDialog::No  : result=KMessageBox::No; break;
	case KDialog::Cancel : result=KMessageBox::Cancel; break;
	default : break;
	}
	stopTickle();

	if (!key.isEmpty() && checkboxReturn)
	{
		KMessageBox::saveDontShowAgainYesNo(key,result);
	}

	return result;
}

