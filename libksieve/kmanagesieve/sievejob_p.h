/*  -*- c++ -*-
    sievejob.h

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KMANAGESIEVE_SIEVEJOB_P_H
#define KSIEVE_KMANAGESIEVE_SIEVEJOB_P_H

#include "sievejob.h"
#include "session.h"

#include <QUrl>
#include <QtCore/QPointer>

namespace KManageSieve
{

class SieveJob::Private
{
public:
    Private(SieveJob *qq)
        : q(qq), mFileExists(DontKnow), mInteractive(true)
    {
    }

    enum Command {
        Get,
        Put,
        Activate,
        Deactivate,
        SearchActive,
        List,
        Delete
    };

    enum Existence {
        DontKnow,
        Yes,
        No
    };

    static Session *sessionForUrl(const QUrl &url);

    void run(Session *session);
    bool handleResponse(const Response &response, const QByteArray &data);
    void killed();

    SieveJob *q;
    QUrl mUrl;
    QString mScript;
    QString mActiveScriptName;
    Existence mFileExists;
    QStack<Command> mCommands;

    // List of Sieve scripts on the server, used by @ref list()
    QStringList mAvailableScripts;

    bool mInteractive;

    static QHash<QUrl, QPointer<Session> > m_sessionPool;
};

}

#endif
