/* -*- mode: c++; c-basic-offset:4 -*-
    systemtrayicon.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "systrayicon.h"
#include "mainwindow.h"

#include <utils/kdsignalblocker.h>

#include <commands/encryptclipboardcommand.h>
#include <commands/signclipboardcommand.h>
#include <commands/decryptverifyclipboardcommand.h>
#include <commands/setinitialpincommand.h>

#include <conf/configuredialog.h>

#include <KIcon>
#include <KLocale>
#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KComponentData>
#include <KWindowSystem>

#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QPointer>
#include <QDebug>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <cassert>

using namespace boost;
using namespace Kleo;
using namespace Kleo::Commands;

namespace {
    class LearnCertificatesCommand : public Kleo::Command {
    public:
        explicit LearnCertificatesCommand( KeyListController * ctrl )
            : Kleo::Command( ctrl ), m_dialog( 0 ) {}

        void doStart() {
            emit finished();
        }
        void doCancel() {}

        QDialog * dialog() {
            if ( !m_dialog ) {
                m_dialog = new QDialog;
                m_dialog->setAttribute( Qt::WA_DeleteOnClose );
                m_dialog->setWindowTitle( i18n("Learn Keys") );
                m_dialog->show();
            }
            return m_dialog;
        }
        QDialog * m_dialog;
    };
}

class SysTrayIcon::Private {
    friend class ::SysTrayIcon;
    SysTrayIcon * const q;
public:
    explicit Private( SysTrayIcon * qq );
    ~Private();

private:
    void slotAbout() {
        if ( !aboutDialog ) {
            aboutDialog = new KAboutApplicationDialog( KGlobal::mainComponent().aboutData() );
            aboutDialog->setAttribute( Qt::WA_DeleteOnClose );
        }

        if ( aboutDialog->isVisible() )
            aboutDialog->raise();
        else
            aboutDialog->show();
    }

    void enableDisableActions() {
        //work around a Qt bug (seen with Qt 4.4.0, Windows): QClipBoard->mimeData() triggers QClipboard::changed(),
        //triggering slotEnableDisableActions again
        const KDSignalBlocker blocker( QApplication::clipboard() );
        openCertificateManagerAction.setEnabled( !q->mainWindow() || !q->mainWindow()->isVisible() );
        encryptClipboardAction.setEnabled( EncryptClipboardCommand::canEncryptCurrentClipboard() );
        openPGPSignClipboardAction.setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
        smimeSignClipboardAction.setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
        decryptVerifyClipboardAction.setEnabled( DecryptVerifyClipboardCommand::canDecryptVerifyCurrentClipboard() );
        setInitialPinAction.setEnabled( anyCardHasNullPin );
        learnCertificatesAction.setEnabled( anyCardCanLearnKeys );

        q->setAttentionWanted( ( anyCardHasNullPin || anyCardCanLearnKeys ) && !q->attentionWindow() );
    }

    void startCommand( Command * cmd ) {
        assert( cmd );
        cmd->setParent( q->mainWindow() );
        cmd->start();
    }

    void slotEncryptClipboard() {
        startCommand( new EncryptClipboardCommand( 0 ) );
    }

    void slotOpenPGPSignClipboard() {
        startCommand( new SignClipboardCommand( GpgME::OpenPGP, 0 ) );
    }

    void slotSMIMESignClipboard() {
        startCommand( new SignClipboardCommand( GpgME::CMS, 0 ) );
    }

    void slotDecryptVerifyClipboard() {
        startCommand( new DecryptVerifyClipboardCommand( 0 ) );
    }

    void slotSetInitialPin() {
        SetInitialPinCommand * cmd = new SetInitialPinCommand;
        q->setAttentionWindow( cmd->dialog() );
        startCommand( cmd );
    }

    void slotLearnCertificates() {
        LearnCertificatesCommand * cmd = new LearnCertificatesCommand( 0 );
        q->setAttentionWindow( cmd->dialog() );
        startCommand( cmd );
    }

private:
    void connectConfigureDialog() {
        if ( configureDialog && q->mainWindow() )
            connect( configureDialog, SIGNAL(configCommitted()), q->mainWindow(), SLOT(slotConfigCommitted()) );
    }
    void disconnectConfigureDialog() {
        if ( configureDialog && q->mainWindow() )
            disconnect( configureDialog, SIGNAL(configCommitted()), q->mainWindow(), SLOT(slotConfigCommitted()) );
    }
    void connectMainWindow() {
        if ( q->mainWindow() )
            connect( q->mainWindow(), SIGNAL(configDialogRequested()), q, SLOT(openOrRaiseConfigDialog()) );
    }
    void disconnectMainWindow() {
        if ( q->mainWindow() )
            connect( q->mainWindow(), SIGNAL(configDialogRequested()), q, SLOT(openOrRaiseConfigDialog()) );
    }

private:
    bool anyCardHasNullPin;
    bool anyCardCanLearnKeys;

    QMenu menu;
    QAction openCertificateManagerAction;
    QAction configureAction;
    QAction aboutAction;
    QAction quitAction;
    QMenu clipboardMenu;
    QAction encryptClipboardAction;
    QAction smimeSignClipboardAction;
    QAction openPGPSignClipboardAction;
    QAction decryptVerifyClipboardAction;
    QMenu cardMenu;
    QAction setInitialPinAction;
    QAction learnCertificatesAction;

    QPointer<KAboutApplicationDialog> aboutDialog;
    QPointer<ConfigureDialog> configureDialog;

    QRect mainWindowPreviousGeometry;
};

SysTrayIcon::Private::Private( SysTrayIcon * qq )
    : q( qq ),
      anyCardHasNullPin( false ),
      anyCardCanLearnKeys( false ),
      menu(),
      openCertificateManagerAction( i18n("&Open Certificate Manager..."), q ),
      configureAction( i18n("&Configure %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      aboutAction( i18n("&About %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      quitAction( i18n("&Shutdown Kleopatra"), q ),
      clipboardMenu( i18n("Clipboard" ) ),
      encryptClipboardAction( i18n("Encrypt..."), q ),
      smimeSignClipboardAction( i18n("S/MIME-Sign..."), q ),
      openPGPSignClipboardAction( i18n("OpenPGP-Sign..."), q ),
      decryptVerifyClipboardAction( i18n("Decrypt/Verify..."), q ),
      cardMenu( i18n("SmartCard") ),
      setInitialPinAction( i18n("Set Initial PIN..."), q ),
      learnCertificatesAction( i18n("Learn Card Certificates..."), q ),
      aboutDialog(),
      mainWindowPreviousGeometry()
{
    q->setNormalIcon( KIcon( "kleopatra" ) );
    q->setAttentionIcon( KIcon( "smartcard" ) );

    KDAB_SET_OBJECT_NAME( menu );
    KDAB_SET_OBJECT_NAME( openCertificateManagerAction );
    KDAB_SET_OBJECT_NAME( configureAction );
    KDAB_SET_OBJECT_NAME( aboutAction );
    KDAB_SET_OBJECT_NAME( quitAction );
    KDAB_SET_OBJECT_NAME( clipboardMenu );
    KDAB_SET_OBJECT_NAME( encryptClipboardAction );
    KDAB_SET_OBJECT_NAME( smimeSignClipboardAction );
    KDAB_SET_OBJECT_NAME( openPGPSignClipboardAction );
    KDAB_SET_OBJECT_NAME( decryptVerifyClipboardAction );
    KDAB_SET_OBJECT_NAME( cardMenu );
    KDAB_SET_OBJECT_NAME( setInitialPinAction );
    KDAB_SET_OBJECT_NAME( learnCertificatesAction );

    connect( &openCertificateManagerAction, SIGNAL(triggered()), q, SLOT(openOrRaiseMainWindow()) );
    connect( &configureAction, SIGNAL(triggered()), q, SLOT(openOrRaiseConfigDialog()) );
    connect( &aboutAction, SIGNAL(triggered()), q, SLOT(slotAbout()) );
    connect( &quitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()) );
    connect( &encryptClipboardAction, SIGNAL(triggered()), q, SLOT(slotEncryptClipboard()) );
    connect( &smimeSignClipboardAction, SIGNAL(triggered()), q, SLOT(slotSMIMESignClipboard()) );
    connect( &openPGPSignClipboardAction, SIGNAL(triggered()), q, SLOT(slotOpenPGPSignClipboard()) );
    connect( &decryptVerifyClipboardAction, SIGNAL(triggered()), q, SLOT(slotDecryptVerifyClipboard()) );
    connect( &setInitialPinAction, SIGNAL(triggered()), q, SLOT(slotSetInitialPin()) );
    connect( &learnCertificatesAction, SIGNAL(triggered()), q, SLOT(slotLearnCertificates()) );

    connect( QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
             q, SLOT(slotEnableDisableActions()) );

    menu.addAction( &openCertificateManagerAction );
    menu.addAction( &configureAction );
    menu.addAction( &aboutAction );
    menu.addSeparator();
    menu.addMenu( &clipboardMenu );
    clipboardMenu.addAction( &encryptClipboardAction );
    clipboardMenu.addAction( &smimeSignClipboardAction );
    clipboardMenu.addAction( &openPGPSignClipboardAction );
    clipboardMenu.addAction( &decryptVerifyClipboardAction );
    menu.addSeparator();
    menu.addMenu( &cardMenu );
    cardMenu.addAction( &setInitialPinAction );
    cardMenu.addAction( &learnCertificatesAction );
    menu.addSeparator();
    menu.addAction( &quitAction );

    q->setContextMenu( &menu );
}

SysTrayIcon::Private::~Private() {}

SysTrayIcon::SysTrayIcon( QObject * p )
    : SystemTrayIcon( p ), d( new Private( this ) )
{
    KGlobal::ref();
    slotEnableDisableActions();
}

SysTrayIcon::~SysTrayIcon() {
    KGlobal::deref();
}

void SysTrayIcon::doMainWindowClosed( QWidget * mw ) {
    d->mainWindowPreviousGeometry = mw->geometry();
}

MainWindow * SysTrayIcon::mainWindow() const {
    return static_cast<MainWindow*>( SystemTrayIcon::mainWindow() );
}

QDialog * SysTrayIcon::attentionWindow() const {
    return static_cast<QDialog*>( SystemTrayIcon::attentionWindow() );
}

static void open_or_raise( QWidget * w ) {
    if ( w->isMinimized() ) {
        KWindowSystem::unminimizeWindow( w->winId());
        w->raise();
    } else if ( w->isVisible() ) {
        w->raise();
    } else {
        w->show();
    }
}

void SysTrayIcon::openOrRaiseMainWindow() {
    MainWindow * mw = mainWindow();
    if ( !mw ) {
        mw = new MainWindow;
        mw->setAttribute( Qt::WA_DeleteOnClose );
        if ( d->mainWindowPreviousGeometry.isValid() )
            mw->setGeometry( d->mainWindowPreviousGeometry );
        setMainWindow( mw );
        d->connectConfigureDialog();
        d->connectMainWindow();
    }
    open_or_raise( mw );
}

void SysTrayIcon::doActivated() {
    if ( d->anyCardHasNullPin )
        d->slotSetInitialPin();
    else if ( d->anyCardCanLearnKeys )
        d->slotLearnCertificates();
    else
        openOrRaiseMainWindow();
}

void SysTrayIcon::openOrRaiseConfigDialog() {
    if ( !d->configureDialog ) {
        d->configureDialog = new ConfigureDialog;
        d->configureDialog->setAttribute( Qt::WA_DeleteOnClose );
        d->connectConfigureDialog();
    }
    open_or_raise( d->configureDialog );
}

void SysTrayIcon::setAnyCardHasNullPin( bool on ) {
    if ( d->anyCardHasNullPin == on )
        return;
    d->anyCardHasNullPin = on;
    slotEnableDisableActions();
}

void SysTrayIcon::setAnyCardCanLearnKeys( bool on ) {
    if ( d->anyCardCanLearnKeys == on )
        return;
    d->anyCardCanLearnKeys = on;
    slotEnableDisableActions();
}

void SysTrayIcon::slotEnableDisableActions() {
    d->enableDisableActions();
}

#include "moc_systrayicon.cpp"

