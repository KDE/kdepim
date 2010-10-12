/* KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kaboutdata.h>

#include <qcheckbox.h>
#include <qdir.h>
#include <qcombobox.h>

#include "kfiledialog.h"

#include <kurlrequester.h>


#include "popmail-factory.h"
#include "setup-dialog.h"
#include "setupDialog.moc"
#include "popmailSettings.h"



PopMailWidgetConfig::PopMailWidgetConfig(QWidget *p,const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new PopMailWidget(p,"PopMailWidget"))
{
	FUNCTIONSETUP;
	fConduitName = i18n("KMail");
	KAboutData *fAbout = new KAboutData("popmailConduit",
		I18N_NOOP("Mail Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Mail Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Dan Pilone, Michael Kropfberger, Adriaan de Groot");
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.kpilot.org/");
	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Original Author"));
	fAbout->addCredit("Michael Kropfberger",
		I18N_NOOP("POP3 code"));
	fAbout->addCredit("Marko Gr&ouml;nroos",
		I18N_NOOP("SMTP support and redesign"),
		"magi@iki.fi",
		"http://www.iki.fi/magi/");

	ConduitConfigBase::addAboutPage(fConfigWidget->fTabWidget,fAbout);
	fWidget=fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fSendMode,SIGNAL(activated(int)));
	CM(fEmailFrom,SIGNAL(textChanged(const QString &)));
	CM(fSignature,SIGNAL(textChanged(const QString &)));
#undef CM

	connect(fConfigWidget->fSendMode,SIGNAL(activated(int)),
		this,SLOT(toggleSendMode(int)));

}

void PopMailWidgetConfig::commit()
{
	FUNCTIONSETUP;

	MailConduitSettings::self()->readConfig();
#define WR(a,b,c) MailConduitSettings::set##a(fConfigWidget->b->c);
	WR(SyncOutgoing,fSendMode,currentItem());
	WR(EmailAddress,fEmailFrom,text());
	WR(Signature,fSignature,url());
#undef WR

	MailConduitSettings::self()->writeConfig();
	unmodified();
}

void PopMailWidgetConfig::load()
{
	FUNCTIONSETUP;
	MailConduitSettings::self()->config()->sync();
	MailConduitSettings::self()->readConfig();

#define RD(a,b,c) fConfigWidget->a->b(MailConduitSettings::c())
	RD(fSendMode,setCurrentItem,syncOutgoing);
	RD(fEmailFrom,setText,emailAddress);
	RD(fSignature,setURL,signature);
#undef RD

	toggleSendMode(fConfigWidget->fSendMode->currentItem());

	MailConduitSettings::self()->writeConfig();
	unmodified();
}


/* slot */ void PopMailWidgetConfig::toggleSendMode(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGKPILOT << fname << ": Got mode " << i << endl;
#endif

#define E(a,b) fConfigWidget->a->setEnabled(b)
	switch(i)
	{
	case SendKMail :
		E(fEmailFrom,true);
		E(fSignature,true);
		break;
	case NoSend : /* FALLTHRU */
	default :
		E(fEmailFrom,false);
		E(fSignature,false);
		break;
	}
#undef E
}



