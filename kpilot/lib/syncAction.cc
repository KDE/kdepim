/* syncAction.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
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

#if KDE_VERSION < 300
#include <kapplication.h>
#else
#include <kapplication.h>
#endif

#include "syncAction.moc"


SyncAction::SyncAction(KPilotDeviceLink  *p,
	const char *name) :
	QObject(p, name),
	fHandle(p)
{
	FUNCTIONSETUP;
	(void) syncAction_id;
}

/* virtual */ QString SyncAction::statusString() const
{
	FUNCTIONSETUP;
	QString s("status=");

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
			.arg(name()));
		emit syncDone(this);
	}
}

/* slot */ void SyncAction::slotDelayDone()
{
	emit syncDone(this);
}

void SyncAction::delayDone()
{
	QTimer::singleShot(0,this,SLOT(slotDelayDone()));
}

InteractiveAction::InteractiveAction(KPilotDeviceLink *p,
	QWidget * visibleparent,
	const char *name) :
	SyncAction(p, name),
	fParent(visibleparent),
	fTickleTimer(0L), 
	fTickleCount(0), 
	fTickleTimeout(0)
{
	FUNCTIONSETUP;
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
			kdWarning() << k_funcinfo
				<< "Couldn't tickle Pilot!" << endl;
		}
	}
}

int InteractiveAction::questionYesNo(const QString & text,
	const QString & caption,
	const QString & key,
	unsigned timeout)
{
	FUNCTIONSETUP;

	KConfig *config = kapp->config();
	KConfigGroupSaver cfgs(config,"Notification Messages");


	if (!key.isEmpty())
	{
		QString prev = config->readEntry(key).lower();

		if (prev == "yes")
		{
			return KDialogBase::Yes;
		}
		else if (prev == "no")
		{
			return KDialogBase::No;
		}
	}

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

	QCheckBox *checkbox = 0L;
	if (!key.isEmpty())
	{
		checkbox = new QCheckBox(i18n("Do not ask again"),topcontents);
		extraSize = QSize(50,0);
	}

	dialog->setMainWidget(topcontents);
	dialog->enableButtonSeparator(false);
	dialog->incInitialSize(extraSize);

	QTimer *timer = new QTimer(dialog);

	QObject::connect(timer, SIGNAL(timeout()),
		dialog, SLOT(slotCancel()));
	if (timeout > 0)
	{
		timer->start(timeout,true);
	}

	int result = dialog->exec();

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Dialog returned " << result << endl;
#endif

	if (!key.isEmpty() && checkbox && checkbox->isChecked())
	{
		if (result == KDialogBase::Yes)
		{
			config->writeEntry(key,"Yes");
		}
		else if (result == KDialogBase::No)
		{
			config->writeEntry(key,"No");
		}
	}

	delete dialog;
	return result;
}
