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
# pragma implementation "EmpathMatchPropertiesDialog.h"
#endif

// Qt includes
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

// KDE includes
#include <klocale.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathMatcher.h"
#include "EmpathMatchPropertiesDialog.h"
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"
#include "Empath.h"
        
EmpathMatchPropertiesDialog::EmpathMatchPropertiesDialog(
        QWidget * parent,
        EmpathMatcher * matcher)
    :    QDialog(parent, "matchPropertiesDialog", true),
        matcher_(matcher)
{
    setCaption(i18n("Match expression"));
    
    bg_choices_ = new QButtonGroup(this, "bg_choices");
    bg_choices_->hide();

    QRadioButton * rb_size =
        new QRadioButton(i18n("Size larger than"), this, "rb_size");
    
    QRadioButton * rb_exprBody =
        new QRadioButton(i18n("Expression in message"),this, "rb_exprBody");
    
    QRadioButton * rb_exprHeader =
        new QRadioButton(i18n("Expression in header"), this, "rb_exprHeader");
    
    QRadioButton * rb_attached =
        new QRadioButton(i18n("Has attachment(s)"), this, "rb_attached");
    
    QRadioButton * rb_all =
        new QRadioButton(i18n("Any message"), this, "rb_all");

    bg_choices_->insert(rb_size,        EmpathMatcher::Size);
    bg_choices_->insert(rb_exprBody,    EmpathMatcher::BodyExpr);
    bg_choices_->insert(rb_exprHeader,  EmpathMatcher::HeaderExpr);    
    bg_choices_->insert(rb_attached,    EmpathMatcher::HasAttachments);
    bg_choices_->insert(rb_all,         EmpathMatcher::AnyMessage);

    sb_size_ = new QSpinBox(1, 1000, 1, this, "sb_size");
    sb_size_->setSuffix(" kB");

    le_exprBody_ = new QLineEdit(this, "le_exprBody_");

    cb_header_ = new QComboBox(true, this, "cb_header");

    le_exprHeader_ = new QLineEdit(this, "le_exprHeader_");
    
    bg_choices_->setButton(EmpathMatcher::Size);
 
    KButtonBox * buttonBox = new KButtonBox(this);

    // Bottom button group
    QPushButton * pb_help   = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    QPushButton * pb_OK     = buttonBox->addButton(i18n("&OK"));
    QPushButton * pb_cancel = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    // Layout

    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QHBoxLayout * layout0 = new QHBoxLayout(layout);
    layout0->addWidget(rb_size);
    layout0->addWidget(sb_size_);

    QHBoxLayout * layout1 = new QHBoxLayout(layout);
    layout1->addWidget(rb_exprBody);
    layout1->addWidget(le_exprBody_);
    
    QHBoxLayout * layout2 = new QHBoxLayout(layout);
    layout2->addWidget(rb_exprHeader);
    layout2->addWidget(cb_header_);
    layout2->addWidget(le_exprHeader_);

    layout->addWidget(rb_attached);
    layout->addWidget(rb_all);

    layout->addStretch(10);
    layout->addWidget(buttonBox);
    
    if (matcher_ == 0)
        return;
    
    sb_size_->setValue(matcher_->size());
    le_exprBody_->setText(matcher_->matchExpr());

    QString s = matcher_->matchHeader();

    if (s.isEmpty())
        cb_header_->insertItem("X-Mailing-List", 0);
    else
        cb_header_->insertItem(matcher_->matchHeader(), 0);

    cb_header_->setCurrentItem(0);
    le_exprBody_->setText(matcher_->matchExpr());

    QObject::connect(pb_OK, SIGNAL(clicked()),
            this, SLOT(s_OK()));
    
    QObject::connect(pb_cancel, SIGNAL(clicked()),
            this, SLOT(s_cancel()));
    
    QObject::connect(pb_help, SIGNAL(clicked()),
            this, SLOT(s_help()));
}

EmpathMatchPropertiesDialog::~EmpathMatchPropertiesDialog()
{
    // Empty.
}

    void
EmpathMatchPropertiesDialog::s_OK()
{
    EmpathMatcher::MatchExprType t =
        (EmpathMatcher::MatchExprType)
        bg_choices_->id(bg_choices_->selected());
    
    matcher_->setType(t);

    switch (t) {
        
        case EmpathMatcher::Size:
            matcher_->setSize(sb_size_->value());
            break;

        case EmpathMatcher::BodyExpr:
            matcher_->setMatchExpr(le_exprBody_->text());
            break;
        
        case EmpathMatcher::HeaderExpr:
            matcher_->setMatchExpr(le_exprHeader_->text());
            break;

        case EmpathMatcher::AnyMessage:
        case EmpathMatcher::HasAttachments:
        default:
            break;
    }
    
    accept();
}

    EmpathMatcher *
EmpathMatchPropertiesDialog::matcher()
{
    return matcher_;
}

    void
EmpathMatchPropertiesDialog::s_cancel()
{
    reject();
}
    
    void
EmpathMatchPropertiesDialog::s_help()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
