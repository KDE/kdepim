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
#include "EmpathSeparatorWidget.h"
#include "EmpathServerTypeDialog.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
    
EmpathAccountsSettingsDialog::EmpathAccountsSettingsDialog(QWidget * parent)
    :   QDialog(parent, "AccountsSettings", true)
{
    setCaption(i18n("Accounts Settings"));

    // Mailbox list
    lv_accts_ = new QListView(this, "lv_accts");

    QWhatsThis::add(lv_accts_, i18n(
            "This is a list of all the accounts (mailboxes)\n"
            "that Empath knows about."));

    lv_accts_->addColumn(i18n("Account Name"));
    lv_accts_->addColumn(i18n("Type"));
    lv_accts_->setAllColumnsShowFocus(true);
    lv_accts_->setFrameStyle(QFrame::Box | QFrame::Sunken);

    // List editing buttons
    KButtonBox * ctrlButtons = new KButtonBox(this, KButtonBox::VERTICAL);
 
    QPushButton * pb_newPOP     = ctrlButtons->addButton(i18n("New &POP3"));
    QPushButton * pb_newIMAP    = ctrlButtons->addButton(i18n("New &IMAP4"));
    QPushButton * pb_edit       = ctrlButtons->addButton(i18n("&Edit"));
    pb_remove_    = ctrlButtons->addButton(i18n("&Remove"));
    
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
    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);
    QHBoxLayout * topLayout = new QHBoxLayout(layout);
    
    topLayout->addWidget(lv_accts_);
    topLayout->addWidget(ctrlButtons);
    
    layout->addStretch(10);

    layout->addWidget(new EmpathSeparatorWidget(this));

    layout->addWidget(buttonBox_);

    // Connections
    QObject::connect(pb_newPOP,     SIGNAL(clicked()),    SLOT(s_newPOP()));
    QObject::connect(pb_newIMAP,    SIGNAL(clicked()),    SLOT(s_newIMAP()));
    QObject::connect(pb_edit,       SIGNAL(clicked()),    SLOT(s_edit()));
    QObject::connect(pb_remove_,    SIGNAL(clicked()),    SLOT(s_remove())); 

    QObject::connect(pb_OK_,        SIGNAL(clicked()),    SLOT(s_OK()));
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),    SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),    SLOT(s_help()));

    QObject::connect(
        empath, SIGNAL(updateFolderLists()), SLOT(s_updateMailboxList()));
    
    QObject::connect(
        lv_accts_, SIGNAL(currentChanged(QListViewItem *)),
        this, SLOT(s_currentChanged(QListViewItem *)));

    QWhatsThis::add(pb_newPOP, i18n(
            "A POP3 mailbox is accessed over a network.\n"
            "You can access one on your own machine if\n"
            "you have a POP3 server program. POP3 mailboxes\n"
            "are read-only. That is, you can only <b>get</b>\n"
            "messages from them, you can't put mail back into\n"
            "them.\n\n"
            "POP3 is the most common mailbox format used by\n"
            "ISPs (Internet Service Providers). It provides\n"
            "a simple mechanism for retrieving messages.\n\n"
            "Note that as 'mbox' format mailboxes are not\n"
            "supported, using a local POP3 server is the only\n"
            "way to retrieve mail from these boxes.\n"));

    QWhatsThis::add(pb_newIMAP, i18n(
            "An IMAP4 mailbox is accessed over a network.\n"
            "You can access one on your own machine if you\n"
            "have an IMAP4 server program."));
}

EmpathAccountsSettingsDialog::~EmpathAccountsSettingsDialog()
{
    // Empty.
}

    void
EmpathAccountsSettingsDialog::s_newPOP()
{
    empath->mailboxList().createNew(EmpathMailbox::POP3);
    s_updateMailboxList();
}

    void
EmpathAccountsSettingsDialog::s_newIMAP()
{
    empath->mailboxList().createNew(EmpathMailbox::IMAP4);
    s_updateMailboxList();
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
EmpathAccountsSettingsDialog::loadData()
{
    s_updateMailboxList();
}

    void
EmpathAccountsSettingsDialog::s_updateMailboxList()
{
    lv_accts_->clear();

    EmpathMailboxListIterator it(empath->mailboxList());

    for (; it.current(); ++it)
        if (it.current()->type() != EmpathMailbox::Maildir)
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
    setText(2, m->autoCheck() ?
        QString().setNum(m->autoCheckInterval()) : QString::null);

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
    accept();
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
    reject();
}


    void
EmpathAccountsSettingsDialog::s_currentChanged(QListViewItem * item)
{
    if (!item == 0)
        return;
    
    if (item->text(1) == "Maildir")
        pb_remove_->setEnabled(false);
    else
        pb_remove_->setEnabled(true);
}

// vim:ts=4:sw=4:tw=78
