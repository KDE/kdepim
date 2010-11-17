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

#ifndef KMANAGESIEVE_SIEVEJOB_P_H
#define KMANAGESIEVE_SIEVEJOB_P_H

#include "sievejob.h"
#include "session.h"

#include <kdebug.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/jobuidelegate.h>
#include <kjobtrackerinterface.h>
#include <KUrl>

#include <QtCore/QPointer>
#include <QtCore/QTextCodec>

using KIO::Job;
using KIO::UDSEntryList;
using KIO::UDSEntry;

namespace KManageSieve {

class SieveJob::Private
{
  public:
    Private( SieveJob *qq )
      : q( qq ), mJob( 0 ), mDecoder( 0 ), mFileExists( DontKnow )
    {
    }

    ~Private()
    {
      delete mDecoder;
    }

    void slotData( KIO::Job*, const QByteArray& );
    void slotDataReq( KIO::Job*, QByteArray& );
    void slotEntries( KIO::Job*, const KIO::UDSEntryList& );
    void slotResult( KJob* );

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

    void schedule( Command command );
    static Session* sessionForUrl( const KUrl &url );

    void run( Session *session );
    bool handleResponse( const Response &response, const QByteArray &data );

    SieveJob *q;
    KUrl mUrl;
    KIO::Job * mJob;
    QTextDecoder * mDecoder;
    QString mScript;
    QString mActiveScriptName;
    Existence mFileExists;
    QStack<Command> mCommands;

    // List of Sieve scripts on the server, used by @ref list()
    QStringList mAvailableScripts;

    static QHash<KUrl, QPointer<Session> > m_sessionPool;
};

}

#endif
