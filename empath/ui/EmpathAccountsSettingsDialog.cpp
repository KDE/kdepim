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
#pragma implementation "EmpathAccountsSettingsDialog.h"
#endif

// Qt includes
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes
#include <klocale.h>
#include <klineeditdlg.h>
#include <kbuttonbox.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathAccountsSettingsDialog.h"
#include "RikGroupBox.h"
#include "EmpathServerTypeDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
    
bool EmpathAccountsSettingsDialog::exists_ = false;

    void
EmpathAccountsSettingsDialog::create()
{
    if (exists_)
        return;

    exists_ = true;
    EmpathAccountsSettingsDialog * d = new EmpathAccountsSettingsDialog;
    CHECK_PTR(d);
    d->show();
    kapp->processEvents();
}


EmpathAccountsSettingsDialog::EmpathAccountsSettingsDialog(QWidget * parent)
    :   QDialog(parent, "AccountsSettingsDialog", false)
{
    setCaption(i18n("Accounts Settings"));

    // Mailbox list
    lv_accts_ = new QListView(this, "lv_accts");

    QWhatsThis::add(lv_accts_, i18n(
            "This is a list of all the accounts (mailboxes)\n"
            "that Empath knows about."));

    lv_accts_->addColumn(i18n("Account Name"));
    lv_accts_->addColumn(i18n("Type"));
    lv_accts_->addColumn(i18n("Timer"));
    lv_accts_->setAllColumnsShowFocus(true);
    lv_accts_->setFrameStyle(QFrame::Box | QFrame::Sunken);

    // List editing buttons
    KButtonBox * ctrlButtons = new KButtonBox(this, KButtonBox::VERTICAL);
 
    QPushButton * pb_new    = ctrlButtons->addButton(i18n("&New"));
    QPushButton * pb_edit   = ctrlButtons->addButton(i18n("&Edit"));
    QPushButton * pb_remove = ctrlButtons->addButton(i18n("&Remove"));
    
    ctrlButtons->layout();

    // Help, Ok, Cancel buttons
    KButtonBox * buttonBox_ = new KButtonBox(this);
    
    QPushButton * pb_help_  = buttonBox_->addButton(i18n("&Help"));    
   
    buttonBox_->addStretch();
    
    QPushButton * pb_OK_    = buttonBox_->addButton(i18n("&OK"));
    QPushButton * pb_cancel_= buttonBox_->addButton(i18n("&Cancel"));
    
    pb_OK_->setDefault(true);
    
    buttonBox_->layout();

    // Layout
    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    QHBoxLayout * topLayout = new QHBoxLayout;
    
    topLayout->addWidget(lv_accts_);
    topLayout->addWidget(ctrlButtons);
    
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(buttonBox_);

    // Connections
    QObject::connect(pb_new,        SIGNAL(clicked()),    SLOT(s_new()));
    QObject::connect(pb_edit,       SIGNAL(clicked()),    SLOT(s_edit()));
    QObject::connect(pb_remove,     SIGNAL(clicked()),    SLOT(s_remove())); 

    QObject::connect(pb_OK_,        SIGNAL(clicked()),    SLOT(s_OK()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),    SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),    SLOT(s_help()));

    QObject::connect(
        empath, SIGNAL(updateFolderLists()), SLOT(s_updateMailboxList()));

    s_updateMailboxList();
}

EmpathAccountsSettingsDialog::~EmpathAccountsSettingsDialog()
{
    exists_ = false;
}

    void
EmpathAccountsSettingsDialog::s_new()
{
    EmpathServerTypeDialog serverTypeDialog(this, "serverTypeDialog");
    
    if (serverTypeDialog.exec() != QDialog::Accepted)
        return;

    EmpathMailbox * m =
        empath->mailboxList().createNew(serverTypeDialog.accountType());

    if (m == 0) {
        empathDebug("Cannot create new mailbox");
        return;
    }
}

    void
EmpathAccountsSettingsDialog::s_edit()
{
    EmpathURL url;

    url.setMailboxName(lv_accts_->currentItem()->text(0));

    if (url.mailboxName() == 0) {
        empathDebug("Nothing selected");
        return;
    }

    empath->s_configureMailbox(url, this);
}

    void
EmpathAccountsSettingsDialog::s_updateMailboxList()
{
    lv_accts_->clear();

    for (EmpathMailboxListIterator it(empath->mailboxList()); it.current();++it)
        new EmpathAccountListItem(lv_accts_, it.current());
}


EmpathAccountListItem::EmpathAccountListItem
    (QListView * parent, EmpathMailbox * m)
    :   QListViewItem(parent, m->name())
{
    if (m == 0)
        return;

    setText(0, m->name());
    setText(1, m->typeAsString());
    setText(2, m->checkMail() ?
        QString().setNum(m->checkMailInterval()) : QString::null);

    setPixmap(0, empathIcon("settings-accounts"));
}

    void
EmpathAccountsSettingsDialog::s_remove()
{
    EmpathURL u;
    u.setMailboxName(lv_accts_->currentItem()->text(0));
    empath->mailboxList().remove(u);
}

    void
EmpathAccountsSettingsDialog::s_OK()
{
    hide();
    s_apply();
    KGlobal::config()->sync();
    delete this;
}

    void
EmpathAccountsSettingsDialog::s_help()
{
    //empathInvokeHelp(QString::null, QString::null);
}

    void
EmpathAccountsSettingsDialog::s_apply()
{
    empath->mailboxList().saveConfig();
    applied_ = true;
}

    void
EmpathAccountsSettingsDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    delete this;
}

// vim:ts=4:sw=4:tw=78
