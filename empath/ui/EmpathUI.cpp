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
#include "EmpathConfig.h"

EmpathUI::EmpathUI()
    : QObject()
{
    empathDebug("ctor");
    
    qInitPngIO();
    
    KConfig * c(KGlobal::config());
    
    c->setGroup(EmpathConfig::GROUP_DISPLAY);
    
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
    empathDebug("dtor");
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
    QMessageBox::information((QWidget *)0, "Pine needles in your bed",
        i18n("Susie is a bitch. She wants to be spanked."), i18n("Administer spanking"));
#if 0
    KAboutDialog * about = new KAboutDialog;
    about->setLogo(KGlobal::iconLoader()->loadIcon("empath.png"));
    about->setAuthor(
        "Rik Hemsley", "rik@kde.org", "http://without.netpedia.net", "");
    about->addContributor("Dirk A. Mueller",    "", "", "");
    about->addContributor("Tybollt",            "", "", "");
    about->addContributor("Torsten Rahn",       "", "", "");
    about->addContributor("Stephen Pitts",      "", "", "");
    about->setVersion(EMPATH_VERSION_STRING);
    QObject::connect(
        about,  SIGNAL(sendEmail(const QString &, const QString &)),
        this,   SLOT(s_sendEmail(const QString &, const QString &)));
    about->show();
#endif
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
    
    for (; it.current(); ++it) {
        if (it.current()->inherits("KTMainWindow"))
            ((KTMainWindow *)it.current())->statusBar()->message(s, 4000);
    }
    
    delete l;
    l = 0;
}

    void
EmpathUI::s_sendEmail(const QString & name, const QString & email)
{
    empath->compose(name + " " + email);
}

// vim:ts=4:sw=4:tw=78
