#ifndef _KPILOT_SYNCACTION_H
#define _KPILOT_SYNCACTION_H
/* syncAction.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>

#include <pi-dlp.h>


#include <qobject.h>
#include <qstring.h>

#include "kpilotlink.h"

class QTimer;
class QSocketNotifier;
class KPilotUser;
class SyncAction;

class SyncAction : public QObject
{
Q_OBJECT

public:
	SyncAction(KPilotDeviceLink *p,
		const char *name=0L);
	~SyncAction();

	typedef enum { Error=-1 } Status;

	int status() const { return fActionStatus; } ;
	virtual QString statusString() const;

protected:
	/**
	* This function starts the actual processing done
	* by the conduit. It should return false if the
	* processing cannot be initiated, f.ex. because
	* some parameters were not set or a needed library
	* is missing. This will be reported to the user.
	* It should return true if processing is started
	* normally. If processing starts normally, it is
	* the _conduit's_ responsibility to eventually
	* emit syncDone(); if processing does not start
	* normally (ie. exec() returns false) then the
	* environment will deal with syncDone().
	*/
	virtual bool exec() = 0;

public slots:
	/**
	* This just calls exec() and deals with the
	* return code.
	*/
	void execConduit();

signals:
	void syncDone(SyncAction *);
	void logMessage(const QString &);
	void logError(const QString &);
	void logProgress(const QString &,int);

	/**
	* It might not be safe to emit syncDone() from exec().
	* So instead, call delayDone() to wait for the main event
	* loop to return if you manage to do all processing
	* immediately.
	*
	* delayDone() returns true, so that return delayDone();
	* is a sensible final statement in exec().
	*/
protected slots:
	void delayedDoneSlot();

protected:
	bool delayDone();

public:
	void addSyncLogEntry(const QString &e,bool log=true)
		{ fHandle->addSyncLogEntry(e,log); } ;
	void addLogMessage( const QString &msg ) { emit logMessage( msg ); }
	void addLogError( const QString &msg ) { emit logError( msg ); }
	void addLogProgress( const QString &msg, int prog ) { emit logProgress( msg, prog ); }
protected:
	KPilotDeviceLink *fHandle;
	int fActionStatus;

	int pilotSocket() const { return fHandle->pilotSocket(); } ;

	int openConduit() { return fHandle->openConduit(); } ;
public:
	/**
	* These are the different syncs that we can do.
	*/
	enum SyncMode
	{
		eDefaultSync=0,
		eFastSync=1,
		eHotSync=2,
		eFullSync=3,
		eCopyPCToHH=4,
		eCopyHHToPC=5,
		eBackup=6,
		eRestore=7,
		eTest=8,
		eLastMode=eTest,
		eLastUserMode=eRestore
	};

	/**
	* Returns a standard name for each of the syncs.
	*/
	static QString syncModeName(SyncMode);

	enum ConflictResolution
	{
		eUseGlobalSetting=-1,
		eAskUser=0,
		eDoNothing,
		eHHOverrides,
		ePCOverrides,
		ePreviousSyncOverrides,
		eDuplicate,
		eDelete,
		eCROffset=-1
	};

protected:
	QTimer *fTickleTimer;
	unsigned fTickleCount,fTickleTimeout;

	/**
	* Call startTickle() some time before showing a dialog to the
	* user (we're assuming a local event loop here) so that while
	* the dialog is up and the user is thinking, the pilot stays
	* awake. Afterwards, call stopTickle().
	*
	* The parameter to startTickle indicates the timeout, in
	* seconds, before signal timeout is emitted. You can connect
	* to that, again, to take down the user interface part if the
	* user isn't reacting.
	*/
	void startTickle(unsigned count=0);
	void stopTickle();
protected slots:
	/**
	* Called whenever the tickle state is on, uses dlp_tickle()
	* or something like that to prevent the pilot from timing out.
	*/
	void tickle();
signals:
	void timeout();
};



class InteractiveAction : public SyncAction
{
Q_OBJECT
public:
	// Note that this takes a QWidget as additional parameter,
	// so that it can display windows relative to that if necessary.
	//
	//
	InteractiveAction(KPilotDeviceLink *p,
		QWidget *visibleparent=0L,
		const char *name=0L);
	virtual ~InteractiveAction();

	// Reminder that the whole point of the class is to implement
	// the pure virtual function exec().
	//
	// virtual void exec()=0;


protected:
	QWidget *fParent;

	/**
	* Ask a yes-no question of the user. This has a timeout so that
	* you don't wait forever for inattentive users. It's much like
	* KMessageBox::questionYesNo(), but with this extra timeout-on-
	* no-answer feature. Returns a KDialogBase::ButtonCode value - Yes,No or
	* Cancel on timeout. If there is a key set and the user indicates not to ask again,
	* the selected answer (Yes or No) is remembered for future reference.
	*
	* @p caption Message Box caption, uses "Question" if null.
	* @p key     Key for the "Don't ask again" code.
	* @p timeout Timeout, in ms.
	*/
	int questionYesNo(const QString &question ,
		const QString &caption = QString::null,
		const QString &key = QString::null,
		unsigned timeout = 20000);
} ;

#endif
