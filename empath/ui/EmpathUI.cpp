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

// Qt includes
#include <qstring.h>
#include <qwidgetlist.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <klocale.h>
#include <ktmainwindow.h>
#include <kaboutdialog.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kurl.h>

// Local includes
#include "Empath.h"
#include "EmpathUI.h"
#include "EmpathMainWindow.h"
#include "EmpathComposeWindow.h"
#include "EmpathMailbox.h"

EmpathUI * EmpathUI::instance_ = 0L;

EmpathUI::EmpathUI()
    : QObject((QObject *)0L, "EmpathUI")
{
    _init();

   (void) new EmpathMainWindow;
}

EmpathUI::~EmpathUI()
{
    // Empty.
}

    void    
EmpathUI::s_newComposer(const EmpathComposeForm & form)
{
    EmpathComposeWindow * w = new EmpathComposeWindow;

    w->setComposeForm(form);
}

    void
EmpathUI::s_setup(Empath::SetupType t, QWidget * /* parent */)
{
    switch (t) {

        default:
            empathDebug("STUB");
            break;
    }
}

    void
EmpathUI::s_getSaveName(const EmpathURL & /* url */, QWidget * /* parent */)
{
#if 0
    KURL saveFilePath =
        KFileDialog::getSaveURL(QString::null, QString::null, parent);
    
    if (saveFilePath.isEmpty())
        return;
   
    empath->s_saveNameReady(url, saveFilePath);
#endif
}

    void
EmpathUI::s_configureMailbox(const EmpathURL & url, QWidget * /* w */)
{
    EmpathMailbox * mailbox = empath->mailbox(url);

    if (mailbox == 0) {
        empathDebug("Cannot find mailbox with url `" + url.asString() + "'");
        return;
    }
 
    switch (mailbox->type()) {

        default:
            empathDebug("STUB");
            break;
    }
}

    void
EmpathUI::_init()
{
    _initActions();
    _connectUp();
    _showWizardIfNeeded();
}

    void
EmpathUI::_showWizardIfNeeded()
{
    // If no mailboxes are configured, then show the setup wizard.

    KConfig * c(KGlobal::config());
    
    c->setGroup("General");
    
    if (c->readListEntry("MailboxList").isEmpty())
        s_setup(Empath::SetupWizard, static_cast<QWidget *>(0L));
}

    void
EmpathUI::_connectUp()
{
    QObject::connect(
        empath, SIGNAL(getSaveName(const EmpathURL &, QWidget *)),
        this,   SLOT(s_getSaveName(const EmpathURL &, QWidget *)));
    
    QObject::connect(
        empath, SIGNAL(newComposer(const EmpathComposeForm &)),
        this,   SLOT(s_newComposer(const EmpathComposeForm &)));
    
    QObject::connect(
        empath, SIGNAL(configureMailbox(const EmpathURL &, QWidget *)),
        this,   SLOT(s_configureMailbox(const EmpathURL &, QWidget *)));

    QObject::connect(
        empath, SIGNAL(setup(Empath::SetupType, QWidget *)),
        this,   SLOT(s_setup(Empath::SetupType, QWidget *)));
}

    void
EmpathUI::_initActions()
{
    actionCollection_ = new KActionCollection(this, "actionCollection");

    ac_messageCompose_ =
        new KAction(
            i18n("&Compose"),
            QIconSet(BarIcon("compose")),
            Key_M, 
            empath,
            SLOT(s_compose()),
            actionCollection_,
            "messageCompose"
        );
}

    void
EmpathUI::s_showFolder(const EmpathURL & url, unsigned int id)
{
    emit(showFolder(url, id));
}

// vim:ts=4:sw=4:tw=78
#include "EmpathUI.moc"
