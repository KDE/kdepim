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
#include "EmpathUI.h"
#include "EmpathUIUtils.h"
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
//#include "EmpathSetupWizard.h"
#include "EmpathConfig.h"

EmpathUI::EmpathUI()
    : QObject()
{
    qInitPngIO();
    
    KConfig * c(KGlobal::config());
    
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
    QObject::connect(
        empath, SIGNAL(getSaveName(const EmpathURL &)),
        this,   SLOT(s_getSaveName(const EmpathURL &)));
    
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
        empath, SIGNAL(setupWizard()),      this, SLOT(s_setupWizard()));
    QObject::connect(
        empath, SIGNAL(setupDisplay()),     this, SLOT(s_setupDisplay()));
    QObject::connect(
        empath, SIGNAL(setupIdentity()),    this, SLOT(s_setupIdentity()));
    QObject::connect(
        empath, SIGNAL(setupSending()),     this, SLOT(s_setupSending()));
    QObject::connect(
        empath, SIGNAL(setupComposing()),   this, SLOT(s_setupComposing()));
    QObject::connect(
        empath, SIGNAL(setupAccounts()),    this, SLOT(s_setupAccounts()));
    QObject::connect(
        empath, SIGNAL(setupFilters()),     this, SLOT(s_setupFilters()));
    QObject::connect(
        empath, SIGNAL(about()),            this, SLOT(s_about()));
    QObject::connect(
        empath, SIGNAL(bugReport()),        this, SLOT(s_bugReport()));
    
    EmpathMainWindow * mainWindow = new EmpathMainWindow("mainWindow");
    kapp->setMainWidget(mainWindow);
}

EmpathUI::~EmpathUI()
{
}

    void    
EmpathUI::s_newComposer(Empath::ComposeType t, const EmpathURL & m)
{
    EmpathComposeWindow * c = new EmpathComposeWindow(t, m);
    CHECK_PTR(c);
    c->show();
}

    void    
EmpathUI::s_newComposer(const QString & recipient)
{
    EmpathComposeWindow * c = new EmpathComposeWindow(recipient);
    CHECK_PTR(c);
    c->show();
}

    void
EmpathUI::s_setupDisplay()
{
    EmpathDisplaySettingsDialog::create();
}
    void
EmpathUI::s_setupIdentity()
{
    EmpathIdentitySettingsDialog::create();
}

    void
EmpathUI::s_setupSending()
{
    EmpathSendingSettingsDialog::create();
}

    void
EmpathUI::s_setupComposing()
{
    EmpathComposeSettingsDialog::create();
}

    void
EmpathUI::s_setupAccounts()
{
    EmpathAccountsSettingsDialog::create();
}

    void
EmpathUI::s_setupFilters()
{
    EmpathFilterManagerDialog::create();
}

    void
EmpathUI::s_about()
{
    QMessageBox::information(
        (QWidget *)0,
        "And the crowd had no idea why.",
        i18n("Where were his ethics ?"),
        i18n("Where were his manners ?"));
}

    void
EmpathUI::s_bugReport()
{
    EmpathComposeWindow * c = new EmpathComposeWindow();
    CHECK_PTR(c);
    c->bugReport();
}

    void
EmpathUI::s_infoMessage(const QString & s)
{
    QWidgetList * l = QApplication::topLevelWidgets();
    
    if (l->isEmpty()) return;
    
    QWidgetListIt it(*l);
    
    for (; it.current(); ++it)
        if (it.current()->inherits("KTMainWindow"))
            ((KTMainWindow *)it.current())->statusBar()->message(s, 4000);
    
    delete l;
    l = 0;
}

    void
EmpathUI::s_sendEmail(const QString & name, const QString & email)
{
    empath->compose(name + " " + email);
}

    void
EmpathUI::s_setupWizard()
{
//    EmpathSetupWizard::create();
}

    void
EmpathUI::s_getSaveName(const EmpathURL & url)
{
    QString saveFilePath =
        KFileDialog::getSaveFileName(QString::null, QString::null, 0, 0);
    
    if (saveFilePath.isEmpty())
        return;
   
    empath->s_saveNameReady(url, saveFilePath);
}

    void
EmpathUI::s_configureMailbox(const EmpathURL & url, QWidget * w)
{
    empathDebug("");
    EmpathMailbox * mailbox = empath->mailbox(url);

    if (mailbox == 0)
        return;
 
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
            break;
    }
}


// vim:ts=4:sw=4:tw=78
