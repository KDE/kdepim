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

// Qt includes
#include <qsplitter.h>
#include <qlayout.h>
#include <qmultilineedit.h>

// KDE includes
#include <kaction.h>

// Local includes
#include "EmpathComposeWidget.h"
#include "EmpathAttachmentListWidget.h"
#include "EmpathEnvelopeWidget.h"

EmpathComposeWidget::EmpathComposeWidget(
        EmpathComposeForm composeForm,
        QWidget * parent
)
    :  
        QWidget(parent, "EmpathComposeWidget"),
        composeForm_(composeForm)
{
    QSplitter * splitter = new QSplitter(Vertical, this, "splitter");
 
    envelopeWidget_ =
        new EmpathEnvelopeWidget(composeForm_.visibleHeaders(), this);

    editorWidget_       = new QMultiLineEdit(splitter);

    editorWidget_->setText(composeForm_.body());

    attachmentWidget_   = new EmpathAttachmentListWidget(splitter);

    attachmentWidget_->hide();

    QVBoxLayout * layout = new QVBoxLayout(this, 4);

    layout->addWidget(envelopeWidget_);
    layout->addWidget(splitter);

    switch (composeForm_.composeType()) {

        case EmpathComposeForm::Normal:
        case EmpathComposeForm::Reply:
        case EmpathComposeForm::ReplyAll:
            editorWidget_->setFocus();
            break;

        case EmpathComposeForm::Forward:
        default:
            envelopeWidget_->setFocus();
            break;
    }

    _initActions();
}

EmpathComposeWidget::~EmpathComposeWidget()
{
    // Empty.
}

    EmpathComposeForm
EmpathComposeWidget::composeForm()
{
    composeForm_.setVisibleHeaders(envelopeWidget_->headers());
    composeForm_.setBody(editorWidget_->text());
//TODO    composeForm_.attachments = attachmentWidget_.attachments();
    return composeForm_;
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
