// setupDialog.cc
//
// Copyright (C) 1998,1999 Dan Pilone
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
#include <kmsgbox.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qdir.h>
#include <qfiledlg.h>
#include <qbttngrp.h>
#include <qradiobt.h>


#include <stream.h>
#include "popmail-conduit.h"
#include "setupDialog.moc"
#include "kpilot.h"

PopMailSendPage::PopMailSendPage(setupDialog *parent,KConfig *config) :
	setupDialogPage(parent,config)
{
	FUNCTIONSETUP;
	QLabel *currentLabel;

	//-----------------------------------------------
	//
	// Sending mail options.
	//
	fSendOutgoing = new QCheckBox(
		klocale->translate("Send &Outgoing mail."), this);
	fSendOutgoing->adjustSize();
	fSendOutgoing->setChecked(config->readNumEntry("SendOutgoing", 1));
	fSendOutgoing->move(110, 10);



	currentLabel = new QLabel(klocale->translate("Email Address: "), 
			    this);
	currentLabel->adjustSize();
	currentLabel->move(10, BELOW(fSendOutgoing));
	fEmailFrom = new QLineEdit(this);
	fEmailFrom->setText(config->readEntry("EmailAddress", "$USER"));
	fEmailFrom->resize(200, fEmailFrom->height());
	fEmailFrom->move(110,currentLabel->y()-4);

	currentLabel = new QLabel(klocale->translate("Signature File: "), 
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

	currentLabel = new QLabel(klocale->translate("Sendmail Cmd:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, fSignature->y()+fSignature->height()+SPACING );
	fSendmailCmd = new QLineEdit(this);
	fSendmailCmd->setText(config->readEntry("SendmailCmd", "/usr/lib/sendmail -t -i"));
	fSendmailCmd->resize(200, fSignature->height());
	fSendmailCmd->move(110, currentLabel->y()-4 );

	currentLabel = new QLabel(klocale->translate("SMTP Server:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, fSendmailCmd->y()+fSendmailCmd->height()+SPACING);
	fSMTPServer = new QLineEdit(this);
	fSMTPServer->setText(config->readEntry("SMTPServer", "mail"));
	fSMTPServer->resize(200, fSignature->height());
	fSMTPServer->move(110, currentLabel->y()-4);

	currentLabel = new QLabel(klocale->translate("SMTP Port:"), this);
	currentLabel->adjustSize();
	currentLabel->move(10, fSMTPServer->y()+fSMTPServer->height()+SPACING);
	fSMTPPort = new QLineEdit(this);
	fSMTPPort->setText(config->readEntry("SMTPPort", "25"));
	fSMTPPort->resize(200, fSignature->height());
	fSMTPPort->move(110, currentLabel->y()-4);


	fUseSMTP = new QCheckBox(
		klocale->translate("Use a &SMTP Server."), this);
	connect(fUseSMTP, SIGNAL(clicked()), this, SLOT(toggleUseSMTP()));
	fUseSMTP->adjustSize();
	fUseSMTP->setChecked(config->readNumEntry("UseSMTP", 0));
	fUseSMTP->move(110, BELOW(fSMTPPort));


	toggleUseSMTP();
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
	config->writeEntry("UseSMTP", (int)fUseSMTP->isChecked());
	config->writeEntry("SendOutgoing", (int)fSendOutgoing->isChecked());

	return 0;
}

/* virtual */ const char *PopMailSendPage::tabName()
{
	return i18n("Sending mail");
}






void PopMailSendPage::browseSignature()
{
	FUNCTIONSETUP;

	QString filename=fSignature->text();

	if (debug_level>UI_ACTIONS)
	{
		cerr << fname << ": Signature currently "
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

	if (debug_level>UI_ACTIONS)
	{
		cerr << fname << ": Signature selected "
			<< filename << endl;
	}

	if (!filename.isEmpty())
	{
		fSignature->setText(filename);
	}
}


void PopMailSendPage::toggleUseSMTP()
{
	FUNCTIONSETUP;

	if(fUseSMTP->isChecked()) 
	{
		fSendmailCmd->setEnabled(false);
		fSMTPServer->setEnabled(true);
		fSMTPPort->setEnabled(true);
	} 
	else 
	{
		fSendmailCmd->setEnabled(true);
		fSMTPServer->setEnabled(false);
		fSMTPPort->setEnabled(false);
	}
}




PopMailReceiveMethodPage::PopMailReceiveMethodPage(setupDialog *parent,
	KConfig *config) :
	setupDialogPage(parent,config)
{
	FUNCTIONSETUP;


	methodGroup=new QButtonGroup(i18n("Retrieve Method"),
		this,"bg");
	methodGroup->move(10,10);

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

	toggleMethod((PopMailConduit::RetrievalMode) 
		(config->readNumEntry("SyncIncoming",PopMailConduit::NONE)));
}

void PopMailReceiveMethodPage::toggleMethod(
	PopMailConduit::RetrievalMode method)
{
	FUNCTIONSETUP;

	fNoMethod->setChecked(method==PopMailConduit::NONE);
	fPOPMethod->setChecked(method==PopMailConduit::POP);
	fUNIXMethod->setChecked(method==PopMailConduit::UNIXMailbox);
}

/* virtual */ int PopMailReceiveMethodPage::commitChanges(KConfig *config)
{
	FUNCTIONSETUP;

	PopMailConduit::RetrievalMode method=PopMailConduit::NONE;

	if (fPOPMethod->isChecked())
	{
		method=PopMailConduit::POP;
	}
	if (fUNIXMethod->isChecked())
	{
		method=PopMailConduit::UNIXMailbox;
	}

	config->writeEntry("SyncIncoming", (int)method);

	return 0;
}

/* virtual */ const char *PopMailReceiveMethodPage::tabName()
{
	return i18n("Receiving Mail");
}


PopMailReceivePage::PopMailReceivePage(setupDialog *parent,
	KConfig *config) :
	setupDialogPage(parent,config)
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
	config->writeEntry("PopServer", fPopServer->text());
	config->writeEntry("PopPort", atoi(fPopPort->text()));
	config->writeEntry("PopUser", fPopUser->text());
	config->writeEntry("LeaveMail", (int)fLeaveMail->isChecked());
	config->writeEntry("StorePass", (int)fStorePass->isChecked());
	config->writeEntry("PopPass", fPopPass->text());
	config->sync(); 
	//
	// Make sure permissions are safe (still not a good idea)
	//
	if(fStorePass->isChecked()) 
	{
		chmod(kapp->localconfigdir() + "/popmail-conduitrc", 0600);
	}
	return 0;
}

/* virtual */ const char *PopMailReceivePage::tabName()
{
	return i18n("POP Mail");
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



/* virtual */ const char *PopMailOptions::groupName()
{
	return configGroup();
}

/* static */ const char *PopMailOptions::configGroup()
{
	return "Email Conduit";
}

PopMailOptions::PopMailOptions(QWidget *parent) :
	setupDialog(parent, "popmailOptions",
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

	KConfig* config = kapp->getConfig();
	config->setGroup(groupName());


	addPage(new PopMailSendPage(this,config));
	addPage(new PopMailReceiveMethodPage(this,config));
	addPage(new PopMailReceivePage(this,config));
	/* addPage(new PopMailPasswordPage(this,config)); */
	addPage(new setupInfoPage(this,
		PopMailConduit::version(),
		"By Michael Kropfberger, and\n"
		"Dan Pilone, Adriaan de Groot"));
}
