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
	virtual const char *tabName();

public slots:
	void browseSignature();
	void toggleUseSMTP();

private:
	QLineEdit* fEmailFrom;
	QLineEdit *fHeaders;
	QLineEdit* fSignature;
	QPushButton *fSignatureBrowse;
	QLineEdit* fSendmailCmd;
	QLineEdit* fSMTPServer;
	QLineEdit* fSMTPPort;
	QCheckBox* fUseSMTP;
	QCheckBox* fSendOutgoing;
} ;

class PopMailReceivePage : public setupDialogPage
{
	Q_OBJECT

public:
	PopMailReceivePage(setupDialog *parent,KConfig *);
	virtual int commitChanges(KConfig *);
	virtual const char *tabName();

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
	virtual const char *tabName();

public slots:
	void togglePopPass();

private:
	QCheckBox* fStorePass;
	QLineEdit* fPopPass;
} ;

*/

class PopMailReceiveMethodPage : public setupDialogPage
{
	Q_OBJECT;

public:
	PopMailReceiveMethodPage(setupDialog *parent,KConfig *);
        virtual int commitChanges(KConfig *);
	virtual const char *tabName();


private:
	QButtonGroup *methodGroup;
	QRadioButton *fNoMethod;
	QRadioButton *fPOPMethod,*fUNIXMethod;

	void toggleMethod(PopMailConduit::RetrievalMode);

} ;

class PopMailOptions : public setupDialog
{
	Q_OBJECT

public:
	PopMailOptions(QWidget *parent=0L);
	~PopMailOptions();
  
	static const char *configGroup();
	virtual const char *groupName();

public slots:


protected:
	virtual void setupWidget();
};

#endif
