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
# pragma implementation "EmpathUI.h"
#endif

// Qt includes
#include <qstring.h>
#include <qwidgetlist.h>
#include <qapplication.h>

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

QString EmpathAboutText;

EmpathUI::EmpathUI()
    : QObject((QObject *)0L, "EmpathUI")
{
    // %1 == Version number.
    // %2+ == Names of contributors
    QString aboutTemplate = i18n(
        "<p>Empath -- Mail client for KDE</p>"
        "<p>Version: %1</p>"
        "<p>Program design and code:<ul><li>%2</li><li>%3</li></ul></p>"
        "<p>Graphics:<ul><li>%4</li></ul></p>");

    EmpathAboutText =
        aboutTemplate.arg("Under Development")
            .arg("Rik Hemsley (rikkus)")
            .arg("Wilco Greven")
            .arg("kraftw");

    // If no mailboxes are configured, then show the setup wizard.

    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_GENERAL);
    
    QStringList l = c->readListEntry(GEN_MAILBOX_LIST);
    
    if (l.isEmpty()) {
        EmpathSetupWizard wiz;
        wiz.exec();
    }

    _connectUp();

    EmpathMainWindow * w = new EmpathMainWindow;
    w->show();
    kapp->setMainWidget(w);
}

EmpathUI::~EmpathUI()
{
    // Empty.
}

    void    
EmpathUI::s_newComposer(EmpathComposer::Form composeForm)
{
    (new EmpathComposeWindow(composeForm))->show();
}

    void
EmpathUI::s_setup(Empath::SetupType t, QWidget * parent)
{
    switch (t) {

        case Empath::SetupDisplay:
            {
                EmpathDisplaySettingsDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;
            
        case Empath::SetupIdentity:
            {
                EmpathIdentitySettingsDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;
            
        case Empath::SetupComposing:
            {
                EmpathComposeSettingsDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;
        
        case Empath::SetupSending:
            {
                EmpathSendingSettingsDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;

        case Empath::SetupAccounts:
            {
                EmpathAccountsSettingsDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;

        case Empath::SetupFilters:
            {
                EmpathFilterManagerDialog d(parent);
                d.loadData();
                d.exec();
            }
            break;

        case Empath::SetupWizard:
            {
                EmpathSetupWizard wiz;
                wiz.exec();
            }

        default:
            empathDebug("Setup what ?");
            break;
    }
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
   
//    empath->s_saveNameReady(url, saveFilePath);
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

        case EmpathMailbox::POP3:
            {
                EmpathConfigPOP3Dialog d(url, w);
                d.exec();
            }
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
        empath, SIGNAL(newComposer(EmpathComposer::Form)),
        this,   SLOT(s_newComposer(EmpathComposer::Form)));
    
    QObject::connect(
        empath, SIGNAL(configureMailbox(const EmpathURL &, QWidget *)),
        this,   SLOT(s_configureMailbox(const EmpathURL &, QWidget *)));

    QObject::connect(
        empath, SIGNAL(setup(Empath::SetupType, QWidget *)),
        this,   SLOT(s_setup(Empath::SetupType, QWidget *)));
}

// vim:ts=4:sw=4:tw=78
