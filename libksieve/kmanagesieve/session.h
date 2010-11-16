/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMANAGESIEVE_SESSION_H
#define KMANAGESIEVE_SESSION_H

#include <KUrl>
#include <QtCore/QObject>
#include <QtCore/QQueue>

class KTcpSocket;

namespace KManageSieve {

class SieveJob;

/** A network session with a manage sieve server.
 * @internal
 */
class Session : public QObject
{
  Q_OBJECT
  public:
    explicit Session( QObject *parent = 0 );
    ~Session();

    void connectToHost( const KUrl &url );
    void disconnectFromHost();

    void scheduleJob( SieveJob* job );

  private slots:
    void dataReceived();
    void socketError();

  private:
    KTcpSocket *m_socket;
    QQueue<SieveJob*> m_jobs;
};

}

#endif
