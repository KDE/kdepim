/* interactiveSync.cc                   KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file specializes SyncAction to a kind that can have interaction
** with the user without the Sync timing out.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

static const char *interactivesync_id =
	"$Id$";

#include "options.h"

#include <time.h>

#include <pi-socket.h>
#include <pi-file.h>

#include <qtimer.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtl.h>
#include <qstyle.h>

#include <kdialogbase.h>
#include <kglobal.h>
#include <kstddirs.h>

#if KDE_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif

#include "pilotUser.h"
#include "kpilotConfig.h"
#include "interactiveSync.moc"


InteractiveAction::InteractiveAction(KPilotDeviceLink * p,
	QWidget * visibleparent,
	QObject * parent,
	const char *name) :
	SyncAction(p, parent, name),
	fParent(visibleparent), 
	fTickleTimer(0L), 
	fTickleCount(0), 
	fTickleTimeout(0)
{
	FUNCTIONSETUP;

	(void) interactivesync_id;
}

InteractiveAction::~InteractiveAction()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTickleTimer);
}


void InteractiveAction::startTickle(unsigned timeout)
{
	FUNCTIONSETUP;
	fTickleTimeout = timeout;
	fTickleCount = 0;
	if (!fTickleTimer)
	{
		fTickleTimer = new QTimer(this);
		QObject::connect(fTickleTimer, SIGNAL(timeout()),
			this, SLOT(tickle()));
	}
	else
	{
		fTickleTimer->stop();
	}

	fTickleTimer->start(1000, false);
}

void InteractiveAction::stopTickle()
{
	FUNCTIONSETUP;
	if (fTickleTimer)
	{
		fTickleTimer->stop();
	}
}

void InteractiveAction::tickle()
{
	FUNCTIONSETUP;
	fTickleCount++;

	// Note that if fTickleTimeout == 0 then this
	// test will never be true until unsigned wraps
	// around, which is 2^32 seconds, which is a long time.
	//
	// This is intentional.
	//
	//
	if (fTickleCount == fTickleTimeout)
	{
		emit timeout();
	}
	else
	{
		if (pi_tickle(pilotSocket()))
		{
			kdWarning() << __FUNCTION__
				<< "Couldn't tickle Pilot!" << endl;
		}
	}
}

int InteractiveAction::questionYesNo(const QString & text,
	const QString & caption, unsigned timeout)
{
	FUNCTIONSETUP;

	KDialogBase *dialog =
		new KDialogBase(caption.isNull()? i18n("Question") : caption,
		KDialogBase::Yes | KDialogBase::No,
		KDialogBase::Yes, KDialogBase::No,
		fParent, "questionYesNo", true, true,
		i18n("Yes"), i18n("No"));

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
#if QT_VERSION < 300
	label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information,
        	kapp->style().guiStyle()));
#else
	label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
#endif
	lay->add( label1 );
	QLabel *label2 = new QLabel( text, contents);
	label2->setMinimumSize(label2->sizeHint());
	lay->add(label2);
	lay->addStretch(1);

	QSize extraSize = QSize(50, 30);

	dialog->setMainWidget(topcontents);
	dialog->enableButtonSeparator(false);
	dialog->incInitialSize(extraSize);

	QTimer *timer = new QTimer(dialog);

	QObject::connect(timer, SIGNAL(timeout()),
		dialog, SLOT(slotCancel()));
	int result = dialog->exec();

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Dialog returned " << result << endl;
#endif

	delete dialog;

	return result;
}

CheckUser::CheckUser(KPilotDeviceLink * p, QWidget * vp, QObject * o):
	InteractiveAction(p, vp, o, "userCheck")
{
	FUNCTIONSETUP;
}

CheckUser::~CheckUser()
{
	FUNCTIONSETUP;
}

/* virtual */ void CheckUser::exec()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & config = KPilotConfig::getConfig();
	config.resetGroup();

	QString guiUserName = config.getUser();
	const char *pilotUserName = fHandle->getPilotUser()->getUserName();
	bool pilotUserEmpty = (!pilotUserName) || (!strlen(pilotUserName));

	// 4 cases to handle:
	//    guiUserName empty / not empty
	//    pilotUserName empty / not empty
	//
	//
	if (guiUserName.isEmpty())
	{
		if (pilotUserEmpty)
		{
			QString defaultUserName =
				i18n("A common name", "John Doe");

			QString q = i18n("<qt>Neither KPilot nor the "
				"Pilot have a user name set. "
				"They <i>should</i> be set. "
				"Should KPilot set them to a default value "
				"(<i>%1</i>)?</qt>").arg(defaultUserName);

			if (questionYesNo(q, i18n("User Unknown")) ==
				KDialogBase::Yes)
			{
				config.setUser(defaultUserName);
				fHandle->getPilotUser()->
					setUserName(defaultUserName.latin1());
			}

		}
		else
		{
			QString q = i18n("<qt>The Pilot has a user name set "
				"(<i>%1</i>) but KPilot does not. Should "
				"KPilot use this user name in future?").
				arg(pilotUserName);

			if (questionYesNo(q, i18n("User Unknown")) ==
				KDialogBase::Yes)
			{
				config.setUser(pilotUserName);
			}
		}
	}
	else
	{
		if (pilotUserEmpty)
		{
			QString q = i18n("<qt>KPilot has a user name set "
				"(<i>%1</i>) but the Pilot does not. "
				"Should KPilot's user name be set in the "
				"Pilot as well?").arg(guiUserName);

			if (questionYesNo(q, i18n("User Unknown")) ==
				KDialogBase::Yes)
			{
#ifdef DEBUG
				DEBUGDAEMON << fname
					<< ": Setting user name in pilot"
					<< endl;
#endif

				const char *l1 = guiUserName.latin1();

#ifdef DEBUG
				DEBUGDAEMON << fname
					<< ": Setting to " << l1 << endl;
#endif

				fHandle->getPilotUser()->setUserName(l1);
			}
		}
		else
		{
			if (guiUserName != pilotUserName)
			{
				QString q = i18n("<qt>The Pilot thinks that "
					"the user name is %1, "
					"however KPilot says you are %2."
					"Should I assume the Pilot is right "
					"and set the user name "
					"for KPilot to %1?").
					arg(pilotUserName).arg(pilotUserName).
					arg(guiUserName);

				int r = questionYesNo(q,
					i18n("User Mismatch"));
				if (r == KDialogBase::Yes)
				{
					config.setUser(pilotUserName);
				}
			}
		}
	}

	emit syncDone(this);
}

