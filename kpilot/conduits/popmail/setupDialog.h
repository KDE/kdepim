#ifndef _POPMAIL_SETUPDIALOG_H
#define _POPMAIL_SETUPDIALOG_H
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qwidget.h>
#include "plugin.h"
class KConfig;
class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QRadioButton;
class QButtonGroup;


#if 0
// A standard dialog page with all the
// settings used when sending mail, both
// with SMTP and sendmail (in future via KMail
// as well?)
//
//
class PopMailSendPage : public QWidget
{
// Q_OBJECT

public:
	PopMailSendPage(QWidget *parent );
	virtual int commitChanges(KConfig& );
	void readSettings(KConfig &);

public /* slots */:
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
	typedef enum { SEND_NONE=0,
		SEND_SENDMAIL=7,
		SEND_KMAIL=8,
		SEND_SMTP=12
		} SendMode ;

	void setMode(SendMode m);
	SendMode getMode() const { return fMode; };

private:
	SendMode fMode;

	QButtonGroup *sendGroup;
	QRadioButton *fNoSend,*fSendmail,*fSMTP, *fKMail;

	QLineEdit* fEmailFrom;
	QLineEdit *fHeaders;
	QLineEdit* fSignature;
	QPushButton *fSignatureBrowse;

	QLineEdit* fSendmailCmd;
	QLineEdit* fSMTPServer;
	QLineEdit* fSMTPPort;
	QLineEdit* fFirewallFQDN;
	QCheckBox *fKMailSendImmediate;
} ;

// A standard dialog page used when receiving
// mail, both via POP and via UNIX mailboxes
// (and in future from KMail?)
//
//
class PopMailReceivePage : public QWidget
{
// Q_OBJECT

public:
	PopMailReceivePage(QWidget *);
	virtual int commitChanges(KConfig& );
	void readSettings(KConfig &);

public /* slots */:
	void browseMailbox();
	void togglePopPass();
	void toggleMode();

public:
	typedef enum {
		RECV_NONE=0,
		RECV_POP=1,
		RECV_UNIX=2
		} RetrievalMode ;

	void setMode(RetrievalMode m);
	RetrievalMode getMode() const { return fMode; };

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

	RetrievalMode fMode;
} ;
#endif


class PopMailWidget; // From setup-dialog.ui

class PopMailWidgetConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	PopMailWidgetConfig(QWidget *, const char *);
	virtual void load();
	virtual void commit();

	static ConduitConfigBase *create(QWidget *w, const char *n)
		{ return new PopMailWidgetConfig(w,n); } ;

	// These enums must follow the order of items in the combo box
	enum RecvMode { NoRecv=0, RecvPOP=1, RecvMBOX=2 } ;
	enum SendMode { NoSend=0, SendSendmail=1, SendSMTP=2, SendKMail=3 } ;

protected:
	PopMailWidget *fConfigWidget;

public slots:
	void toggleRecvMode(int);
	void toggleSendMode(int);

signals:
	void moose();
} ;


#endif
