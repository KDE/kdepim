/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef EXCHANGE_ACCOUNT_H
#define EXCHANGE_ACCOUNT_H

#include <qobject.h>
#include <qstring.h>

#include <kurl.h>

namespace KPIM {
	
class ExchangeAccount : public QObject {
    Q_OBJECT
  public:
    ExchangeAccount( QString& host, QString& account, QString& password );
    /** 
     Create a new account object, read data from group app data
     */
    ExchangeAccount( QString group );
    ~ExchangeAccount();

    void save( QString const& group );
    void load( QString const& group );

    QString const & host() { return mHost; }
    QString const & account() { return mAccount; }
    QString const & password() { return mPassword; }

    void setHost( QString host ) { mHost = host; }
    void setAccount( QString account ) { mAccount = account; }
    void setPassword( QString password ) { mPassword = password; }

    KURL baseURL();
    KURL calendarURL();

    //  Put authentication info in KDE password store for auto-authentication
    //  with later webdav access
    void authenticate();

  private:
    QString mHost;
    QString mAccount;
    QString mPassword;
};

}

#endif

