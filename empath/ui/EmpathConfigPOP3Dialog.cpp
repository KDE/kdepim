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
# pragma implementation "EmpathConfigPOP3Dialog.h"
#endif

// Qt includes
#include <qlayout.h>

// KDE includes
#include <klocale.h>
#include <kbuttonbox.h>

// Local includes
#include "EmpathConfigPOP3Dialog.h"
#include "EmpathConfigPOP3Widget.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathUIUtils.h"

bool EmpathConfigPOP3Dialog::exists_ = false;

    void
EmpathConfigPOP3Dialog::create(const EmpathURL & url, QWidget * parent)
{
    if (exists_)
        return;

    exists_ = true;

    EmpathConfigPOP3Dialog * d = new EmpathConfigPOP3Dialog(url, parent);

    d->show();
    d->loadData();
}
 
EmpathConfigPOP3Dialog::EmpathConfigPOP3Dialog
    (const EmpathURL & url, QWidget * parent)
    :   QDialog(parent, "ConfigPOP3Dialog", true),
        url_(url)
{
    QString caption = i18n("Configuring mailbox %1");
    
    setCaption(caption.arg(url_.mailboxName()));

    settingsWidget_ = new EmpathConfigPOP3Widget(url_, this);

    QPushButton tempButton((QWidget *)0, "MI");
    
    KButtonBox * buttonBox    = new KButtonBox(this);

    // Bottom button group
    pb_Help_    = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    pb_OK_      = buttonBox->addButton(i18n("&OK"));
    pb_Cancel_  = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    QObject::connect(pb_OK_, SIGNAL(clicked()),
            this, SLOT(s_OK()));
    
    QObject::connect(pb_Cancel_, SIGNAL(clicked()),
            this, SLOT(s_Cancel()));
    
    QObject::connect(pb_Help_, SIGNAL(clicked()),
            this, SLOT(s_Help()));

    QVBoxLayout * mainLayout =
        new QVBoxLayout(this, dialogSpace);

    mainLayout->addWidget(settingsWidget_);
    mainLayout->addStretch(10);
    mainLayout->addWidget(buttonBox);
    
    mainLayout->activate();
}

EmpathConfigPOP3Dialog::~EmpathConfigPOP3Dialog()
{
    exists_ = false;
}

    void
EmpathConfigPOP3Dialog::s_OK()
{
    settingsWidget_->saveData();
    accept();
}

    void
EmpathConfigPOP3Dialog::s_Cancel()
{
    reject();
}

    void
EmpathConfigPOP3Dialog::s_Help()
{
    // STUB
}

    void
EmpathConfigPOP3Dialog::loadData()
{
    settingsWidget_->loadData();
}

    void
EmpathConfigPOP3Dialog::saveData()
{
    settingsWidget_->saveData();
}


// vim:ts=4:sw=4:tw=78
