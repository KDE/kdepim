/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
#include <qwidgetstack.h>
#include <qpopupmenu.h>

// KDE includes
#include <ktmainwindow.h>
#include <ktoolbar.h>
#include <kprogress.h>
#include <kapp.h>
#include <kaction.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_Message.h>

class EmpathFolderWidget;
class EmpathMessageListWidget;
class EmpathMessageWidget;
class EmpathMainWidget;

enum BarPosition {};
class EmpathTask;

class EmpathProgressIndicator : public QWidget
{
    Q_OBJECT

    public:

        EmpathProgressIndicator(EmpathTask *, QWidgetStack * parent);
        virtual ~EmpathProgressIndicator();

    protected slots:

        void s_setMaxValue(int);
        void s_incValue();
        void s_finished();

    private:
        
        KProgress * progress_;
};

class EmpathMainWindow : public KTMainWindow
{
    Q_OBJECT

    public:
        
        EmpathMainWindow();
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
        void s_fileClose();
        
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

        // Configure menu slots
        void s_setupDisplay();
        void s_setupIdentity();
        void s_setupComposing();
        void s_setupSending();
        void s_setupAccounts();
        void s_setupFilters();

        // Help menu slots
        void s_help();
        void s_about();
        void s_aboutQt();
        
        void s_newTask(EmpathTask * t);

        void s_setHideReadChecked(bool);

    private:
    
        // General
        KMenuBar        * menu_;
        
        QPopupMenu      * selectMenu_;
        QPopupMenu      * goMenu_;
        QPopupMenu      * folderMenu_;
        QPopupMenu      * messageMenu_;
        QPopupMenu      * threadMenu_;
        QPopupMenu      * optionsMenu_;
        QPopupMenu      * helpMenu_;

        // Empath stuff

        EmpathMessageListWidget * messageListWidget_;
        EmpathMainWidget        * mainWidget_;
        
        // Setup methods
        void _setupMenuBar();
        void _setupToolBar();
        void _setupStatusBar();

        bool _messageSelected();

        int hideReadIndex_;

        QWidgetStack * progressStack_;
};

#endif
// vim:ts=4:sw=4:tw=78
