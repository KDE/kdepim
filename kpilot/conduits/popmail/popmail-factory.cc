/* popmail-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the popmail-conduit plugin.
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
#include <qlineedit.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>

#include "setupDialog.h"

#include "popmail-factory.moc"


extern "C"
{

void *init_libpopmailconduit()
{
	return new PopmailConduitFactory;
}

} ;


KAboutData *PopmailConduitFactory::fAbout = 0L;
PopmailConduitFactory::PopmailConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("popmailconduit");
	fAbout = new KAboutData("popmailConduit",
		I18N_NOOP("Mail Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Mail Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Dan Pilone, Michael Kropfberger, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addAuthor("Michael Kropfberger",
		I18N_NOOP("Original Author"));
}

PopmailConduitFactory::~PopmailConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *PopmailConduitFactory::createObject( QObject *p,
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
			return new PopmailWidgetSetup(w,n,a);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Couldn't cast parent to widget."
				<< endl;
#endif
			return 0L;
		}
	}

#if 0
	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new PopmailConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}
#endif
	return 0L;
}

PopmailWidgetSetup::PopmailWidgetSetup(QWidget *w, const char *n,
	const QStringList & a) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;

	QTabWidget *t = new QTabWidget(widget());
	fSendPage = new PopMailSendPage(t);
	t->addTab(fSendPage,i18n("Send Mail"));
	fRecvPage = new PopMailReceivePage(t);
	t->addTab(fRecvPage,i18n("Retrieve Mail"));

	setTabWidget(t);
	addAboutPage(false,PopmailConduitFactory::about());

	t->adjustSize();
}

PopmailWidgetSetup::~PopmailWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void PopmailWidgetSetup::commitChanges()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Popmail-conduit");

	fSendPage->commitChanges(*fConfig);
	fRecvPage->commitChanges(*fConfig);
}

/* virtual */ void PopmailWidgetSetup::readSettings()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver s(fConfig,"Popmail-conduit");

	fSendPage->readSettings(*fConfig);
	fRecvPage->readSettings(*fConfig);
}


// $Log$

