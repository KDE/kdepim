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
# pragma implementation "EmpathUI.h"
#endif

// Qt includes
#include <qstring.h>
#include <qwidgetlist.h>
#include <qapplication.h>
#include <qpngio.h>
#include <qmessagebox.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>
#include <ktmainwindow.h>
#include <kaboutdialog.h>
#include <kfiledialog.h>

// Local includes
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathUI.h"
#include "EmpathSetupWizard.h"
#include "EmpathMainWindow.h"
#include "EmpathComposeWindow.h"
#include "EmpathDisplaySettingsDialog.h"
#include "EmpathIdentitySettingsDialog.h"
#include "EmpathComposeSettingsDialog.h"
#include "EmpathSendingSettingsDialog.h"
#include "EmpathAccountsSettingsDialog.h"
#include "EmpathFilterManagerDialog.h"
#include "EmpathConfigMaildirDialog.h"
#include "EmpathConfigIMAP4Dialog.h"
#include "EmpathConfigPOP3Dialog.h"

EmpathUI::EmpathUI()
    : QObject()
{
    _wizard();
    
//    qInitPngIO();
    
    _connectUp();

    kapp->setMainWidget(new EmpathMainWindow);
}

EmpathUI::~EmpathUI()
{
}

    void
EmpathUI::s_newComposer(Empath::ComposeType t, const EmpathURL & m)
{
    (new EmpathComposeWindow(t, m))->show();
}

    void    
EmpathUI::s_newComposer(const QString & recipient)
{
    (new EmpathComposeWindow(recipient))->show();
}

    void
EmpathUI::s_setupDisplay(QWidget * parent)
{
    EmpathDisplaySettingsDialog d(parent);
    d.loadData();
    d.exec();
}
    void
EmpathUI::s_setupIdentity(QWidget * parent)
{
    EmpathIdentitySettingsDialog d(parent);
    d.loadData();
    d.exec();
}

    void
EmpathUI::s_setupSending(QWidget * parent)
{
    EmpathSendingSettingsDialog d(parent);
    d.loadData();
    d.exec();
}

    void
EmpathUI::s_setupComposing(QWidget * parent)
{
    EmpathComposeSettingsDialog d(parent);
    d.loadData();
    d.exec();
}

    void
EmpathUI::s_setupAccounts(QWidget * parent)
{
    EmpathAccountsSettingsDialog d(parent);
    d.loadData();
    d.exec();
}

    void
EmpathUI::s_setupFilters(QWidget * parent)
{
    EmpathFilterManagerDialog d(parent);
    d.loadData();
    d.exec();
}

    void
EmpathUI::s_about(QWidget * parent)
{
    QMessageBox::information(
        parent,
        "Hey !",
        i18n("Is it raining icepicks on your steel shore ?"),
        i18n("Yes/No"));
}

    void
EmpathUI::s_bugReport()
{
    EmpathComposeWindow * w = new EmpathComposeWindow;
    w->bugReport();
    w->show();
}

    void
EmpathUI::s_infoMessage(const QString & s)
{
    QWidgetList * l = QApplication::topLevelWidgets();
    
    if (l->isEmpty())
        return;
    
    QWidgetListIt it(*l);
    
    for (; it.current(); ++it)
        if (it.current()->inherits("KTMainWindow"))
            ((KTMainWindow *)it.current())->statusBar()->message(s, 4000);
    
    delete l;
    l = 0;
}


    void
EmpathUI::s_getSaveName(const EmpathURL & url, QWidget * parent)
{
    QString saveFilePath =
        KFileDialog::getSaveFileName(QString::null, QString::null, parent);
    
    if (saveFilePath.isEmpty())
        return;
   
    empath->s_saveNameReady(url, saveFilePath);
}

    void
EmpathUI::s_configureMailbox(const EmpathURL & url, QWidget * w)
{
    EmpathMailbox * mailbox = empath->mailbox(url);

    if (mailbox == 0) {
        empathDebug("Cannot find mailbox with url `" + url.asString() + "'");
        return;
    }
 
    switch (mailbox->type()) {

        case EmpathMailbox::Maildir:
            EmpathConfigMaildirDialog::create(url, w);
            break;

        case EmpathMailbox::POP3:
            EmpathConfigPOP3Dialog::create(url, w);
            break;

        case EmpathMailbox::IMAP4:
            EmpathConfigIMAP4Dialog::create(url, w);
            break;

        default:
            empathDebug("I can't configure a mailbox if I don't know its type");
            break;
    }
}

    void
EmpathUI::_connectUp()
{
    QObject::connect(
        empath, SIGNAL(getSaveName(const EmpathURL &, QWidget *)),
        this,   SLOT(s_getSaveName(const EmpathURL &, QWidget *)));
    
    QObject::connect(
        empath, SIGNAL(infoMessage(const QString &)),
        this,   SLOT(s_infoMessage(const QString &)));

    QObject::connect(
        empath, SIGNAL(newComposer(Empath::ComposeType, const EmpathURL &)),
        this,   SLOT(s_newComposer(Empath::ComposeType, const EmpathURL &)));
    
    QObject::connect(
        empath, SIGNAL(newComposer(const QString &)),
        this,   SLOT(s_newComposer(const QString &)));
    
    QObject::connect(
        empath, SIGNAL(configureMailbox(const EmpathURL &, QWidget *)),
        this,   SLOT(s_configureMailbox(const EmpathURL &, QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupDisplay(QWidget *)),
        this,   SLOT(s_setupDisplay(QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupIdentity(QWidget *)),
        this,   SLOT(s_setupIdentity(QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupSending(QWidget *)),
        this,   SLOT(s_setupSending(QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupComposing(QWidget *)),
        this,   SLOT(s_setupComposing(QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupAccounts(QWidget *)),
        this,   SLOT(s_setupAccounts(QWidget *)));

    QObject::connect(
        empath, SIGNAL(setupFilters(QWidget *)),
        this,   SLOT(s_setupFilters(QWidget *)));

    QObject::connect(
        empath, SIGNAL(about(QWidget *)),
        this,   SLOT(s_about(QWidget *)));

    QObject::connect(
        empath, SIGNAL(bugReport()),
        this,   SLOT(s_bugReport()));
}

    void
EmpathUI::_wizard()
{
    // If no mailboxes are configured, then show the setup wizard.

    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_GENERAL);
    
    QStringList l = c->readListEntry(GEN_MAILBOX_LIST);
    
    if (l.isEmpty()) {
        EmpathSetupWizard wiz;
        wiz.exec();
    }
}
 
// vim:ts=4:sw=4:tw=78
