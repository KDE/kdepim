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
# pragma implementation "EmpathFilterActionDialog.h"
#endif

// KDE includes
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "EmpathFolderChooserWidget.h"
#include "EmpathFilterEventHandler.h"
#include "EmpathFilter.h"
#include "EmpathAddressSelectionWidget.h"
#include "EmpathFilterActionDialog.h"
#include "EmpathConfig.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
        
EmpathFilterActionDialog::EmpathFilterActionDialog(
        EmpathFilter * filter,
        QWidget * parent,
        const char * name)
    :    QDialog(parent, name, true),
        filter_(filter)
{
    setCaption(i18n("Filter Action"));
    
    bg_choices_ = new QButtonGroup(this, "bg_choices");
    bg_choices_->hide();

    rb_moveFolder_ =
        new QRadioButton(i18n("Move to"), this, "rb_moveFolder");

    rb_copyFolder_ =
        new QRadioButton(i18n("Copy to"), this, "rb_copyFolder");

    rb_forwardTo_ =
        new QRadioButton(i18n("Forward to"), this, "rb_forward");

    rb_delete_ =
        new QRadioButton(i18n("Delete"), this, "rb_delete");

    rb_ignore_ =
        new QRadioButton(i18n("Ignore"), this, "rb_ignore");

    bg_choices_->insert(rb_moveFolder_, EmpathFilterEventHandler::MoveFolder);
    bg_choices_->insert(rb_copyFolder_, EmpathFilterEventHandler::CopyFolder);
    bg_choices_->insert(rb_delete_,     EmpathFilterEventHandler::Delete);
    bg_choices_->insert(rb_ignore_,     EmpathFilterEventHandler::Ignore);
    bg_choices_->insert(rb_forwardTo_,  EmpathFilterEventHandler::Forward);
    
    bg_choices_->setButton(EmpathFilterEventHandler::MoveFolder);

    fcw_moveFolder_ = new EmpathFolderChooserWidget(this);
    fcw_copyFolder_ = new EmpathFolderChooserWidget(this);
    asw_address_    = new EmpathAddressSelectionWidget(this);
    cb_continue_    = new QCheckBox(i18n("Continue matching"), this);
    cb_continue_->setChecked(true);

    buttonBox_    = new KButtonBox(this);
    
    pb_help_    = buttonBox_->addButton(i18n("&Help"));    
    buttonBox_->addStretch();
    pb_OK_        = buttonBox_->addButton(i18n("&OK"));
    pb_cancel_    = buttonBox_->addButton(i18n("&Cancel"));
    
    buttonBox_->layout();

    // Layout

    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QHBoxLayout * layout0 = new QHBoxLayout(layout);
    layout0->addWidget(rb_moveFolder_);
    layout0->addWidget(fcw_moveFolder_);

    QHBoxLayout * layout1 = new QHBoxLayout(layout);
    layout1->addWidget(rb_copyFolder_);
    layout1->addWidget(fcw_copyFolder_);

    QHBoxLayout * layout2 = new QHBoxLayout(layout);
    layout2->addWidget(rb_forwardTo_);
    layout2->addWidget(asw_address_);

    layout->addWidget(rb_delete_);
    layout->addWidget(rb_ignore_);

    layout->addWidget(cb_continue_);
    
    layout->addStretch(10);
    layout->addWidget(buttonBox_);

    QObject::connect(pb_OK_,        SIGNAL(clicked()), SLOT(s_OK()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()), SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()), SLOT(s_help()));

    load();
}

EmpathFilterActionDialog::~EmpathFilterActionDialog()
{
    // Empty.
}

    void
EmpathFilterActionDialog::load()
{
    if (filter_ == 0) {
        empathDebug("Filter is 0 !!!");
        return;
    }

    EmpathFilterEventHandler * handler = filter_->eventHandler();

    if (handler == 0) return;
    
    bg_choices_->setButton((int)handler->actionType());
    
    switch (handler->actionType()) {
        
        case EmpathFilterEventHandler::MoveFolder:
            fcw_moveFolder_->setURL(handler->moveOrCopyFolder());
            break;
        
        case EmpathFilterEventHandler::CopyFolder:
            fcw_copyFolder_->setURL(handler->moveOrCopyFolder());
            break;
            
        case EmpathFilterEventHandler::Forward:
            asw_address_->setText(handler->forwardAddress());
            break;
            
        case EmpathFilterEventHandler::Delete:
        case EmpathFilterEventHandler::Ignore:
        default:
            break;
    }
}

    void
EmpathFilterActionDialog::s_OK()
{
    EmpathFilterEventHandler * handler = new EmpathFilterEventHandler;
    
    EmpathFilterEventHandler::ActionType t =
        (EmpathFilterEventHandler::ActionType)
        bg_choices_->id(bg_choices_->selected());

    switch (t) {

        case EmpathFilterEventHandler::MoveFolder:
            handler->setMoveFolder(fcw_moveFolder_->url());
            break;

        case EmpathFilterEventHandler::CopyFolder:
            handler->setCopyFolder(fcw_copyFolder_->url());
            break;
            
        case EmpathFilterEventHandler::Delete:
            handler->setDelete();
            break;

        case EmpathFilterEventHandler::Forward:
            handler->setForward(asw_address_->text());
            break;

        case EmpathFilterEventHandler::Ignore:
        default:
            handler->setIgnore();
            break;
    }
    
    filter_->setEventHandler(handler);
    
    accept();
}

    void
EmpathFilterActionDialog::s_cancel()
{
    reject();
}
    
    void
EmpathFilterActionDialog::s_help()
{
    // STUB
}

// vim:ts=4:sw=4:tw=78
