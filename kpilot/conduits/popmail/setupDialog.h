// setupDialog.h
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


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
