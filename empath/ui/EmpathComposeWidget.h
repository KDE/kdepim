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
# pragma interface "EmpathComposeWidget.h"
#endif

#ifndef EMPATHCOMPOSEWIDGET_H
#define EMPATHCOMPOSEWIDGET_H

// Qt includes
#include <qwidget.h>
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
class QSplitter;
class QActionCollection;

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
        EmpathComposeWidget(EmpathComposeForm, QWidget * parent);

        /**
         * dtor
         */
        ~EmpathComposeWidget();

        /**
         * Return a composeform when the user wants to send the 
         * message.
         */
        EmpathComposeForm composeForm();
        
        /**
         * Test if there are any attachments for this message.
         */
        bool messageHasAttachments();
        
        // void bugReport();
        
        bool haveTo();
        bool haveSubject();

        QActionCollection * actionCollection() { return actionCollection_; }
        
    protected slots:
        
        void    s_editorDone(bool ok, QCString text);
    
        void    s_undo();
        void    s_redo();
        void    s_cut();
        void    s_copy();
        void    s_paste();
        void    s_selectAll();

/*        
        void    s_addAttachment();
        void    s_editAttachment();
        void    s_removeAttachment();
*/
        
    private:

        void _initActions();
        
        void _spawnExternalEditor(const QCString & text);
        
        QCString _body();

        EmpathComposeForm composeForm_;
       
        EmpathEnvelopeWidget        * envelopeWidget_;
        QMultiLineEdit              * editorWidget_;
        EmpathAttachmentListWidget  * attachmentWidget_; // Iconview or listview

        QActionCollection * actionCollection_;
        
        // QVBoxLayout      * headerLayout_;
        
        EmpathURL       url_;
        QString         recipient_;
        
        int             maxSizeColOne_;

        QSplitter * splitter_;
};

#endif
// vim:ts=4:sw=4:tw=78
