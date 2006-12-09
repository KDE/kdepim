#ifndef _KPILOT_NOTEPAD_CONDUIT_H
#define _KPILOT_NOTEPAD_CONDUIT_H
/* notepad-conduit.h			KPilot
**
** Copyright (C) 2004 by Adriaan de Groot, Joern Ahrens, Angus Ainslie
**
** The code for NotepadActionThread::unpackNotePad was taken from
** Angus Ainslies read-notepad.c, which is part of pilot-link.
** NotepadActionThread::saveImage is also based on read-notepad.c.
**
** This file is part of the Notepad conduit, a conduit for KPilot that
** store the notepad drawings to files.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"

#include <qthread.h>
struct NotePad;
class NotepadActionThread;

class NotepadConduit : public ConduitAction
{
public:
	NotepadConduit(KPilotLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~NotepadConduit();
	virtual bool event(QEvent *e);

protected:
	virtual bool exec();           // From ConduitAction

private:
	NotepadActionThread *thread;
};


/**
 * This class saves the notepads to disk
 */
class NotepadActionThread : public QThread
{
public:
	NotepadActionThread(QObject *parent, KPilotLink *link);

	virtual void run();
	int	getFailed() { return notSaved; }
	int getSaved() { return saved; }

private:
	QObject *fParent;
	KPilotLink *fLink;

	/**
	 * counts how many notepads couldn't be saved during the sync
	 */
	int notSaved;
	/**
	 * counts how many files a saved during the sync
	 */
	int saved;

	int unpackNotePad(struct NotePad *a, unsigned char *buffer, int len);

	/**
	* Saves a single NotePad structure to disk, using the name in
	* the Note @p n, or if no name is specified, using the
	* timestamp in the note.
	*/
	void saveImage(struct NotePad *n);
};

#endif
