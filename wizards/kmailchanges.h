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

namespace KWallet {
  class Wallet;
}

class CreateImapAccount : public KConfigPropagator::Change
{
  public:
    class CustomWriter
    {
      public:
        virtual void writeFolder( KConfig &, int folderId ) = 0;
        virtual void writeIds( int accountId, int transportId ) = 0;
    };

    CreateImapAccount( const QString &accountName, const QString &title );
    ~CreateImapAccount();

    void setServer( const QString & );
    void setUser( const QString & );
    void setPassword( const QString & );
    void setRealName( const QString & );
    void setPort( int );
    /**
      Set email. Default is "user@server".
    */
    void setEmail( const QString & );

    void setDefaultDomain( const QString & );

    void enableSieve( bool );
    void setSieveVacationFileName( const QString& );
    void enableSavePassword( bool );

    enum Encryption { None, SSL, TLS };
    enum Authentication { NONE, PLAIN, LOGIN, NTLM_SPA, GSSAPI, DIGEST_MD5, CRAM_MD5 };

    void setEncryption( Encryption );
    void setAuthentication( Authentication );

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
    bool writeToWallet( const QString &type, int id );

  protected:
    QString mAccountName;

    QString mServer;
    QString mUser;
    QString mPassword;
    QString mRealName;
    int mPort;
    QString mEmail;
    QString mDefaultDomain;

    QString mSieveVacationFileName;
    bool mEnableSieve;
    bool mEnableSavePassword;

    Encryption mEncryption;
    Authentication mAuthentication;
    Authentication mAuthenticationSend;

    int mSmtpPort;

    int mExistingAccountId;
    int mExistingTransportId;

    CustomWriter *mCustomWriter;

  private:
    static KWallet::Wallet *mWallet;
};

class CreateDisconnectedImapAccount : public CreateImapAccount
{
  public:
    enum GroupwareType
    {
      GroupwareNone,
      GroupwareKolab,
      GroupwareScalix
    };

    CreateDisconnectedImapAccount( const QString &accountName );
    virtual void apply();

    void enableLocalSubscription( bool b ) { mLocalSubscription = b; }
    void setGroupwareType( GroupwareType type ) { mGroupwareType = type; }

  private:
    bool mLocalSubscription;
    GroupwareType mGroupwareType;
};

class CreateOnlineImapAccount : public CreateImapAccount
{
  public:
    CreateOnlineImapAccount( const QString &accountName );
    virtual void apply();
};

#endif
