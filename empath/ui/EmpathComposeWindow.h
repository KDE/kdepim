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
# pragma interface "EmpathComposeWindow.h"
#endif

#ifndef EMPATHCOMPOSEWINDOW_H
#define EMPATHCOMPOSEWINDOW_H

// Qt includes
#include <qpopupmenu.h>

// KDE includes
#include <ktmainwindow.h>
#include <kapp.h>
#include <kstdaccel.h>

// Local includes
#include "EmpathDefines.h"
#include "Empath.h"
#include "EmpathURL.h"

class EmpathComposeWidget;

/**
 * Holds a compose widget.
 */
class EmpathComposeWindow : public KTMainWindow
{
    Q_OBJECT

    public:
        
        EmpathComposeWindow(EmpathComposer::Form);
        ~EmpathComposeWindow();
        
    protected slots:

        // File menu slots
        void s_fileSendMessage();
        void s_fileSendLater();
        void s_fileSaveAs();
        void s_filePrint();
        void s_fileClose();
        
        // Edit menu slots
    
        void s_editUndo();
        void s_editRedo();
        void s_editCut();
        void s_editCopy();
        void s_editPaste();
        void s_editDelete();
        void s_editSelectAll();
        void s_editFindInMessage();
        void s_editFind();
        void s_editFindAgain();
        
        // Message menu slots
        void s_messageSaveAs();
        void s_messageCopyTo();

        // Help menu slots
        void s_help();

        void s_confirmDelivery  (bool);
        void s_confirmReading   (bool);
        void s_addSignature     (bool);
        void s_digitallySign    (bool);
        void s_encrypt          (bool);
        
    signals:
        
        void cut();
        void copy();
        void paste();
        void selectAll();

    private:
        
        void _init();
        void _askForRecipient();
        void _askForSubject();
    
        QPopupMenu  * fileMenu_;
        QPopupMenu  * editMenu_;
        QPopupMenu  * headersMenu_;
        QPopupMenu  * attachmentMenu_;
        QPopupMenu  * messageMenu_;
        QPopupMenu  * priorityMenu_;
        QPopupMenu  * helpMenu_;

        // Empath stuff

        EmpathComposeWidget * composeWidget_;
        
        // Setup methods
        void setupMenuBar();
        void setupToolBar();
        void setupStatusBar();
        
        int id_confirmDelivery_;
        int id_confirmReading_;
        int id_addSignature_;
        int id_digitallySign_;
        int id_encrypt_;
};

#endif
// vim:ts=4:sw=4:tw=78