class RestoreAction::RestoreActionPrivate {
      public:
	QString fDatabaseDir;
	QValueList < struct db >fDBList;
	QTimer fTimer;
	int fDBIndex;
};

bool operator < (const db & a, const db & b) {
	if (a.creator == b.creator)
	{
		if (a.type != b.type)
		{
			if (a.type == pi_mktag('a', 'p', 'p', 'l'))
				return false;
			else
				return true;
		}
	}

	return a.maxblock < b.maxblock;
}

RestoreAction::RestoreAction(KPilotDeviceLink * p, QWidget * visible, 
	QObject * o) :
	InteractiveAction(p, visible, o, "restoreAction")
{
	FUNCTIONSETUP;

	fP = new RestoreActionPrivate;
	fP->fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		QString("kpilot/DBBackup/"));
}

/* virtual */ void RestoreAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Restoring from base directory "
		<< fP->fDatabaseDir << endl;
#endif

	QString dirname = fP->fDatabaseDir +
		fHandle->getPilotUser()->getUserName() + "/";

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Restoring user " << dirname << endl;
#endif

	if (questionYesNo(i18n("<qt>Are you sure you want to completely "
				"restore your Pilot from the backup directory "
				"(<i>%1</i>)? This will erase any information "
				"you currently have on your Pilot.</qt>").
			arg(dirname),
			i18n("Restore Pilot")) != KDialogBase::Yes)
	{
		emit logError(i18n("Restore <i>not</i> performed."));

		addSyncLogEntry(i18n("Restore not performed."));
		emit syncDone(this);

		return;
	}

	QDir dir(dirname, QString::null, QDir::Name,
		QDir::Files | QDir::Readable | QDir::NoSymLinks);

	if (!dir.exists())
	{
		kdWarning() << __FUNCTION__
			<< ": Restore directory "
			<< dirname << " does not exist." << endl;
		fStatus = Error;
		return;
	}

	for (int i = 0; i < dir.count(); i++)
	{
		QString s;
		struct db dbi;
		struct DBInfo info;
		struct pi_file *f;

		s = dir[i];

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Adding " << s << " to restore list." << endl;
#endif

		qstrcpy(dbi.name, QFile::encodeName(dirname + s));

		f = pi_file_open(dbi.name);
		if (!f)
		{
			kdWarning() << __FUNCTION__
				<< ": Can't open " << dbi.name << endl;
			continue;
		}

		pi_file_get_info(f, &info);

		dbi.creator = info.creator;
		dbi.type = info.type;
		dbi.flags = info.flags;
		dbi.maxblock = 0;

		fP->fDBList.append(dbi);

		pi_file_close(f);
		f = 0L;
	}

	fP->fDBIndex = 0;
	fStatus = GettingFileInfo;

	QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(getNextFileInfo()));

	fP->fTimer.start(0, false);
}

