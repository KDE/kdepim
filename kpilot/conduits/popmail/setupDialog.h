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

class PopMailSendPage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailSendPage(setupDialog *parent,KConfig *);
	virtual int commitChanges(KConfig *);

public slots:
	void browseSignature();

public:
	void setMode(PopMailConduit::SendMode m);

private:
	QLineEdit* fEmailFrom;
	QLineEdit *fHeaders;
	QLineEdit* fSignature;
	QPushButton *fSignatureBrowse;

	QLineEdit* fSendmailCmd;
	QLineEdit* fSMTPServer;
	QLineEdit* fSMTPPort;
} ;

class PopMailUNIXPage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailUNIXPage(setupDialog *,KConfig *);
	virtual int commitChanges(KConfig *);
	virtual void setEnabled(bool);

public slots:
	void browseMailbox();

private:
	QLineEdit *fMailbox;
	QPushButton *fMailboxBrowse;
} ;

class PopMailReceivePage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailReceivePage(setupDialog *parent,KConfig *);
	virtual int commitChanges(KConfig *);

	virtual void setEnabled(bool);

public slots:
	void togglePopPass();

private:
	QLineEdit* fPopServer;
	QLineEdit* fPopPort;
	QLineEdit* fPopUser;
	QCheckBox* fLeaveMail;
	QCheckBox* fSyncIncoming;
	QLineEdit *fPopPass;
	QCheckBox *fStorePass;
} ;

/* class PopMailPasswordPage : public setupDialogPage
{
	Q_OBJECT;

public:
	PopMailPasswordPage(setupDialog *parent,KConfig *);
        virtual int commitChanges(KConfig *);

public slots:
	void togglePopPass();

private:
	QCheckBox* fStorePass;
	QLineEdit* fPopPass;
} ;

*/

// Now methods for both send and receive
//
//
class PopMailGeneralPage : public setupDialogPage
{
	Q_OBJECT;

public:
	PopMailGeneralPage(setupDialog *parent,KConfig *);
        virtual int commitChanges(KConfig *);


private:

	QButtonGroup *methodGroup;
	QRadioButton *fNoMethod;
	QRadioButton *fPOPMethod,*fUNIXMethod;

	QButtonGroup *sendGroup;
	QRadioButton *fNoSend,*fSendmail,*fSMTP;

	void setReceiveMethod(PopMailConduit::RetrievalMode);
	void setSendMethod(PopMailConduit::SendMode);

public:
	PopMailConduit::SendMode getSendMethod() const;
	PopMailConduit::RetrievalMode getReceiveMethod() const;
} ;

class PopMailOptions : public setupDialog
{
	Q_OBJECT

friend class PopMailConduit;
public:
	PopMailOptions(QWidget *parent=0L);
	~PopMailOptions();
  
protected:
	static const QString PopGroup;

public slots:
	void modeChangeReceive();
	void modeChangeSend();

protected:
	virtual void setupWidget();

	// Remember the different pages because there
	// is some interaction between the general page
	// and the others.
	//
	//
	PopMailGeneralPage *pgeneral;
	PopMailSendPage *psend; 
	PopMailUNIXPage *punix;
	PopMailReceivePage *ppop;
};

#endif
