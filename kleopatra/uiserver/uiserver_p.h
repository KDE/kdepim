/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/uiserver_p.h

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
#ifndef __KLEOPATRA_UISERVER_UISERVER_P_H__
#define __KLEOPATRA_UISERVER_UISERVER_P_H__

#include "uiserver.h"

#include "assuanserverconnection.h"
#include "assuancommand.h"

#include <utils/wsastarter.h>

#include <QTcpServer>
#include <QFile>

#include <kleo-assuan.h>

#include <boost/bind.hpp>

#include <algorithm>
#include <vector>

namespace
{
template <typename Ex>
void throw_(const QString &message)
{
    throw Ex(message.toUtf8().constData());
}
}

namespace Kleo
{

class UiServer::Private : public QTcpServer
{
    Q_OBJECT
    friend class ::Kleo::UiServer;
    UiServer *const q;
public:
    explicit Private(UiServer *qq);
    static bool isStaleAssuanSocket(const QString &socketName);

private:
    void makeListeningSocket();
    // platform-specific creation impl for makeListeningSocket():
    void doMakeListeningSocket(const QByteArray &encodedFileName);
    QString makeFileName(const QString &hint = QString()) const;
    void ensureDirectoryExists(const QString &path) const;
    static QString systemErrorString();

protected:
    /* reimp */ void incomingConnection(int fd);

private Q_SLOTS:
    void slotConnectionClosed(Kleo::AssuanServerConnection *conn);

private:
    QFile file;
    std::vector< boost::shared_ptr<AssuanCommandFactory> > factories;
    std::vector< boost::shared_ptr<AssuanServerConnection> > connections;
    QString suggestedSocketName;
    QString actualSocketName;
    assuan_sock_nonce_t nonce;
    const WSAStarter _wsastarter;
    bool cryptoCommandsEnabled;
};

}

#endif /* __KLEOPATRA_UISERVER_UISERVER_P_H__ */
