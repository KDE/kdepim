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
# pragma implementation "EmpathMessageMarkDialog.h"
#endif

// Qt includes
#include <qwhatsthis.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathMessageMarkDialog.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
        
EmpathMessageMarkDialog::EmpathMessageMarkDialog(
        QWidget * parent,
        const char * name)
    :    QDialog(parent, name, true)
{
    setCaption(i18n("Mark Messages"));

    typeButtonGroup_    = new QButtonGroup(this, "typeButtonGroup");
    
    typeButtonGroup_->hide();
    typeButtonGroup_->setExclusive(true);

    stateButtonGroup_    = new QButtonGroup(this, "stateButtonGroup");
    
    stateButtonGroup_->hide();
    stateButtonGroup_->setExclusive(true);

    // Mark type group box

    rb_tagged_    =
        new QRadioButton(i18n("Tagged"), this, "rb_tagged");
    
    QWhatsThis::add(rb_tagged_, i18n(
            "Tagging messages is for your own\n"
            "benefit. You can use this to simply\n"
            "remember which messages you are interested\n"
            "in; you can select 'tagged messages' for\n"
            "some operations."));
    
    int h = rb_tagged_->sizeHint().height();
    
    rb_replied_    =
        new QRadioButton(i18n("Replied"), this, "rb_replied");
    
    QWhatsThis::add(rb_replied_, i18n(
            "You can mark a message as replied to\n"
            "if you wish. Why you'd want to is your\n"
            "business."));
    
    rb_read_    =
        new QRadioButton(i18n("Read"), this, "rb_read");
    
    QWhatsThis::add(rb_read_, i18n(
            "Marking messages as read is useful when\n"
            "you know you're not interested in some messages\n"
            "and don't want to read them. They'll henceforth\n"
            "be treated just the same as those you really have\n"
            "read."));

    rb_tagged_->setChecked(true);
        
    // State group box

    rb_on_    =
        new QRadioButton(i18n("On"), this, "rb_on");
    
    QWhatsThis::add(rb_on_, i18n(
            "If you select this then every selected message\n"
            "will have its state switched on. For example,\n"
            "if you select 'Tag' and 'On' then untagged messages\n"
            "will become tagged."));
    
    rb_off_    =
        new QRadioButton(i18n("Off"), this, "rb_off");
    
    QWhatsThis::add(rb_off_, i18n(
            "If you select this then every selected message\n"
            "will have its state switched off. For example,\n"
            "if you select 'Tag' and 'Off' then tagged messages\n"
            "will become untagged."));
    
    rb_toggle_    =
        new QRadioButton(i18n("Toggle"), this, "rb_toggle");
    
    QWhatsThis::add(rb_toggle_, i18n(
            "If you select this then every selected message\n"
            "will have its state toggled. For example, if you\n"
            "select 'Tag' and 'Toggle' then tagged messages\n"
            "will be untagged, and vice versa."));
    
    rb_on_->setChecked(true);
    
    l_tagged_    = new QLabel(this);
    
    l_tagged_->setPixmap(empathIcon("tree-marked"));
    
    l_replied_    = new QLabel(this);
    
    
    l_replied_->setPixmap(empathIcon("tree-replied"));
    
    l_read_        = new QLabel(this);

    l_read_->setPixmap(empathIcon("tree-read"));
    
    stateButtonGroup_->insert(rb_tagged_,    RMM::Marked);
    stateButtonGroup_->insert(rb_replied_,    RMM::Replied);
    stateButtonGroup_->insert(rb_read_,        RMM::Read);

    typeButtonGroup_->insert(rb_on_,        On);
    typeButtonGroup_->insert(rb_off_,        Off);
    typeButtonGroup_->insert(rb_toggle_,    Toggle);

    // Button box
    
    buttonBox_ = new KButtonBox(this);
    
    pb_Help_    = buttonBox_->addButton(i18n("&Help"));
    buttonBox_->addStretch();
    pb_OK_        = buttonBox_->addButton(i18n("&OK"));
    pb_Cancel_    = buttonBox_->addButton(i18n("&Cancel"));
    buttonBox_->layout();
    
    QObject::connect(pb_Help_, SIGNAL(clicked()),
            this, SLOT(s_Help()));
    
    QObject::connect(pb_OK_, SIGNAL(clicked()),
            this, SLOT(s_OK()));
    
    QObject::connect(pb_Cancel_, SIGNAL(clicked()),
            this, SLOT(s_Cancel()));
    
    // Layouts
    
}

    void
EmpathMessageMarkDialog::s_OK()
{
    accept();
}

    void
EmpathMessageMarkDialog::s_Cancel()
{
    reject();
}

    void
EmpathMessageMarkDialog::s_Help()
{
    //empathInvokeHelp("","");
}

    EmpathMessageMarkDialog::MarkType
EmpathMessageMarkDialog::markType()
{
    return (
        (EmpathMessageMarkDialog::MarkType)
        (typeButtonGroup_->id(typeButtonGroup_->selected())));
}

    RMM::MessageStatus
EmpathMessageMarkDialog::status()
{
    return (
        (RMM::MessageStatus)
        (stateButtonGroup_->id(stateButtonGroup_->selected())));
}

// vim:ts=4:sw=4:tw=78
