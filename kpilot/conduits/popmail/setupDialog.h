/* setupDialog.h			KPilot
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef __POPMAIL_SETUP_H
#define __POPMAIL_SETUP_H

#include "gsetupDialog.h"

class KConfig;
class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QRadioButton;
class QButtonGroup;

// A standard dialog page with all the
// settings used when sending mail, both
// with SMTP and sendmail (in future via KMail
// as well?)
//
//
class PopMailSendPage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailSendPage(setupDialog *parent,KConfig& );
	virtual int commitChanges(KConfig& );

public slots:
	/**
	* Called to browse for a signature file.
	*/
	void browseSignature();
	/**
	* Called when the user changes the mode
	* through the radio buttons. This enables /
	* disables the relevant fields.
	*/
	void toggleMode();

public:
	void setMode(PopMailConduit::SendMode m);
	PopMailConduit::SendMode getMode() const { return fMode; };

private:
	PopMailConduit::SendMode fMode;

	QButtonGroup *sendGroup;
	QRadioButton *fNoSend,*fSendmail,*fSMTP;

	QLineEdit* fEmailFrom;
	QLineEdit *fHeaders;
	QLineEdit* fSignature;
	QPushButton *fSignatureBrowse;

	QLineEdit* fSendmailCmd;
	QLineEdit* fSMTPServer;
	QLineEdit* fSMTPPort;
} ;

// A standard dialog page used when receiving
// mail, both via POP and via UNIX mailboxes
// (and in future from KMail?)
//
//
class PopMailReceivePage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailReceivePage(setupDialog *,KConfig& );
	virtual int commitChanges(KConfig& );

public slots:
	void browseMailbox();
	void togglePopPass();
	void toggleMode();

public:
	void setMode(PopMailConduit::RetrievalMode m);
	PopMailConduit::RetrievalMode getMode() const { return fMode; };

private:
	QButtonGroup *methodGroup;
	QRadioButton *fNoReceive;
	QRadioButton *fReceivePOP,*fReceiveUNIX;

	QLineEdit *fMailbox;
	QPushButton *fMailboxBrowse;

	QLineEdit* fPopServer;
	QLineEdit* fPopPort;
	QLineEdit* fPopUser;
	QCheckBox* fLeaveMail;
	QCheckBox* fSyncIncoming;
	QLineEdit *fPopPass;
	QCheckBox *fStorePass;

	PopMailConduit::RetrievalMode fMode;
} ;



class PopMailOptions : public setupDialog
{
	Q_OBJECT

friend class PopMailConduit;	// For getting the PopGroup
public:
	PopMailOptions(QWidget *parent=0L);
	~PopMailOptions();
  
protected:
	static const QString PopGroup;

protected:
	virtual void setupWidget();
} ;

#endif


// $Log:$
