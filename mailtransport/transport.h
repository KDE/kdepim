/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#ifndef KPIM_TRANSPORT_H
#define KPIM_TRANSPORT_H

#include <mailtransport/transportbase.h>
#include <mailtransport/mailtransport_export.h>

#include <kconfigbase.h>

namespace KPIM {

/**
  Represents the settings of a specific mail transport.

  To create a new empty Transport object, use TransportManager::createTransport().
*/
class MAILTRANSPORT_EXPORT Transport : public TransportBase
{
  friend class TransportManager;

  public:
    typedef QList<Transport*> List;

    /**
      Returns true if this transport is valid, ie. has all necessary data set.
    */
    bool isValid() const;

    /**
      Returns the password of this transport.
    */
    QString password();

    /**
      Sets the password of this transport.
      @param passwd The new password.
    */
    void setPassword( const QString &passwd );

    /**
      Returns true if all settings have been loaded.
      This is the way to find out if the password has already been loaded
      from the wallet.
    */
    bool isComplete() const;

    /**
      Returns a string representation of the authentication type.
    */
    QString authenticationTypeString() const;

    /**
      Returns true if this is a adhoc transport, ie. a non-perisistent transport
      configuration based on a url entered by the user.
    */
    bool isAdHoc() const;

    /**
      Returns a deep copy of this Transport object which will no longer be
      automatically updated. Use this if you need to store a Transport object
      over a longer time. However it is recommended to store transport identifiers
      instead if possible.
    */
    Transport* clone() const;

  protected:
    /**
      Creates a Transport object. Should only be used by TransportManager.
      @param cfgGroup The KConfig group to read its data from.
    */
    Transport( const QString &cfgGroup );

    virtual void usrReadConfig();
    virtual void usrWriteConfig();

    /**
      Returns true if the password was not stored in the wallet.
    */
    bool needsWalletMigration() const;

    /**
      Try to migrate the password from the config file to the wallet.
    */
    void migrateToWallet();

    /**
      Sets the adhoc flag.
    */
    void setAdHoc( bool b );

  private:
    void readPassword();

  private:
    QString mPassword;
    bool mPasswordLoaded;
    bool mPasswordDirty;
    bool mStorePasswordInFile;
    bool mNeedsWalletMigration;
    bool mIsAdHoc;
};

}

#endif
