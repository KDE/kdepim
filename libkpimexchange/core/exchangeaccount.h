/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <kurl.h>
#include <kio/job.h>

namespace KPIM {
	
class ExchangeAccount : public QObject {
  Q_OBJECT
  public:
    ExchangeAccount( QString host, QString account, QString password );
    ExchangeAccount( QString host, QString account, QString mailbox, QString password );
    /** 
     Create a new account object, read data from group app data
     */
    ExchangeAccount( QString group );
    ~ExchangeAccount();

    void save( QString const& group );
    void load( QString const& group );

    QString const & host() { return mHost; }
    QString const & account() { return mAccount; }
    QString const & mailbox() { return mMailbox; }
    QString const & password() { return mPassword; }

    void setHost( QString host ) { mHost = host; }
    void setAccount( QString account ) { mAccount = account; }
    void setMailbox( QString mailbox ) { mMailbox = mailbox; }
    void setPassword( QString password ) { mPassword = password; }

    KURL baseURL();
    KURL calendarURL();

    // Returns the mailbox URL of this user. QString::null if unsuccessful
    static QString tryFindMailbox( const QString& host, const QString& user, const QString& password );

    //  Put authentication info in KDE password store for auto-authentication
    //  with later webdav access. Also calculates the calendar URL.
    void authenticate();
    void authenticate( QWidget* window );

  private:
    void authenticate( int windowId );
    void calcFolderURLs();

  private slots:
    void slotFolderResult( KIO::Job * );

  private:
    QString mHost;
    QString mAccount;
    QString mMailbox;
    QString mPassword;

    KURL* mCalendarURL;
};

}

#endif

