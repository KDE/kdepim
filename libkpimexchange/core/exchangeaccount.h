/*
    This file is part of libkpimexchange.

    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
 
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
#ifndef EXCHANGE_ACCOUNT_H
#define EXCHANGE_ACCOUNT_H

#include <tqobject.h>
#include <tqstring.h>

#include <kdepimmacros.h>
#include <kurl.h>
#include <kio/job.h>

namespace KPIM {
	
class KDE_EXPORT ExchangeAccount : public QObject
{
    Q_OBJECT
  public:
    ExchangeAccount( const TQString &host, const TQString &port,
                     const TQString &account, const TQString &password,
                     const TQString &mailbox = TQString::null );
    /** 
     Create a new account object, read data from group app data
     */
    ExchangeAccount( const TQString &group );
    ~ExchangeAccount();

    void save( TQString const &group );
    void load( TQString const &group );

    TQString host() { return mHost; }
    TQString port() { return mPort; }
    TQString account() { return mAccount; }
    TQString mailbox() { return mMailbox; }
    TQString password() { return mPassword; }

    void setHost( TQString host ) { mHost = host; }
    void setPort( TQString port ) { mPort = port; }
    void setAccount( TQString account ) { mAccount = account; }
    void setMailbox( TQString mailbox ) { mMailbox = mailbox; }
    void setPassword( TQString password ) { mPassword = password; }

    KURL baseURL();
    KURL calendarURL();

    // Returns the mailbox URL of this user. TQString::null if unsuccessful
    static TQString tryFindMailbox( const TQString &host, const TQString &port,
                                   const TQString &user,
                                   const TQString &password );

    //  Put authentication info in KDE password store for auto-authentication
    //  with later webdav access. Also calculates the calendar URL.
    bool authenticate();
    bool authenticate( TQWidget *window );

  private:
    bool authenticate( int windowId );
    void calcFolderURLs();
    static TQString tryMailbox( const TQString &_url, const TQString &user,
                               const TQString &password );

  private slots:
    void slotFolderResult( KIO::Job * );

  private:
    TQString mHost;
    TQString mPort;
    TQString mAccount;
    TQString mMailbox;
    TQString mPassword;

    KURL *mCalendarURL;
    bool mError;
};

}

#endif

