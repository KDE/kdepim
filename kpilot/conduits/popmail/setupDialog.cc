// setupDialog.cc
//
// Copyright (C) 1998,1999,2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$

// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static char *id="$Id$";



#include <sys/types.h>
#include <sys/stat.h>
#include <kconfig.h>
#include <kapp.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qdir.h>
#include <qfiledlg.h>
#include <qbttngrp.h>
#include <qpushbutton.h>
#include <qradiobt.h>
#include <kstddirs.h>
#include <kdebug.h>

#include <stream.h>
#include "popmail-conduit.h"
#include "setupDialog.moc"
#include "kpilot.h"

PopMailSendPage::PopMailSendPage(setupDialog *parent,KConfig *config) :
	setupDialogPage(i18n("Sending Mail"),parent,config)
{
	FUNCTIONSETUP;
	QLabel *currentLabel;

	//-----------------------------------------------
	//
	// Sending mail options.
	//
	currentLabel = new QLabel(i18n("Email Address: "), 
			    this);
	currentLabel->adjustSize();
	currentLabel->move(10, 10);
	fEmailFrom = new QLineEdit(this);
	fEmailFrom->setText(config->readEntry("EmailAddress", "$USER"));
	fEmailFrom->resize(200, fEmailFrom->height());
	fEmailFrom->move(110,currentLabel->y()-4);

	currentLabel = new QLabel(i18n("Signature File: "), 
			    this);
	currentLabel->adjustSize();
	currentLabel->move(10, fEmailFrom->y()+fEmailFrom->height()+SPACING );
	fSignature = new QLineEdit(this);
	fSignature->setText(config->readEntry("Signature", ""));
	fSignature->resize(200, fSignature->height());
	fSignature->move(110, currentLabel->y()-4);
	fSignatureBrowse=new QPushButton(i18n("Browse"),this);
	fSignatureBrowse->adjustSize();
	fSignatureBrowse->move(RIGHT(fSignature),fSignature->y());
	connect(fSignatureBrowse,SIGNAL(clicked()),
		this,SLOT(browseSignature()));

	currentLabel = new QLabel(i18n("Sendmail Cmd:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fSignature));
	fSendmailCmd = new QLineEdit(this);
	fSendmailCmd->setText(config->readEntry("SendmailCmd", "/usr/lib/sendmail -t -i"));
	fSendmailCmd->resize(200, fSendmailCmd->height());
	fSendmailCmd->move(110, currentLabel->y()-4 );

	currentLabel = new QLabel(i18n("SMTP Server:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, fSendmailCmd->y()+fSendmailCmd->height()+SPACING);
	fSMTPServer = new QLineEdit(this);
	fSMTPServer->setText(config->readEntry("SMTPServer", "mail"));
	fSMTPServer->resize(200, fSendmailCmd->height());
	fSMTPServer->move(110, currentLabel->y()-4);

	currentLabel = new QLabel(i18n("SMTP Port:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fSMTPServer));
	fSMTPPort = new QLineEdit(this);
	fSMTPPort->setText(config->readEntry("SMTPPort", "25"));
	fSMTPPort->resize(200, fSendmailCmd->height());
	fSMTPPort->move(110, currentLabel->y()-4);

}

/* virtual */ int PopMailSendPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;


	if (parentSetup->queryFile(i18n("Signature File"),fSignature->text()))
	{
		config->writeEntry("Signature", fSignature->text());
	}

	config->writeEntry("EmailAddress", fEmailFrom->text());


	config->writeEntry("SendmailCmd", fSendmailCmd->text());
	config->writeEntry("SMTPServer", fSMTPServer->text());
	config->writeEntry("SMTPPort", fSMTPPort->text());

	return 0;
}


void PopMailSendPage::setMode(PopMailConduit::SendMode m)
{
	FUNCTIONSETUP;

	fSendmailCmd->setEnabled(m==PopMailConduit::SEND_SENDMAIL);
	fSMTPServer->setEnabled(m==PopMailConduit::SEND_SMTP);
	fSMTPPort->setEnabled(m==PopMailConduit::SEND_SMTP);
}





PopMailGeneralPage::PopMailGeneralPage(setupDialog *parent,
	KConfig *config) :
	setupDialogPage(i18n("General"),parent,config)
{
	FUNCTIONSETUP;

	sendGroup=new QButtonGroup(i18n("Send Method"),
		this,"sb");
	sendGroup->move(10,10);

	fNoSend=new QRadioButton(i18n("&Do not send mail"),sendGroup);
	fSendmail=new QRadioButton(i18n("Use &Sendmail"),sendGroup);
	fSMTP=new QRadioButton(i18n("Use S&MTP"),sendGroup);
	fNoSend->move(10,20);
	fNoSend->adjustSize();
	fSendmail->move(10,BELOW(fNoSend));
	fSendmail->adjustSize();
	fSMTP->move(10,BELOW(fSendmail));
	fSMTP->adjustSize();

	sendGroup->adjustSize();


	setSendMethod((PopMailConduit::SendMode) 
		(config->readNumEntry("SyncOutgoing",
			PopMailConduit::SEND_NONE)));

	methodGroup=new QButtonGroup(i18n("Retrieve Method"),
		this,"bg");
	methodGroup->move(RIGHT(sendGroup),sendGroup->y());

	fNoMethod=new QRadioButton(i18n("Do &Not receive mail"),
		methodGroup);
	fPOPMethod=new QRadioButton(i18n("Use &POP3 server"),
		methodGroup);
	fUNIXMethod=new QRadioButton(i18n("Use &UNIX Mailbox"),
		methodGroup);

	fNoMethod->move(10,20);
	fNoMethod->adjustSize();
	fPOPMethod->move(10,BELOW(fNoMethod));
	fPOPMethod->adjustSize();
	fUNIXMethod->adjustSize();
	fUNIXMethod->move(10,BELOW(fPOPMethod));

	methodGroup->adjustSize();

	
	setReceiveMethod((PopMailConduit::RetrievalMode) 
		(config->readNumEntry("SyncIncoming",PopMailConduit::NONE)));

	connect(fNoSend,SIGNAL(clicked()),
		parent,SLOT(modeChangeSend()));
	connect(fSMTP,SIGNAL(clicked()),
		parent,SLOT(modeChangeSend()));
	connect(fSendmail,SIGNAL(clicked()),
		parent,SLOT(modeChangeSend()));
		
	connect(fNoMethod,SIGNAL(clicked()),
		parent,SLOT(modeChangeReceive()));
	connect(fPOPMethod,SIGNAL(clicked()),
		parent,SLOT(modeChangeReceive()));
	connect(fUNIXMethod,SIGNAL(clicked()),
		parent,SLOT(modeChangeReceive()));
}

void PopMailSendPage::browseSignature()
{
	FUNCTIONSETUP;

	QString filename=fSignature->text();

	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Signature currently "
			<< fSignature->text() << endl;
	}

	if (filename.isEmpty()) 
	{
		filename=QDir::currentDirPath();
	}
	else
	{
		filename=QFileInfo( filename ).dirPath();
	}

	filename = QFileDialog::getOpenFileName(filename,"*");

	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": Signature selected "
			<< filename << endl;
	}

	if (!filename.isEmpty())
	{
		fSignature->setText(filename);
	}
}



void PopMailGeneralPage::setReceiveMethod(
	PopMailConduit::RetrievalMode method)
{
	FUNCTIONSETUP;

	fNoMethod->setChecked(method==PopMailConduit::NONE);
	fPOPMethod->setChecked(method==PopMailConduit::POP);
	fUNIXMethod->setChecked(method==PopMailConduit::UNIXMailbox);
}

void PopMailGeneralPage::setSendMethod(
	PopMailConduit::SendMode method)
{
	FUNCTIONSETUP;

	fNoSend->setChecked(method==PopMailConduit::SEND_NONE);
	fSendmail->setChecked(method==PopMailConduit::SEND_SENDMAIL);
	fSMTP->setChecked(method==PopMailConduit::SEND_SMTP);
}

PopMailConduit::SendMode PopMailGeneralPage::getSendMethod() const
{
	if (fSendmail->isChecked())
	{
		return PopMailConduit::SEND_SENDMAIL;
	}
	if (fSMTP->isChecked())
	{
		return PopMailConduit::SEND_SMTP;
	}

	return PopMailConduit::SEND_NONE;
}

PopMailConduit::RetrievalMode PopMailGeneralPage::getReceiveMethod() const
{
	if (fPOPMethod->isChecked())
	{
		return PopMailConduit::POP;
	}
	if (fUNIXMethod->isChecked())
	{
		return PopMailConduit::UNIXMailbox;
	}
	return PopMailConduit::NONE;
}

/* virtual */ int PopMailGeneralPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	PopMailConduit::RetrievalMode method=getReceiveMethod();
	PopMailConduit::SendMode send=getSendMethod();


	config->writeEntry("SyncIncoming", (int)method);
	config->writeEntry("SyncOutgoing", (int)send);

	return 0;
}



PopMailUNIXPage::PopMailUNIXPage(setupDialog *parent,
	KConfig *config) :
	setupDialogPage(i18n("Mailbox"),parent,config)
{
	FUNCTIONSETUP;
	QLabel *currentLabel;

	currentLabel = new QLabel(i18n("UNIX Mailbox:"),this);
	currentLabel->adjustSize();
	currentLabel->move(10,10);

	{
	QString defaultMailbox;
	char *u=getenv("USER");
	if (u==0L)
	{
		u=getenv("HOME");
		if (u==0L)
		{
			defaultMailbox="mbox";
		}
		else
		{
			defaultMailbox=QString(u)+QString("mbox");
		}
	}
	else
	{
		defaultMailbox=QString("/var/spool/mail/")+QString(u);
	}
	fMailbox=new QLineEdit(this);
	fMailbox->setText(config->readEntry("UNIX Mailbox",defaultMailbox));
	fMailbox->resize(200,fMailbox->height());
	fMailbox->move(RIGHT(currentLabel),currentLabel->y());
	}
	fMailboxBrowse=new QPushButton(i18n("Browse"),this);
	fMailboxBrowse->adjustSize();
	fMailboxBrowse->move(RIGHT(fMailbox),fMailbox->y());
	connect(fMailboxBrowse,SIGNAL(clicked()),
		this,SLOT(browseMailbox()));
}

/* virtual */ int PopMailUNIXPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;
	config->writeEntry("UNIX Mailbox", fMailbox->text());
	return 0;
}


void PopMailUNIXPage::setEnabled(bool e)
{
	FUNCTIONSETUP;

	if (e)
	{
		fMailbox->setEnabled(true);
		fMailboxBrowse->setEnabled(true);
	}
	else
	{
		fMailbox->setEnabled(false);
		fMailboxBrowse->setEnabled(false);
	}
}

void PopMailUNIXPage::browseMailbox()
{
	FUNCTIONSETUP;

	QString filename=fMailbox->text();

	if (debug_level & UI_TEDIOUS)
	{
		kdDebug() << fname << ": Mailbox currently "
			<< fMailbox->text() << endl;
	}

	if (filename.isEmpty()) 
	{
		filename=QDir::currentDirPath();
	}
	else
	{
		filename=QFileInfo( filename ).dirPath();
	}

	filename = QFileDialog::getOpenFileName(filename,"*");

	if (debug_level & UI_MINOR)
	{
		kdDebug() << fname << ": Mailbox selected "
			<< filename << endl;
	}

	if (!filename.isEmpty())
	{
		fMailbox->setText(filename);
	}
}


PopMailReceivePage::PopMailReceivePage(setupDialog *parent,
	KConfig *config) :
	setupDialogPage(i18n("POP"),parent,config)
{
	FUNCTIONSETUP;
	QLabel *currentLabel;

	//-----------------------------------------------
	//
	// Receiving mail options.
	//

	currentLabel = new QLabel(i18n("POP Server:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, 10);
	fPopServer = new QLineEdit(this);
	fPopServer->setText(config->readEntry("PopServer", "pop"));
	fPopServer->resize(200, fPopServer->height());
	fPopServer->move(110, currentLabel->y()-4);

	currentLabel = new QLabel(i18n("POP Port:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fPopServer));
	fPopPort = new QLineEdit(this);
	fPopPort->setText(config->readEntry("PopPort", "110"));
	fPopPort->resize(200, fPopPort->height());
	fPopPort->move(110, currentLabel->y()-4);

	currentLabel = new QLabel(i18n("POP Username:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fPopPort));
	fPopUser = new QLineEdit(this);
	fPopUser->setText(config->readEntry("PopUser", "$USER"));
	fPopUser->resize(200, fPopUser->height());
	fPopUser->move(110, currentLabel->y()-4);

	fLeaveMail = new QCheckBox(i18n("&Leave mail on server."), this);
	fLeaveMail->adjustSize();
	fLeaveMail->setChecked(config->readNumEntry("LeaveMail", 1));
	fLeaveMail->move(110, BELOW(fPopUser));

	currentLabel = new QLabel(i18n("Pop Password:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fLeaveMail));
	fPopPass = new QLineEdit(this);
	fPopPass->setEchoMode(QLineEdit::Password);
	fPopPass->setText(config->readEntry("PopPass", ""));
	fPopPass->resize(200, fPopPass->height());
	fPopPass->move(110, currentLabel->y()-4);
	fPopPass->setEnabled(config->readNumEntry("StorePass", 0));



	fStorePass = new QCheckBox(i18n("Save &Pop password."), this);
	connect(fStorePass, SIGNAL(clicked()), this, SLOT(togglePopPass()));
	fStorePass->adjustSize();
	fStorePass->setChecked(config->readNumEntry("StorePass", 0));
	fStorePass->move(110, BELOW(fPopPass));

	currentLabel=new QLabel(i18n(
		"It is a really BAD idea to save your password unless\n"
		"you are sure the rest of your system security is perfect."),
		this);
	currentLabel->adjustSize();
	currentLabel->move(10,BELOW(fStorePass));

	togglePopPass();
}




/* virtual */ int PopMailReceivePage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;
	config->writeEntry("PopServer", fPopServer->text().latin1());
	config->writeEntry("PopPort", atoi(fPopPort->text().latin1()));
	config->writeEntry("PopUser", fPopUser->text().latin1());
	config->writeEntry("LeaveMail", (int)fLeaveMail->isChecked());
	config->writeEntry("StorePass", (int)fStorePass->isChecked());
	config->writeEntry("PopPass", fPopPass->text().latin1());
	config->sync(); 
	//
	// Make sure permissions are safe (still not a good idea)
	//
	if(fStorePass->isChecked()) 
	{
	    chmod(KGlobal::dirs()->findResource("config", "popmail-conduitrc").latin1(), 0600);
	}
	return 0;
}





void PopMailReceivePage::setEnabled(bool e)
{
	fStorePass->setEnabled(e);
	fPopServer->setEnabled(e);
	fPopPort->setEnabled(e);
	fPopUser->setEnabled(e);
	fLeaveMail->setEnabled(e);
	fStorePass->setEnabled(e);
	fPopPass->setEnabled(e);

	if (e)
	{
		togglePopPass();
	}
	else
	{
		fPopPass->setEnabled(e);
	}
}



void PopMailReceivePage::togglePopPass()
{
	FUNCTIONSETUP;

	if(fStorePass->isChecked())
	{
		fPopPass->setEnabled(true);
	}
	else
	{
		fPopPass->setEnabled(false);
	}
}



/* static */ const QString PopMailOptions::PopGroup("popmailOptions");

PopMailOptions::PopMailOptions(QWidget *parent) :
	setupDialog(parent, PopGroup,
		PopMailConduit::version())
{
	FUNCTIONSETUP;
	setupWidget();
	setupDialog::setupWidget();
}

PopMailOptions::~PopMailOptions()
{
	FUNCTIONSETUP;

}


  




void
PopMailOptions::setupWidget()
{
	FUNCTIONSETUP;

	KConfig* config = KPilotLink::getConfig();
	config->setGroup(PopGroup);


	pgeneral=new PopMailGeneralPage(this,config);
	psend=new PopMailSendPage(this,config);
	ppop=new PopMailReceivePage(this,config);
	punix=new PopMailUNIXPage(this,config);

	addPage(pgeneral);
	addPage(psend);
	addPage(ppop);
	addPage(punix);
	/*
	addPage(new setupInfoPage(this,
		    "Popmail conduit",
		    "By Michael Kropfberger, and\n"
		    "Dan Pilone, Adriaan de Groot"));
	*/
	addPage(new setupInfoPage(this));

	modeChangeReceive();
	modeChangeSend();
}

void PopMailOptions::modeChangeReceive()
{
	PopMailConduit::RetrievalMode  mode=pgeneral->getReceiveMethod();
	
	if (mode == PopMailConduit::NONE)
	{
		ppop->setEnabled(false);
		punix->setEnabled(false);
	}
	if (mode == PopMailConduit::POP)
	{
		punix->setEnabled(false);
		ppop->setEnabled(true);
	}
	if (mode == PopMailConduit::UNIXMailbox)
	{
		punix->setEnabled(true);
		ppop->setEnabled(false);
	}
}

void  PopMailOptions::modeChangeSend()
{
	psend->setMode(pgeneral->getSendMethod());
}
