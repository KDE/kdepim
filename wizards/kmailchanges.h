/*
    This file is part of kdepim.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KMAILCHANGES_H
#define KMAILCHANGES_H

#include <kconfigpropagator.h>
#include <kconfig.h>

class CreateImapAccount : public KConfigPropagator::Change
{
  public:
    class CustomWriter
    {
      public:
        virtual ~CustomWriter() {}
        virtual void writeFolder( KConfig &, int folderId ) = 0;
        virtual void writeIds( int accountId, int transportId ) = 0;
    };

    CreateImapAccount( const QString &accountName, const QString &title );
    virtual ~CreateImapAccount();

    void setServer( const QString & );
    void setUser( const QString & );
    void setPassword( const QString & );
    void setRealName( const QString & );
    /**
      Set email. Default is "user@server".
    */
    void setEmail( const QString & );

    void setDefaultDomain( const QString & );

    void enableSieve( bool );
    void setSieveVacationFileName( const QString& );
    void enableSavePassword( bool );

    enum Encryption { None, SSL, TLS };

    void setEncryption( Encryption );

    enum Authentication { PLAIN, LOGIN };

    void setAuthenticationSend( Authentication );

    void setSmtpPort( int );

    void setExistingAccountId( int );
    void setExistingTransportId( int );

    /**
      Set custom writer. CreateImapAccount takes ownerhsip of the
      object.
    */
    void setCustomWriter( CustomWriter * );

  protected:
    QString mAccountName;

    QString mServer;
    QString mUser;
    QString mPassword;
    QString mRealName;
    QString mEmail;
    QString mDefaultDomain;

    QString mSieveVacationFileName;
    bool mEnableSieve;
    bool mEnableSavePassword;

    Encryption mEncryption;
    Authentication mAuthenticationSend;

    int mSmtpPort;

    int mExistingAccountId;
    int mExistingTransportId;

    CustomWriter *mCustomWriter;
};

class CreateDisconnectedImapAccount : public CreateImapAccount
{
  public:
    CreateDisconnectedImapAccount( const QString &accountName );
    virtual void apply();

    void enableLocalSubscription( bool b ) { mLocalSubscription = b; }

  private:
    bool mLocalSubscription;
};

class CreateOnlineImapAccount : public CreateImapAccount
{
  public:
    CreateOnlineImapAccount( const QString &accountName );
    virtual void apply();
};

#endif
