/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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
# pragma interface "EmpathMainWindow.h"
#endif

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
#include <RMM_Message.h>

class EmpathSettingsDialog;
class EmpathFolderWidget;
class EmpathMessageListWidget;
class EmpathMessageWidget;
class EmpathComposeWindow;
class EmpathAboutBox;
class EmpathMainWidget;

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

        EmpathMessageListWidget    * messageListWidget();

    protected slots:

        void s_toolbarMoved(BarPosition);
        
        // File menu slots
        void s_fileSendNew();
        void s_fileAddressBook();
        void s_fileQuit();
        
        // Edit menu slots
    
        void s_editSelectTagged();
        void s_editSelectRead();
        void s_editSelectAll();
        void s_editInvertSelection();
        
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

        // Help menu slots
        void s_help();
        void s_aboutQt();
        
        // Debugging
        void s_dumpWidgetList();
        
    protected:
        
        void closeEvent(QCloseEvent *);

    private:
    
        // General
        KMenuBar        * menu_;
        KStatusBar        * status_;
        
        QPopupMenu        * fileMenu_;
        QPopupMenu        * editMenu_;
        QPopupMenu        * folderMenu_;
        QPopupMenu        * messageMenu_;
        QPopupMenu        * optionsMenu_;
        QPopupMenu        * helpMenu_;

        // Empath stuff

        EmpathFolderWidget            * folderWidget_;
        EmpathMessageListWidget        * messageListWidget_;
        EmpathMessageWidget            * messageWidget_;
        EmpathMainWidget            * mainWidget_;
        
        EmpathSettingsDialog        * settingsDialog_;
        EmpathComposeWindow            * composeWindow_;
        EmpathAboutBox                * aboutBox_;

        // Setup methods
        void _setupMenuBar();
        void _setupToolBar();
        void _setupStatusBar();

        bool queryExit();
        bool _messageSelected();
        
        RMM::RMessage * _getFirstSelectedMessage() const;
};

#endif
// vim:ts=4:sw=4:tw=78
