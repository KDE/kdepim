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

SyncAction::SyncAction(KPilotLink  *p,
	const char *name) :
	QObject(p, name),
	fHandle(p),
	fParent(0L)
{
	FUNCTIONSETUP;
}

SyncAction::SyncAction(KPilotLink *p,
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

	DEBUGKPILOT << fname << ": Exec " << name() << endl;

	bool r = this->exec();

	DEBUGKPILOT << fname << ": Exec " << name()
		<< (r ? " is running" : " failed to start") << endl;

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

	if (!maps[i].name)
	{
		WARNINGKPILOT << "No mode set by arguments ("
			<< args.join(",") << ") defaulting to HotSync." << endl;
	}
}

SyncAction::SyncMode::SyncMode(Mode m, bool test, bool local) :
	fMode(m),
	fTest(test),
	fLocal(local)
{
	if ( ((int)m<(int)eHotSync) || ((int)m>(int)eRestore) )
	{
		WARNINGKPILOT << "Mode value " << (int)m << " is illegal"
			", defaulting to HotSync." << endl;
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
		WARNINGKPILOT << "Mode " << fMode << " does not have a name." << endl;
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
		WARNINGKPILOT << "Bad sync mode " << mode << " requested." << endl ;
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

	WARNINGKPILOT << "Bad sync mode " << m << " requested." << endl ;
	fMode = eHotSync;
	return false;
}

void SyncAction::startTickle(unsigned timeout)
{
	FUNCTIONSETUP;

	if (!deviceLink())
	{
		WARNINGKPILOT << "Trying to tickle without a device." << endl;
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
		WARNINGKPILOT << "Trying to tickle without a device." << endl;
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

	KDialogBase *dialog =
		new KDialogBase(caption.isNull()? i18n("Question") : caption,
		KDialogBase::Yes | KDialogBase::No,
		KDialogBase::Yes, KDialogBase::No,
		fParent, "questionYesNo", true, true,
		yes.isEmpty() ? KStdGuiItem::yes() : yes,
		no.isEmpty() ? KStdGuiItem::no() : no);

	if ( (timeout > 0) && ( deviceLink() ) )
	{
		QObject::connect(deviceLink(), SIGNAL(timeout()),
			dialog, SLOT(slotCancel()));
		startTickle(timeout);
	}

#if KDE_IS_VERSION(3,3,0)
	r = (KMessageBox::ButtonCode) KMessageBox::createKMessageBox(dialog,
		QMessageBox::Question,
		text,
		QStringList(),
		(key.isEmpty() ? QString::null : i18n("&Do not ask again")),
		&checkboxReturn,
		0);

#else
	// The following code is taken from KDialogBase.cc,
	// part of the KDE 2.2 libraries. Copyright 2001
	// by Waldo Bastian.
	//
	//
	QVBox *topcontents = new QVBox(dialog);

	topcontents->setSpacing(KDialog::spacingHint() * 2);
	topcontents->setMargin(KDialog::marginHint() * 2);

	QWidget *contents = new QWidget(topcontents);
	QHBoxLayout *lay = new QHBoxLayout(contents);

	lay->setSpacing(KDialog::spacingHint() * 2);

	lay->addStretch(1);
	QLabel *label1 = new QLabel( contents);
	label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
	lay->add( label1 );
	QLabel *label2 = new QLabel( text, contents);
	label2->setMinimumSize(label2->sizeHint());
	lay->add(label2);
	lay->addStretch(1);

	QSize extraSize = QSize(50, 30);

	QCheckBox *checkbox = 0L;
	if (!key.isEmpty())
	{
		checkbox = new QCheckBox(i18n("Do not ask again"),topcontents);
		extraSize = QSize(50,0);
	}

	dialog->setMainWidget(topcontents);
	dialog->enableButtonSeparator(false);
	dialog->incInitialSize(extraSize);

	r = dialog->exec();
	if (checkbox)
	{
		checkboxReturn = checkbox->isChecked();
	}
#endif

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

	if ( (timeout > 0) && (deviceLink()) )
	{
		QObject::connect(deviceLink(), SIGNAL(timeout()),
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
	// The following code is taken from KDialogBase.cc,
	// part of the KDE 2.2 libraries. Copyright 2001
	// by Waldo Bastian.
	//
	//
	QVBox *topcontents = new QVBox(dialog);

	topcontents->setSpacing(KDialog::spacingHint() * 2);
	topcontents->setMargin(KDialog::marginHint() * 2);

	QWidget *contents = new QWidget(topcontents);
	QHBoxLayout *lay = new QHBoxLayout(contents);

	lay->setSpacing(KDialog::spacingHint() * 2);

	lay->addStretch(1);
	QLabel *label1 = new QLabel( contents);
	label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
	lay->add( label1 );
	QLabel *label2 = new QLabel( text, contents);
	label2->setMinimumSize(label2->sizeHint());
	lay->add(label2);
	lay->addStretch(1);

	QSize extraSize = QSize(50, 30);

	QCheckBox *checkbox = 0L;
	if (!key.isEmpty())
	{
		checkbox = new QCheckBox(i18n("Do not ask again"),topcontents);
		extraSize = QSize(50,0);
	}

	dialog->setMainWidget(topcontents);
	dialog->enableButtonSeparator(false);
	dialog->incInitialSize(extraSize);

	r = dialog->exec();
	if (checkbox)
	{
		checkboxReturn = checkbox->isChecked();
	}
#endif

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

