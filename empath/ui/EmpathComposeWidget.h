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
# pragma interface "EmpathComposeWidget.h"
#endif

#ifndef EMPATHCOMPOSEWIDGET_H
#define EMPATHCOMPOSEWIDGET_H

// Qt includes
#include <qwidget.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombo.h>
#include <qdict.h>
#include <qdatetime.h>
#include <qfileinfo.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathURL.h"
#include "EmpathHeaderSpecWidget.h"
#include "EmpathComposer.h"
#include <RMM_Message.h>

class QMultiLineEdit;

class EmpathEnvelopeWidget;
class EmpathAttachmentListWidget;

/**
 * The container for the various widgets used when composing.
 */
class EmpathComposeWidget : public QWidget
{
    Q_OBJECT

    public:
        
        /**
         * Standard ctor
         */
        EmpathComposeWidget(
            const EmpathComposer::Form & composeForm,
            QWidget * parent = 0, const char * name = 0);

        /**
         * dtor
         */
        ~EmpathComposeWidget();

        /**
         * Return a composeform when the user wants to send the 
         * message.
         */
        EmpathComposer::Form & composeForm();
        
        /**
         * Test if there are any attachments for this message.
         */
        bool messageHasAttachments();
        
        // void bugReport();
        
        bool haveTo();
        bool haveSubject();
        
    protected slots:
        
        void    s_editorDone(bool ok, QCString text);
    
        void    s_cut();
        void    s_copy();
        void    s_paste();
        void    s_selectAll();
        
        void    s_addAttachment();
        void    s_editAttachment();
        void    s_removeAttachment();
        
    private:

        void    _spawnExternalEditor(const QCString & text);
        
        QCString _body();

        EmpathComposer::Form composeForm_;
       
        EmpathEnvelopeWidget        * envelopeWidget_;
        QMultiLineEdit              * editorWidget_;
        EmpathAttachmentListWidget  * attachmentWidget_; // Iconview or listview
        
        // QVBoxLayout      * headerLayout_;
        
        EmpathURL       url_;
        QString         recipient_;
        
        int             maxSizeColOne_;
};

#endif
// vim:ts=4:sw=4:tw=78
