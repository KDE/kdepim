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
# pragma implementation "EmpathComposeWidget.h"
#endif

// Qt includes
#include <qsplitter.h>
#include <qmultilineedit.h>
#include <qvaluelist.h>
#include <qaction.h>

// KDE includes
#include <kprocess.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kaction.h>

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
        EmpathComposeForm composeForm,
        QWidget * parent)
    :  
        QWidget(parent, "EmpathComposeWidget"),
        composeForm_(composeForm)
{
    splitter_ = new QSplitter(Vertical, this, "splitter");
 
    envelopeWidget_ = new EmpathEnvelopeWidget(
            composeForm_.visibleHeaders(), this, "envelopeWidget");
    editorWidget_ = new QMultiLineEdit(splitter_, "editorWidget");
    attachmentWidget_ = new EmpathAttachmentListWidget(splitter_);

    _initActions();
    
    splitter_->setResizeMode(attachmentWidget_, QSplitter::FollowSizeHint);
    
    KConfig * c = KGlobal::config();
   
    using namespace EmpathConfig;

    c->setGroup(GROUP_COMPOSE);

#if 0
    //FIXME Qt hasn't got setWordWrap in today
    if (!c->readBoolEntry(C_WRAP_LINES, true))
        editorWidget_->setWordWrap(QMultiLineEdit::NoWrap);

    else {

        editorWidget_->setWordWrap(QMultiLineEdit::FixedColumnWrap);
        editorWidget_->setWrapColumnOrWidth(
            c->readUnsignedNumEntry(C_WRAP_COLUMN, 76));
    }
#endif

    c->setGroup(GROUP_DISPLAY);

    QFont globalFixedFont = KGlobal::fixedFont();

    editorWidget_->setFont(c->readFontEntry(UI_FIXED_FONT, &globalFixedFont));

    QCString body = composeForm_.body();
    if (!body.isEmpty())
        editorWidget_->setText(body);
 
    // Layouts.
    QVBoxLayout * layout = new QVBoxLayout(this, 4);

    layout->addWidget(envelopeWidget_);
    layout->addWidget(splitter_);

    switch (composeForm_.composeType()) {

        case ComposeForward:
            envelopeWidget_->setFocus();
            break;

        case ComposeNormal:
            envelopeWidget_->setFocus();

            c->setGroup(GROUP_COMPOSE);

            if (c->readBoolEntry(C_USE_EXT_EDIT, false)) {
                editorWidget_->setEnabled(false);
                _spawnExternalEditor(editorWidget_->text().local8Bit());
            }

            break;

        case ComposeReply:
        case ComposeReplyAll:
            editorWidget_->setFocus();

            c->setGroup(GROUP_COMPOSE);

            if (c->readBoolEntry(C_USE_EXT_EDIT, false)) {
                editorWidget_->setEnabled(false);
                _spawnExternalEditor(editorWidget_->text().local8Bit());
            }

            break;

        default:
            empathDebug("Er, what kind of composition is this ?");
            break;
    }
}

EmpathComposeWidget::~EmpathComposeWidget()
{
}

    EmpathComposeForm
EmpathComposeWidget::composeForm()
{
    composeForm_.setVisibleHeaders(envelopeWidget_->headers());
    composeForm_.setBody(editorWidget_->text().local8Bit());
    // composeForm_.attachments    = attachmentWidget_.attachments();
    return composeForm_;
}

    bool
EmpathComposeWidget::messageHasAttachments()
{
    return false;
}

    void
EmpathComposeWidget::_spawnExternalEditor(const QCString & text)
{
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
    editorWidget_->setFocus();
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

/*
    void
EmpathComposeWidget::s_addAttachment()
{
    attachmentWidget_->s_attachmentAdd();
    QValueList<int> sizes;
    sizes   <<  editorWidget_->height()
            << attachmentWidget_->minimumSizeHint().height();
    splitter_->setSizes(sizes);
}

    void
EmpathComposeWidget::s_editAttachment()
{
    attachmentWidget_->s_attachmentEdit();
}

    void
EmpathComposeWidget::s_removeAttachment()
{
    attachmentWidget_->removeAttachment();
    QValueList<int> sizes;
    sizes   <<  editorWidget_->height()
            << attachmentWidget_->minimumSizeHint().height();
    splitter_->setSizes(sizes);
}
*/

    bool 
EmpathComposeWidget::haveTo()       
{ 
    return envelopeWidget_->haveTo(); 
}

    bool 
EmpathComposeWidget::haveSubject()  
{
    return envelopeWidget_->haveSubject(); 
}

    void
EmpathComposeWidget::s_undo()
{
    editorWidget_->undo();
}

    void
EmpathComposeWidget::s_redo()
{
    editorWidget_->redo();
}

    void
EmpathComposeWidget::_initActions()
{
    actionCollection_ = new QActionCollection(this, "actionCollection");

    QValueList<QAction *> childActions( 
            attachmentWidget_->actionCollection()->actions());

    QValueListIterator<QAction *> it;
    for (it = childActions.begin(); it != childActions.end(); ++it)
        actionCollection_->insert(*it);
}

// vim:ts=4:sw=4:tw=78
