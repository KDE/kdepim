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
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.

*/
#ifndef EXCHANGE_ACCOUNT_H
#define EXCHANGE_ACCOUNT_H

#include <qobject.h>
#include <qstring.h>

#include <kdepimmacros.h>
#include <kurl.h>
#include <kio/job.h>

namespace KPIM {
	
class KDE_EXPORT ExchangeAccount : public QObject
{
    Q_OBJECT
  public:
    ExchangeAccount( const QString &host, const QString &port,
                     const QString &account, const QString &password,
                     const QString &mailbox = QString::null );
    /** 
     Create a new account object, read data from group app data
     */
    ExchangeAccount( const QString &group );
    ~ExchangeAccount();

    void save( QString const &group );
    void load( QString const &group );

    QString host() { return mHost; }
    QString port() { return mPort; }
    QString account() { return mAccount; }
    QString mailbox() { return mMailbox; }
    QString password() { return mPassword; }

    void setHost( QString host ) { mHost = host; }
    void setPort( QString port ) { mPort = port; }
    void setAccount( QString account ) { mAccount = account; }
    void setMailbox( QString mailbox ) { mMailbox = mailbox; }
    void setPassword( QString password ) { mPassword = password; }

    KURL baseURL();
    KURL calendarURL();

    // Returns the mailbox URL of this user. QString::null if unsuccessful
    static QString tryFindMailbox( const QString &host, const QString &port,
                                   const QString &user,
                                   const QString &password );

    //  Put authentication info in KDE password store for auto-authentication
    //  with later webdav access. Also calculates the calendar URL.
    bool authenticate();
    bool authenticate( QWidget *window );

  private:
    bool authenticate( int windowId );
    void calcFolderURLs();
    static QString tryMailbox( const QString &_url, const QString &user,
                               const QString &password );

  private slots:
    void slotFolderResult( KIO::Job * );

  private:
    QString mHost;
    QString mPort;
    QString mAccount;
    QString mMailbox;
    QString mPassword;

    KURL *mCalendarURL;
    bool mError;
};

}

#endif

