/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma interface "EmpathFilterEventHandler.h"
#endif

#ifndef EMPATHFILTEREVENTHANDER_H
#define EMPATHFILTEREVENTHANDER_H

// Qt includes
#include <qobject.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathURL.h"

class EmpathFilterEventHandler : public QObject
{
	Q_OBJECT

	public:
		
		enum ActionType { MoveFolder, CopyFolder, Delete, Ignore, Forward };
		
		EmpathFilterEventHandler();
		
		virtual ~EmpathFilterEventHandler();

		void setMoveFolder(const EmpathURL &);
		void setCopyFolder(const EmpathURL &);
		void setDelete();
		void setIgnore();
		void setForward(const QString &);

		void handleMessage	(const EmpathURL &);
		
		bool load			(const QString &);
		void save			(const QString &);

		QString		description()		const;
		ActionType	actionType()		const;
		EmpathURL	moveOrCopyFolder()	const;
		QString		forwardAddress()	const;

	private:

		QString		forwardAddress_;
		EmpathURL	moveCopyFolder_;
		ActionType	actionType_;
};

#endif // EMPATHFILTEREVENTHANDLER_H

