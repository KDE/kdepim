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
# pragma interface "EmpathMessageViewWindow.h"
#endif

#ifndef EMPATHMESSAGEVIEWWINDOW_H
#define EMPATHMESSAGEVIEWWINDOW_H

// Qt includes
#include <qpopupmenu.h>
#include <qlayout.h>

// KDE includes
#include <ktmainwindow.h>
#include <kstdaccel.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathMessageViewWidget;

class EmpathMessageViewWindow : public KTMainWindow
{
	Q_OBJECT

	public:
		
		EmpathMessageViewWindow(const EmpathURL & url, const char * name = 0L);
		~EmpathMessageViewWindow();

	protected slots:

		void s_fileClose();
		void s_editCopy();
		void s_editFindInMessage();
		void s_editFind();
		void s_editFindAgain();
		void s_messageNew();
		void s_messageReply();
		void s_messageReplyAll();
		void s_messageSaveAs();
		void s_messageCopyTo();
		void s_messageForward();
		void s_messageBounce();
		void s_messageDelete();
		void s_messagePrint();
		void s_aboutQt();

	private:
		
		EmpathMessageViewWidget * messageView_;
		
//		KStdAccel		* accel;

		KConfig			* config_;
		QPopupMenu		* fileMenu_;
		QPopupMenu		* editMenu_;
		QPopupMenu		* messageMenu_;
		QPopupMenu		* helpMenu_;

		// Setup methods
		void setupMenuBar();

		void setupToolBar();

		void setupStatusBar();

		EmpathURL		url_;
};

#endif
