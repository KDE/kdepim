/*******************************************************************************
**
** Filename   : akonadicontrol.cpp
** Created on : 27 May, 2006
** Copyright  : (c) 2006 Till Adam
** Email      : adam@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include "akonadicontrol.h"

AkonadiControl::AkonadiControl() 
{
    connect( &m_server, SIGNAL( error( QProcess::ProcessError ) ),
             SLOT( slotServerError( QProcess::ProcessError ) ) );
    connect( &m_server, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             SLOT( slotServerFinshed( int, QProcess::ExitStatus ) ) );

    startStorageServer();
}

AkonadiControl::~AkonadiControl()
{
}

void AkonadiControl::startStorageServer()
{
    m_server.start( "akonadi" );
    if ( !m_server.waitForStarted( ) ) {
      qDebug( ) << "Starting the server failed:" << m_server.errorString( );
      // FIXME
      // we only get here for non-fatal errors. Determine what to do.
      return;
    }
    qDebug( ) << "Akonadi StorageServer started!";
    sanityCheckStorageServer();

}


void AkonadiControl::sanityCheckStorageServer()
{
    // dbus ping?
    // IMAP test login?
}

void AkonadiControl::slotServerError( QProcess::ProcessError error )
{
    qDebug() << "Server error: " << error << endl;
    switch ( error ) {
        case QProcess::Crashed:
            // do nothing, we'll respawn
            break;
        case QProcess::FailedToStart:
        default:
            QCoreApplication::instance()->quit();
            break;
    }
}

void AkonadiControl::slotServerFinshed( int exitCode, QProcess::ExitStatus status )
{
    qDebug() << "Server finished: " << status << endl;
    // restart
    startStorageServer();
}

#include "akonadicontrol.moc"


