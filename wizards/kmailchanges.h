/*
    This file is part of kdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KMAILCHANGES_H
#define KMAILCHANGES_H

#include <kconfigpropagator.h>
#include <kconfig.h>

class CreateDisconnectedImapAccount : public KConfigPropagator::Change
{
  public:
    class CustomWriter
    {
      public:
        virtual void write( KConfig &, int accountId ) = 0;
    };
  
    CreateDisconnectedImapAccount( const QString &accountName );
    ~CreateDisconnectedImapAccount();

    void apply();

    void setServer( const QString & );
    void setUser( const QString & );
    void setPassword( const QString & );
    void setRealName( const QString & );
    /**
      Set email. Default is "user@server".
    */
    void setEmail( const QString & );

    void enableSieve( bool );
    void enableSavePassword( bool );

    enum Encryption { None, SSL, TLS };

    void setEncryption( Encryption );

    enum Authentication { PLAIN, LOGIN };

    void setAuthenticationSend( Authentication );

    void setSmtpPort( int );

    /**
      Set custom writer. CreateDisconnectedImapAccount takes ownerhsip of the
      object.
    */
    void setCustomWriter( CustomWriter * );

  private:
    QString mAccountName;

    QString mServer;
    QString mUser;
    QString mPassword;
    QString mRealName;
    QString mEmail;

    bool mEnableSieve;
    bool mEnableSavePassword;

    Encryption mEncryption;
    Authentication mAuthenticationSend;

    int mSmtpPort;

    CustomWriter *mCustomWriter;
};

#endif
