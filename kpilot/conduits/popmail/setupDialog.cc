/* setupDialog.cc			KPilot
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static const char *setupDialog_id=
	"$Id$";

#include "options.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <kconfig.h>
#include <kstddirs.h>
#include <klineedit.h>

#include <qcheckbox.h>
#include <qdir.h>
#include <qcombobox.h>

#include "kfiledialog.h"

#include <kurlrequester.h>

#include "uiDialog.h"

#include "popmail-factory.h"
#include "setup-dialog.h"
#include "setupDialog.moc"
#include "popmailSettings.h"



PopMailWidgetConfig::PopMailWidgetConfig(QWidget *p,const char *n) :
	ConduitConfigBase(p,n),
	fConfigWidget(new PopMailWidget(p,"PopMailWidget"))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Popmail");
	UIDialog::addAboutPage(fConfigWidget->fTabWidget,PopMailConduitFactory::about());
	fWidget=fConfigWidget;

#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM(fStorePass,SIGNAL(toggled(bool)));
	CM(fPopPass,SIGNAL(textChanged(const QString &)));
	CM(fRecvMode,SIGNAL(activated(int)));
	CM(fSendMode,SIGNAL(activated(int)));
#undef CM

	connect(fConfigWidget->fStorePass,SIGNAL(toggled(bool)),
		fConfigWidget->fPopPass,SLOT(setEnabled(bool)));
	connect(fConfigWidget->fRecvMode,SIGNAL(activated(int)),
		this,SLOT(toggleRecvMode(int)));
	connect(fConfigWidget->fSendMode,SIGNAL(activated(int)),
		this,SLOT(toggleSendMode(int)));

	(void) setupDialog_id;
}

void PopMailWidgetConfig::commit()
{
	FUNCTIONSETUP;
	KConfig*fConfig=MailConduitSettings::self()->config();
	KConfigGroupSaver s(fConfig,PopMailConduitFactory::group());
#define WR(a,b,c) fConfig->writeEntry(c,fConfigWidget->a->b);
	WR(fSendMode,currentItem(),PopMailConduitFactory::syncIncoming());
	WR(fEmailFrom,text(),"EmailAddress");
	WR(fSignature,url(),"Signature");
	WR(fLeaveMail,isChecked(),"LeaveMail");
#undef WR
}

void PopMailWidgetConfig::load()
{
	FUNCTIONSETUP;
	KConfig*fConfig=MailConduitSettings::self()->config();
	KConfigGroupSaver s(fConfig,PopMailConduitFactory::group());
#define RD(a,b,c,d,e) fConfigWidget->a->b(fConfig->read##c##Entry(d,e))
	RD(fSendMode,setCurrentItem,Num,PopMailConduitFactory::syncIncoming(),(int)NoSend);
	RD(fRecvMode,setCurrentItem,Num,PopMailConduitFactory::syncOutgoing(),(int)NoRecv);
	RD(fEmailFrom,setText,,"EmailAddress",QString::null);
	RD(fSignature,setURL,,"Signature",CSL1("$HOME/.signature"));
	RD(fLeaveMail,setChecked,Bool,"LeaveMail",true);
#undef RD

	toggleSendMode(fConfigWidget->fSendMode->currentItem());
	toggleRecvMode(fConfigWidget->fRecvMode->currentItem());
}


/* slot */ void PopMailWidgetConfig::toggleRecvMode(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got mode " << i << endl;
#endif

#define E(a,b) fConfigWidget->a->setEnabled(b)
	switch(i)
	{
	case RecvPOP :
		E(fPopPass,true);
		E(fStorePass,true);
		E(fPopServer,true);
		E(fPopUser,true);
		E(fLeaveMail,true);
		E(fMailbox,false);
		break;
	case RecvMBOX :
		E(fPopPass,false);
		E(fStorePass,false);
		E(fPopServer,false);
		E(fPopUser,false);
		E(fLeaveMail,false);
		E(fMailbox,true);
		break;
	case NoRecv : /* FALLTHRU */
	default :
		E(fPopPass,false);
		E(fStorePass,false);
		E(fPopServer,false);
		E(fPopUser,false);
		E(fLeaveMail,false);
		E(fMailbox,false);
		break;
	}
#undef E
}

/* slot */ void PopMailWidgetConfig::toggleSendMode(int i)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got mode " << i << endl;
#endif

#define E(a,b) fConfigWidget->a->setEnabled(b)
	switch(i)
	{
	case SendKMail :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,false);
		E(fSendmailCmd,false);
		break;
	case SendSMTP :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,true);
		E(fSendmailCmd,false);
		break;
	case SendSendmail :
		E(fEmailFrom,true);
		E(fSignature,true);
		E(fSMTPServer,false);
		E(fSendmailCmd,true);
		break;
	case NoSend : /* FALLTHRU */
	default :
		E(fEmailFrom,false);
		E(fSignature,false);
		E(fSMTPServer,false);
		E(fSendmailCmd,false);
		break;
	}
#undef E
}



