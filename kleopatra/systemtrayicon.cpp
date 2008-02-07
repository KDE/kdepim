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

#include "systemtrayicon.h"

#include <KIcon>
#include <KLocale>
#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KComponentData>

#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QCoreApplication>
#include <QProcess>
#include <QPointer>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <cassert>

using namespace boost;

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
    void slotActivated( ActivationReason reason=QSystemTrayIcon::Trigger );
    void slotEnableDisableActions();

private:
    QMenu menu;
    QAction openCertificateManagerAction;
    QAction aboutAction;
    QAction quitAction;

    QPointer<KAboutApplicationDialog> aboutDialog;

    QPointer<QWidget> mainWindow;
    QRect previousGeometry;
};

SystemTrayIcon::Private::Private( SystemTrayIcon * qq )
    : q( qq ),
      menu(),
      openCertificateManagerAction( i18n("&Open Certificate Manager..."), q ),
      aboutAction( i18n("&About %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      quitAction( i18n("&Shutdown Kleopatra"), q ),
      aboutDialog(),
      mainWindow(),
      previousGeometry()
{
    KDAB_SET_OBJECT_NAME( menu );
    KDAB_SET_OBJECT_NAME( openCertificateManagerAction );
    KDAB_SET_OBJECT_NAME( aboutAction );
    KDAB_SET_OBJECT_NAME( quitAction );

    connect( &openCertificateManagerAction, SIGNAL(triggered()), q, SLOT(slotActivated()) );
    connect( &aboutAction, SIGNAL(triggered()), q, SLOT(slotAbout()) );
    connect( &quitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()) );
    connect( q, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(slotActivated(QSystemTrayIcon::ActivationReason)) );

    menu.addAction( &openCertificateManagerAction );
    menu.addAction( &aboutAction );
    menu.addSeparator();
    menu.addAction( &quitAction );

    q->setContextMenu( &menu );

    slotEnableDisableActions();
}

SystemTrayIcon::Private::~Private() {}

SystemTrayIcon::SystemTrayIcon( QObject * p )
    : QSystemTrayIcon( KIcon( "kleopatra" ), p ), d( new Private( this ) )
{

}

SystemTrayIcon::~SystemTrayIcon() {}

void SystemTrayIcon::setMainWindow( QWidget * mw ) {
    if ( d->mainWindow )
        return;
    d->mainWindow = mw;
    mw->installEventFilter( this );
    d->slotEnableDisableActions();
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

void SystemTrayIcon::Private::slotActivated( ActivationReason reason ) {
    if ( reason != QSystemTrayIcon::Trigger )
        return;
    if ( !mainWindow ) {
        mainWindow = q->doCreateMainWindow();
        assert( mainWindow );
        if ( previousGeometry.isValid() )
            mainWindow->setGeometry( previousGeometry );
        mainWindow->installEventFilter( q );
    }
    if ( mainWindow->isMinimized() ) {
        mainWindow->showNormal();
        mainWindow->raise();
    } else if ( mainWindow->isVisible() ) {
        mainWindow->raise();
    } else {
        mainWindow->show();
    }
}

void SystemTrayIcon::Private::slotEnableDisableActions() {
    openCertificateManagerAction.setEnabled( !mainWindow || !mainWindow->isVisible() );
}

#include "moc_systemtrayicon.cpp"

