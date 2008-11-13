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

#include "systemtrayicon.h"

#include <commands/encryptclipboardcommand.h>
#include <commands/signclipboardcommand.h>
#include <commands/decryptverifyclipboardcommand.h>

#include <conf/configuredialog.h>

#include <KIcon>
#include <KLocale>
#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KComponentData>
#include <KWindowSystem>

#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QApplication>
#include <QClipboard>
#include <QProcess>
#include <QPointer>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <cassert>

using namespace boost;
using namespace Kleo;
using namespace Kleo::Commands;

namespace {
    class SignalBlocker {
    public:
        explicit SignalBlocker( QObject * obj ) : object( obj ), previous( object && object->blockSignals( true ) ) {
            assert( object );
        }

        ~SignalBlocker() {
            if ( object )
                object->blockSignals( previous );
        }
    private:
        const QPointer<QObject> object;
        const bool previous;
    };
}

class SystemTrayIcon::Private {
    friend class ::SystemTrayIcon;
    SystemTrayIcon * const q;
public:
    explicit Private( SystemTrayIcon * qq );
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
    void slotActivated( ActivationReason reason ) {
        if ( reason == QSystemTrayIcon::Trigger )
            q->openOrRaiseMainWindow();
    }

    void slotEnableDisableActions() {
        //work around a Qt bug (seen with Qt 4.4.0, Windows): QClipBoard->mimeData() triggers QClipboard::changed(),
        //triggering slotEnableDisableActions again
        const SignalBlocker block( QApplication::clipboard() );
        openCertificateManagerAction.setEnabled( !mainWindow || !mainWindow->isVisible() );
        encryptClipboardAction.setEnabled( EncryptClipboardCommand::canEncryptCurrentClipboard() );
        openPGPSignClipboardAction.setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
        smimeSignClipboardAction.setEnabled( SignClipboardCommand::canSignCurrentClipboard() );
        decryptVerifyClipboardAction.setEnabled( DecryptVerifyClipboardCommand::canDecryptVerifyCurrentClipboard() );
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

private:
    void connectConfigureDialog() {
        if ( configureDialog && mainWindow )
            connect( configureDialog, SIGNAL(configCommitted()), mainWindow, SLOT(slotConfigCommitted()) );
    }
    void disconnectConfigureDialog() {
        if ( configureDialog && mainWindow )
            disconnect( configureDialog, SIGNAL(configCommitted()), mainWindow, SLOT(slotConfigCommitted()) );
    }
    void connectMainWindow() {
        if ( mainWindow )
            connect( mainWindow, SIGNAL(configDialogRequested()), q, SLOT(openOrRaiseConfigDialog()) );
    }
    void disconnectMainWindow() {
        if ( mainWindow )
            connect( mainWindow, SIGNAL(configDialogRequested()), q, SLOT(openOrRaiseConfigDialog()) );
    }
private:
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

    QPointer<KAboutApplicationDialog> aboutDialog;
    QPointer<ConfigureDialog> configureDialog;

    QPointer<QWidget> mainWindow;
    QRect previousGeometry;
};

SystemTrayIcon::Private::Private( SystemTrayIcon * qq )
    : q( qq ),
      menu(),
      openCertificateManagerAction( i18n("&Open Certificate Manager..."), q ),
      configureAction( i18n("&Configure %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      aboutAction( i18n("&About %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      quitAction( i18n("&Quit Kleopatra"), q ),
      clipboardMenu( i18n("Clipboard" ) ),
      encryptClipboardAction( i18n("Encrypt..."), q ),
      smimeSignClipboardAction( i18n("S/MIME-Sign..."), q ),
      openPGPSignClipboardAction( i18n("OpenPGP-Sign..."), q ),
      decryptVerifyClipboardAction( i18n("Decrypt/Verify..."), q ),
      aboutDialog(),
      mainWindow(),
      previousGeometry()
{
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

    connect( &openCertificateManagerAction, SIGNAL(triggered()), q, SLOT(openOrRaiseMainWindow()) );
    connect( &configureAction, SIGNAL(triggered()), q, SLOT(openOrRaiseConfigDialog()) );
    connect( &aboutAction, SIGNAL(triggered()), q, SLOT(slotAbout()) );
    connect( &quitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()) );
    connect( &encryptClipboardAction, SIGNAL(triggered()), q, SLOT(slotEncryptClipboard()) );
    connect( &smimeSignClipboardAction, SIGNAL(triggered()), q, SLOT(slotSMIMESignClipboard()) );
    connect( &openPGPSignClipboardAction, SIGNAL(triggered()), q, SLOT(slotOpenPGPSignClipboard()) );
    connect( &decryptVerifyClipboardAction, SIGNAL(triggered()), q, SLOT(slotDecryptVerifyClipboard()) );

    connect( QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
             q, SLOT(slotEnableDisableActions()) );

    connect( q, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(slotActivated(QSystemTrayIcon::ActivationReason)) );

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
    menu.addAction( &quitAction );

    q->setContextMenu( &menu );

    slotEnableDisableActions();
}

SystemTrayIcon::Private::~Private() {}

SystemTrayIcon::SystemTrayIcon( QObject * p )
    : QSystemTrayIcon( KIcon( "kleopatra" ), p ), d( new Private( this ) )
{
    KGlobal::ref();
}

SystemTrayIcon::~SystemTrayIcon() {
    KGlobal::deref();
}

void SystemTrayIcon::setMainWindow( QWidget * mw ) {
    if ( d->mainWindow )
        return;
    d->disconnectConfigureDialog();
    d->disconnectMainWindow();
    d->mainWindow = mw;
    d->connectConfigureDialog();
    d->connectMainWindow();
    mw->installEventFilter( this );
    d->slotEnableDisableActions();
}

QWidget * SystemTrayIcon::mainWindow() const {
    return d->mainWindow;
}

bool SystemTrayIcon::eventFilter( QObject * o, QEvent * e ) {
    if ( o == d->mainWindow )
        switch ( e->type() ) {
        case QEvent::Close:
            d->previousGeometry = static_cast<QWidget*>( o )->geometry();
            // fall through:
        case QEvent::Show:
        case QEvent::DeferredDelete:
            QMetaObject::invokeMethod( this, "slotEnableDisableActions", Qt::QueuedConnection );
        default: ;
        }
    return false;
}

void SystemTrayIcon::openOrRaiseMainWindow() {
    if ( !d->mainWindow ) {
        d->mainWindow = doCreateMainWindow();
        assert( d->mainWindow );
        if ( d->previousGeometry.isValid() )
            d->mainWindow->setGeometry( d->previousGeometry );
        d->mainWindow->installEventFilter( this );
        d->connectConfigureDialog();
        d->connectMainWindow();
    }
    if ( d->mainWindow->isMinimized() ) {
        KWindowSystem::unminimizeWindow( d->mainWindow->winId());
        d->mainWindow->raise();
    } else if ( d->mainWindow->isVisible() ) {
        d->mainWindow->raise();
    } else {
        d->mainWindow->show();
    }
}

void SystemTrayIcon::openOrRaiseConfigDialog() {
    if ( !d->configureDialog ) {
        d->configureDialog = new ConfigureDialog;
        d->configureDialog->setAttribute( Qt::WA_DeleteOnClose );
        d->connectConfigureDialog();
    }

    if ( d->configureDialog->isVisible() )
        d->configureDialog->raise();
    else
        d->configureDialog->show();
}

#include "moc_systemtrayicon.cpp"

