#ifndef _KPILOT_SYNCACTION_H
#define _KPILOT_SYNCACTION_H
/* syncAction.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


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

	typedef enum { Error=-1 } Status;

	int status() const { return fStatus; } ;
	virtual QString statusString() const;

public slots:
	virtual void exec() = 0;

signals:
	void syncDone(SyncAction *);
	void logMessage(const QString &);
	void logError(const QString &);
	void logProgress(const QString &,int);

protected:
	KPilotDeviceLink *fHandle;
	int fStatus;

	void addSyncLogEntry(const QString &e,bool suppress=false) 
		{ fHandle->addSyncLogEntry(e,suppress); } ;
	int pilotSocket() const { return fHandle->pilotSocket(); } ;
} ;



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

protected slots:
	/**
	* Called whenever the tickle state is on, uses dlp_tickle()
	* or something like that to prevent the pilot from timing out.
	*/
	void tickle();

signals:
	void timeout();

protected:
	QWidget *fParent;
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

	/**
	* Ask a yes-no question of the user. This has a timeout so that
	* you don't wait forever for inattentive users. It's much like
	* KMessageBox::questionYesNo(), but with this extra timeout-on-
	* no-answer feature. Returns a KDialogBase value - Yes,No or
	* Cancel on timeout.
	*
	* @p caption Message Box caption, uses "Question" if null.
	* @p timeout Timeout, in ms.
	*/
	int questionYesNo(const QString &question ,
		const QString &caption = QString::null,
		unsigned timeout = 20000);
} ;


// $Log$

#endif
