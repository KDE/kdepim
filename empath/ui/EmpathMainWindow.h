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

#ifndef EMPATHMAINWINDOW_H
#define EMPATHMAINWINDOW_H

// Qt includes
#include <qpopupmenu.h>

// KDE includes
#include <ktmainwindow.h>
#include <ktoolbar.h>
#include <kstdaccel.h>
#include <kapp.h>

// Local includes
#include "EmpathDefines.h"

class EmpathSettingsDialog;
class EmpathFolderWidget;
class EmpathMessageListWidget;
class EmpathMessageWidget;
class EmpathComposeWindow;
class EmpathAboutBox;
class EmpathStatusWidget;
class EmpathMainWidget;
class RMessage;

enum BarPosition {};

class EmpathMainWindow : public KTMainWindow
{
	Q_OBJECT

	public:
		
		EmpathMainWindow(const char * name);
		~EmpathMainWindow();

		void statusMessage(const QString & messageText, int seconds);
		void clearStatusMessage();
	
		void newMailArrived();

		EmpathStatusWidget		* statusWidget();
		EmpathMessageListWidget	* messageListWidget();

	protected slots:

		void s_toolbarMoved(BarPosition);
		
		// File menu slots
		void s_fileNewMessage();
		void s_fileNewFolder();
		void s_fileViewMessage();
		void s_fileEmptyTrash();
		void s_fileGetNew();
		void s_fileSendNew();
		void s_fileSettings();
		void s_fileAddressBook();
		void s_filePrint();
		void s_fileQuit();
		
		// Edit menu slots
	
		void s_editUndo();
		void s_editRedo();
		void s_editCut();
		void s_editCopy();
		void s_editPaste();
		void s_editDelete();
		void s_editSelect();
		void s_editFindInMessage();
		void s_editFind();
		void s_editFindAgain();
		
		// Folder menu slots
		void s_folderNew();
		void s_folderEdit();
		void s_folderClear();
		void s_folderDelete();

		// Message menu slots
		void s_messageNew();
		void s_messageReply();
		void s_messageReplyAll();
		void s_messageForward();
		void s_messageBounce();
		void s_messageDelete();
		void s_messageSaveAs();
		void s_messageCopyTo();
		void s_messageMoveTo();
		void s_messagePrint();
		void s_messageFilter();
		void s_messageView();
		void s_messageViewSource();

		// Options menu slots
		void s_optionsSettingsDisplay();
		void s_optionsSettingsIdentity();
		void s_optionsSettingsCompose();
		void s_optionsSettingsSending();
		void s_optionsSettingsAccounts();
		void s_optionsSettingsFilters();

		// Help menu slots
		void s_help();
		void s_aboutEmpath();
		void s_aboutQt();
		
		// Debugging
		void s_dumpWidgetList();

	private:
	
		// General
		KStdAccel		* accel;

		KMenuBar		* menu;
		KToolBar		* tool;
		KStatusBar		* status;
		
		KConfig			* config;
	
		QPopupMenu		* fileMenu_;
		QPopupMenu		* editMenu_;
		QPopupMenu		* folderMenu_;
		QPopupMenu		* messageMenu_;
		QPopupMenu		* optionsMenu_;
		QPopupMenu		* helpMenu_;

		// Empath stuff

		EmpathFolderWidget			* folderWidget_;
		EmpathMessageListWidget		* messageListWidget_;
		EmpathMessageWidget			* messageWidget_;
		EmpathMainWidget			* mainWidget_;
		
		EmpathSettingsDialog		* settingsDialog_;
		EmpathComposeWindow			* composeWindow_;
		EmpathAboutBox				* aboutBox_;
		EmpathStatusWidget			* statusWidget_;

		// Setup methods
		void setupMenuBar();

		void setupToolBar();

		void setupStatusBar();

		// Other methods

		bool queryExit();
		
		RMessage * _getFirstSelectedMessage() const;
};

#endif
