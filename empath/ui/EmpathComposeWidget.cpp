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
# pragma implementation "EmpathComposeWidget.h"
#endif

// Qt includes
#include <qsplitter.h>
#include <qvaluelist.h>

// KDE includes
#include <kprocess.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <keditcl.h>
#include <kapp.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathEnvelopeWidget.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathEditorProcess.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMailSender.h"
#include "EmpathComposer.h"
#include <RMM_DateTime.h>
#include <RMM_Address.h>

EmpathComposeWidget::EmpathComposeWidget(
        const EmpathComposer::Form &    composeForm,
        QWidget *                       parent,
        const char *                    name)
    :
        QWidget(parent, name),
        composeForm_(composeForm)
{
    QSplitter * splitter = new QSplitter(Vertical, this, "splitter");
 
    envelopeWidget_ = 
            new EmpathEnvelopeWidget(composeForm_.visibleHeaders, this, "envelopeWidget");
    editorWidget_ = 
            new QMultiLineEdit(splitter, "editorWidget");
    attachmentWidget_ = 
            new EmpathAttachmentListWidget(splitter, "attachmentWidget");
    
    KConfig * c = KGlobal::config();
   
    // If user wants us to wrap lines at a specific value, we can do that
    // in the editor to their specified width. This should give the user
    // a good idea of what their text will look like on the other end.
    //
    // If the user doesn't want us to wrap, we'll wrap text dynamically
    // in the editor anyway to make editing easier. We must not wrap the
    // actual text we send though.

    c->setGroup(EmpathConfig::GROUP_COMPOSE);
    if (!c->readBoolEntry(EmpathConfig::C_WRAP_LINES, true))
        editorWidget_->setWordWrap(QMultiLineEdit::DynamicWrap);
    else {
        editorWidget_->setWordWrap(QMultiLineEdit::FixedColumnWrap);
        editorWidget_->setWrapColumnOrWidth(
            c->readUnsignedNumEntry(EmpathConfig::C_WRAP_COLUMN, 76));
    }
        
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    editorWidget_->setFont(
        KGlobal::config()->readFontEntry(EmpathConfig::UI_FIXED_FONT));
    editorWidget_->setText(composeForm_.body);
 
    // Layouts.
    QVBoxLayout * layout    = new QVBoxLayout(this, 4);

    layout->addWidget(envelopeWidget_,  0);
    layout->addWidget(splitter,         1);

    // set the behaviour of the splitter and its children.
    QValueList<int> sizes;
    sizes.append(height());
    sizes.append(0);
    splitter->setSizes(sizes);
    splitter->setResizeMode(attachmentWidget_, QSplitter::KeepSize);

    if (composeForm_.composeType == EmpathComposer::ComposeForward) 
        return;

    if (composeForm_.composeType == EmpathComposer::ComposeReply ||
        composeForm_.composeType == EmpathComposer::ComposeReplyAll)
        editorWidget_->setFocus();
    
    KGlobal::config()->setGroup(EmpathConfig::GROUP_COMPOSE);

    if (KGlobal::config()->readBoolEntry(
            EmpathConfig::C_USE_EXT_EDIT, false)) {
        
        editorWidget_->setEnabled(false);
        _spawnExternalEditor(editorWidget_->text().local8Bit());
    }
}

EmpathComposeWidget::~EmpathComposeWidget()
{
    empathDebug("dtor");
}

    EmpathComposer::Form &
EmpathComposeWidget::composeForm()
{
    composeForm_.visibleHeaders = envelopeWidget_->headers();
    composeForm_.body           = editorWidget_->text().local8Bit();
    // composeForm_.attachments    = attachmentWidget_.attachments();
    return composeForm_;
}

    bool
EmpathComposeWidget::messageHasAttachments()
{
    empathDebug("messageHasAttachments() called");
    return false;
}

    void
EmpathComposeWidget::_spawnExternalEditor(const QCString & text)
{
    empathDebug("spawnExternalEditor() called");
    
    EmpathEditorProcess * p = new EmpathEditorProcess(text);
    CHECK_PTR(p);
    
    QObject::connect(
        p,      SIGNAL( done            (bool, QCString)),
        this,   SLOT(   s_editorDone    (bool, QCString)));
    
    p->go();
}

    void
EmpathComposeWidget::s_editorDone(bool ok, QCString text)
{
    if (!ok) {
        empath->s_infoMessage(i18n("Message not modified by external editor"));
        editorWidget_->setEnabled(true);
        return;
    }
    
    editorWidget_->setText(text);
    editorWidget_->setEnabled(true);
}

    void
EmpathComposeWidget::s_cut()
{
    editorWidget_->cut();
}

    void
EmpathComposeWidget::s_copy()
{
    editorWidget_->copy();
}

    void
EmpathComposeWidget::s_paste()
{
    editorWidget_->paste();
}

    void
EmpathComposeWidget::s_selectAll()
{
    editorWidget_->selectAll();
}

    void
EmpathComposeWidget::s_addAttachment()
{
    empathDebug("addAttachment() called");
    attachmentWidget_->addAttachment();
}

    void
EmpathComposeWidget::s_editAttachment()
{
    empathDebug("editAttachment() called");
    attachmentWidget_->editAttachment();
}

    void
EmpathComposeWidget::s_removeAttachment()
{
    empathDebug("removeAttachment() called");
    attachmentWidget_->removeAttachment();
}

    bool 
EmpathComposeWidget::haveTo()       
{ 
    return true;
    // return envelopeWidget_->haveTo(); 
}

    bool 
EmpathComposeWidget::haveSubject()  
{
    return true;
    // return envelopeWidget_->haveSubject(); 
}

// vim:ts=4:sw=4:tw=78
