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
    void slotOpenCertificateManager() {
        emit q->activated( QSystemTrayIcon::Trigger );
    }
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
    void slotCheckConfiguration();

private:    
    QMenu menu;
    QAction openCertificateManagerAction;
    QAction aboutAction;
    QAction checkConfigAction;
    QAction quitAction;

    QPointer<KAboutApplicationDialog> aboutDialog;
};

SystemTrayIcon::Private::Private( SystemTrayIcon * qq )
    : q( qq ),
      menu(),
      openCertificateManagerAction( i18n("&Open Certificate Manager..."), q ),
      aboutAction( i18n("&About %1...", KGlobal::mainComponent().aboutData()->programName() ), q ),
      checkConfigAction( i18n("&Check GnuPG Config..."), q ),
      quitAction( i18n("&Shutdown Kleopatra"), q ),
      aboutDialog()
{
    KDAB_SET_OBJECT_NAME( menu );
    KDAB_SET_OBJECT_NAME( openCertificateManagerAction );
    KDAB_SET_OBJECT_NAME( aboutAction );
    KDAB_SET_OBJECT_NAME( checkConfigAction );
    KDAB_SET_OBJECT_NAME( quitAction );

    connect( &openCertificateManagerAction, SIGNAL(triggered()), q, SLOT(slotOpenCertificateManager()) );
    connect( &aboutAction, SIGNAL(triggered()), q, SLOT(slotAbout()) );
    connect( &quitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()) );
    connect( &checkConfigAction, SIGNAL(triggered()), q, SLOT(slotCheckConfiguration()) );

    menu.addAction( &openCertificateManagerAction );
    menu.addAction( &aboutAction );
    menu.addSeparator();
    menu.addAction( &checkConfigAction );
    menu.addSeparator();
    menu.addAction( &quitAction );

    q->setContextMenu( &menu );
}

SystemTrayIcon::Private::~Private() {}

SystemTrayIcon::SystemTrayIcon( QObject * p )
    : QSystemTrayIcon( KIcon( "kleopatra" ), p ), d( new Private( this ) )
{

}

SystemTrayIcon::~SystemTrayIcon() {}

void SystemTrayIcon::Private::slotCheckConfiguration() {
    assert( checkConfigAction.isEnabled() );
    if ( !checkConfigAction.isEnabled() )
        return;

    checkConfigAction.setEnabled( false );
    const shared_ptr<QAction> enabler( &checkConfigAction, bind( &QAction::setEnabled, _1, true ) );

    // 1. start process
    QProcess process;
    process.setProcessChannelMode( QProcess::MergedChannels );
    process.start( "gpgconf", QStringList() << "--check-config", QIODevice::ReadOnly );


    // 2. show dialog:
    QDialog dlg;
    QVBoxLayout vlay( &dlg );
    QLabel label( i18n("This is the result of the GnuPG config check:" ), &dlg );
    QTextEdit textEdit( &dlg );
    QDialogButtonBox box( QDialogButtonBox::Close, Qt::Horizontal, &dlg );

    textEdit.setReadOnly( true );
    textEdit.setWordWrapMode( QTextOption::NoWrap );

    vlay.addWidget( &label );
    vlay.addWidget( &textEdit, 1 );
    vlay.addWidget( &box );

    dlg.show();

    connect( box.button( QDialogButtonBox::Close ), SIGNAL(clicked()), &dlg, SLOT(reject()) );
    connect( &dlg, SIGNAL(finished(int)), &process, SLOT(terminate()) );

    // 3. wait for either dialog close or process exit
    QEventLoop loop;
    connect( &process, SIGNAL(finished(int,QProcess::ExitStatus)), &loop, SLOT(quit()) );
    connect( &dlg, SIGNAL(finished(int)), &loop, SLOT(quit()) );

    const QPointer<QObject> Q( q );
    loop.exec();

    // safety exit:
    if ( !Q )
        return;

    // check whether it was the dialog that was closed, and return in
    // that case:
    if ( !dlg.isVisible() )
        return;

    if ( process.error() != QProcess::UnknownError )
        textEdit.setPlainText( QString::fromUtf8( process.readAll() ) + '\n' + process.errorString() );
    else
        textEdit.setPlainText( QString::fromUtf8( process.readAll() ) );

    // wait for dialog close:
    assert( dlg.isVisible() );
    loop.exec();
}

#include "moc_systemtrayicon.cpp"

