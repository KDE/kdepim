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
# pragma interface "EmpathMailboxMaildir.h"
#endif

#ifndef EMPATHMAILBOXMAILDIR_H
#define EMPATHMAILBOXMAILDIR_H

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qlist.h>

// Local includes
#include "RMM_Message.h"
#include "EmpathDefines.h"
#include "EmpathMaildir.h"
#include "EmpathMailbox.h"

class EmpathMailboxMaildir : public EmpathMailbox
{
	Q_OBJECT
	
	public:

		EmpathMailboxMaildir(const QString & name);
		QString writeNewMail(RMM::RMessage & message);
		
		virtual ~EmpathMailboxMaildir();
	
		void setPath(const QString & path);
		const QString & path() const { return path_; }
		
#include "EmpathMailboxAbstract.h"

	private:
	
		bool					_recursiveRemove(const QString &);
		void					_recursiveReadFolders(const QString &);
		EmpathMaildir *			_box(const EmpathURL & id);
		void					_setupDefaultFolders();
		
		QString					path_;
		QString					canonName_;
		EmpathMaildirList		boxList_;
};

#endif