/* slot */ void RestoreAction::getNextFileInfo()
{
	FUNCTIONSETUP;

	ASSERT(fStatus == GettingFileInfo);
	ASSERT(fP->fDBIndex < fP->fDBList.count());

	struct db &dbi = fP->fDBList[fP->fDBIndex];
	pi_file *f;

	fP->fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Getting info on " << dbi.name << endl;
#endif

	f = pi_file_open(dbi.name);
	if (!f)
	{
		kdWarning() << __FUNCTION__
			<< ": Can't open " << dbi.name << endl;
		goto nextFile;
	}

	int max;

	pi_file_get_entries(f, &max);

	for (int i = 0; i < max; i++)
	{
		int size;

		if (dbi.flags & dlpDBFlagResource)
		{
			pi_file_read_resource(f, i, 0, &size, 0, 0);
		}
		else
		{
			pi_file_read_record(f, i, 0, &size, 0, 0, 0);
		}

		if (size > dbi.maxblock)
		{
			dbi.maxblock = size;
		}
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Read " << max << " entries for this database." << endl;
#endif

      nextFile:
	if (f)
		pi_file_close(f);

	if (fP->fDBIndex >= fP->fDBList.count())
	{
		QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(getNextFileInfo()));
		fP->fTimer.stop();

		qBubbleSort(fP->fDBList);

		fP->fDBIndex = 0;
		fStatus = InstallingFiles;

		QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(installNextFile()));
		fP->fTimer.start(0, false);
	}
}

/* slot */ void RestoreAction::installNextFile()
{
	FUNCTIONSETUP;

	ASSERT(fStatus == InstallingFiles);
	ASSERT(fP->fDBIndex < fP->fDBList.count());

	struct db &dbi = fP->fDBList[fP->fDBIndex];

	fP->fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Trying to install " << dbi.name << endl;
#endif

	if (fP->fDBIndex >= fP->fDBList.count() - 1)
	{
		QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(getNextFileInfo()));
		fP->fTimer.stop();

		fStatus = Done;
	}

	if (dlp_OpenConduit(pilotSocket()) < 0)
	{
		kdWarning() << __FUNCTION__
			<< ": Restore apparently cancelled." << endl;
		fStatus = Done;
		emit syncDone(this);

		return;
	}

	addSyncLogEntry(i18n("Restoring %1 ...").arg(dbi.name));

	pi_file *f =
		pi_file_open(const_cast <
		char *>((const char *)QFile::encodeName(dbi.name)));
	if (!f)
	{
		kdWarning() << __FUNCTION__
			<< ": Can't open "
			<< dbi.name << " for restore." << endl;
		return;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << __FUNCTION__
			<< ": Couldn't  restore " << dbi.name << endl;
	}

	pi_file_close(f);


	if (fStatus == Done)
	{
		addSyncLogEntry(i18n("OK."));
		emit syncDone(this);
	}
}

/* virtual */ QString RestoreAction::statusString() const
{
	FUNCTIONSETUP;
	QString s;

	switch (status())
	{
	case InstallingFiles:
		s.append("Installing Files (");
		s.append(QString::number(fP->fDBIndex));
		s.append(")");
		break;
	case GettingFileInfo:
		s.append("Getting File Info (");
		s.append(QString::number(fP->fDBIndex));
		s.append(")");
		break;
	default:
		return SyncAction::statusString();
	}

	return s;
}


// $Log$
// Revision 1.3  2001/09/27 17:28:32  adridg
// Conflict management
//
// Revision 1.2  2001/09/25 08:22:29  cschumac
// Make it compile with Qt3.
//
// Revision 1.1  2001/09/24 22:25:54  adridg
// New SyncActions with support for interaction with the user
//
