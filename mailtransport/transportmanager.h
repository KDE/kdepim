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

#ifndef KPIM_TRANSPORTMANAGER_H
#define KPIM_TRANSPORTMANAGER_H

#include <mailtransport/mailtransport_export.h>

#include <QList>
#include <QObject>

class KConfig;
class KJob;

namespace KWallet {
  class Wallet;
}

namespace KPIM {

class Transport;
class TransportJob;

/**
  Takes care of loading and storing mail transport settings and
  creating of transport jobs.
*/
class MAILTRANSPORT_EXPORT TransportManager : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.pim.TransportManager")

  friend class Transport;

  public:
    /**
      Destructor.
    */
    virtual ~TransportManager();

    /**
      Returns the TransportManager instance.
    */
    static TransportManager* self();

    /**
      Tries to load passwords asynchronously from KWallet if needed.
      The passwordsChanged() signal is emitted once the passwords have been loaded.
      Nothing happens if the passwords were already available.
    */
    void loadPasswordsAsync();

    /**
      Returns the Transport object with the given id.
      @param id The identifier of the Transport.
      @param def if set to true, the default transport will be returned if the
      specified Transport object could not be found, 0 otherwise.
      @returns A Transport object for immediate use. It might become invalid as
      soon as the event loop is entered again due to remote changes. If you need
      to store a Transport object, store the transport identifier instead.
    */
    Transport* transportById( int id, bool def = true ) const;

    /**
      Returns the transport object with the given name.
      @param name The transport name.
      @param def if set to true, the default transport will be returned if the
      specified Transport object could not be found, 0 otherwise.
      @returns A Transport object for immediate use, see transportById() for
      limitations.
    */
    Transport* transportByName( const QString &name, bool def = true ) const;

    /**
      Returns a list of all available transports.
      Note: The Transport objects become invalid as soon as a change occur, so
      they are only suitable for immediate use.
    */
    QList<Transport*> transports() const;

    /**
      Creates a new, empty Transport object. The object is owned by the caller.
      If you want to add the Transport permanently (eg. after configuring it)
      call addTransport().
    */
    Transport* createTransport() const;

    /**
      Adds the given transport. The object ownership is transferred to
      TransportMananger, ie. you must not delete @p transport.
      @param transport The Transport object to add.
    */
    void addTransport( Transport* transport );

    /**
      Creates a mail transport job for the given transport identifier.
      Returns 0 if the specified transport is invalid.
      @param transportId The transport identifier.
    */
    TransportJob* createTransportJob( int transportId );

    /**
      Executes the given transport job. This is the preferred way to start
      transport jobs. It takes care of asynchronously loading passwords from
      KWallet if necessary.
      @param job The completely configured transport job to execute.
    */
    void schedule( TransportJob* job );

  public slots:
    /**
      Returns true if there are no mail transports at all.
    */
    Q_SCRIPTABLE bool isEmpty() const;

    /**
      Returns a list of transport identifiers.
    */
    Q_SCRIPTABLE QList<int> transportIds() const;

    /**
      Returns a list of transport names.
    */
    Q_SCRIPTABLE QStringList transportNames() const;

    /**
      Returns the default transport name.
    */
    Q_SCRIPTABLE QString defaultTransportName() const;

    /**
      Returns the default transport identifier.
      Invalid if there are no transports at all.
    */
    Q_SCRIPTABLE int defaultTransportId() const;

    /**
      Sets the default transport. The change will be in effect immediately.
      @param id The identifier of the new default transport.
    */
    Q_SCRIPTABLE void setDefaultTransport( int id );

    /**
      Deletes the specified transport.
      @param id The identifier of the mail transport to remove.
    */
    Q_SCRIPTABLE void removeTransport( int id );

  signals:
    /**
      Emitted when transport settings have changed (by this or any other
      TransportManager instance).
    */
    Q_SCRIPTABLE void transportsChanged();

    /**
      Internal signal to synchronize all TransportManager instances.
      This signal is emitted by the instance writing the changes.
      You probably want to use transportsChanged() instead.
    */
    Q_SCRIPTABLE void changesCommitted();

    /**
      Emitted when passwords have been loaded from the wallet.
    */
    void passwordsChanged();

  protected:
    /**
      Returns a pointer to an open wallet if available, 0 otherwise.
      The wallet is opened synchronously if necessary.
    */
    KWallet::Wallet* wallet();

    /**
      Loads all passwords synchronously.
    */
    void loadPasswords();

  private:
    TransportManager();
    void readConfig();
    void writeConfig();
    void emitChangesCommitted();
    int createId() const;
    void prepareWallet();
    void validateDefault();
    void migrateToWallet();

  private slots:
    void slotTransportsChanged();
    void slotWalletOpened( bool success );
    void dbusServiceOwnerChanged( const QString &service, const QString &oldOwner, const QString &newOwner );
    void jobResult( KJob* job );

  private:
    static TransportManager* mInstance;
    KConfig *mConfig;
    QList<Transport*> mTransports;
    bool mMyOwnChange;
    KWallet::Wallet *mWallet;
    bool mWalletOpenFailed;
    bool mWalletAsyncOpen;
    int mDefaultTransportId;
    bool mIsMainInstance;
    QList<TransportJob*> mWalletQueue;
};

}

#endif
