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

#ifndef MAILTRANSPORT_TRANSPORTMANAGER_H
#define MAILTRANSPORT_TRANSPORTMANAGER_H

#include <mailtransport/mailtransport_export.h>
#include <mailtransport/transporttype.h>

#include <QtCore/QList>
#include <QtCore/QObject>

#include <akonadi/agenttype.h>

class KJob;

namespace KWallet {
  class Wallet;
}

namespace MailTransport {

class Transport;
class TransportConfigWidget;
class TransportJob;
class TransportManagerPrivate;

/**
  @short Central transport management interface.

  This class manages the creation, configuration, and removal of mail
  transports, as well as the loading and storing of mail transport settings.

  It also handles the creation of transport jobs, although that behaviour is
  deprecated and you are encouraged to use MessageQueueJob.

  @see MessageQueueJob.
*/
class MAILTRANSPORT_EXPORT TransportManager : public QObject
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kde.pim.TransportManager" )

  friend class Transport;
  friend class TransportManagerPrivate;

  public:

    /**
      Destructor.
    */
    virtual ~TransportManager();

    /**
      Returns the TransportManager instance.
    */
    static TransportManager *self();

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
    Transport *transportById( int id, bool def = true ) const;

    /**
      Returns the transport object with the given name.
      @param name The transport name.
      @param def if set to true, the default transport will be returned if the
      specified Transport object could not be found, 0 otherwise.
      @returns A Transport object for immediate use, see transportById() for
      limitations.
    */
    Transport *transportByName( const QString &name, bool def = true ) const;

    /**
      Returns a list of all available transports.
      Note: The Transport objects become invalid as soon as a change occur, so
      they are only suitable for immediate use.
    */
    QList<Transport *>transports() const;

    /**
      Returns a list of all available transport types.
    */
    TransportType::List types() const;

    /**
      Creates a new, empty Transport object. The object is owned by the caller.
      If you want to add the Transport permanently (eg. after configuring it)
      call addTransport().
    */
    Transport *createTransport() const;

    /**
      Adds the given transport. The object ownership is transferred to
      TransportMananger, ie. you must not delete @p transport.
      @param transport The Transport object to add.
    */
    void addTransport( Transport *transport );

    /**
      Creates a mail transport job for the given transport identifier.
      Returns 0 if the specified transport is invalid.
      @param transportId The transport identifier.

      @deprecated use MessageQueueJob to queue messages
                  and rely on the Dispatcher Agent to send them.
    */
    MAILTRANSPORT_DEPRECATED TransportJob *createTransportJob( int transportId );

    /**
      Creates a mail transport job for the given transport identifer,
      or transport name.
      Returns 0 if the specified transport is invalid.
      @param transport A string defining a mail transport.

      @deprecated use MessageQueueJob to queue messages
                  and rely on the Dispatcher Agent to send them.
    */
    MAILTRANSPORT_DEPRECATED TransportJob *createTransportJob( const QString &transport );

    /**
      Executes the given transport job. This is the preferred way to start
      transport jobs. It takes care of asynchronously loading passwords from
      KWallet if necessary.
      @param job The completely configured transport job to execute.

      @deprecated use MessageQueueJob to queue messages
                  and rely on the Dispatcher Agent to send them.
    */
    MAILTRANSPORT_DEPRECATED void schedule( TransportJob *job );

    /**
      Tries to create a transport based on KEMailSettings.
      If the data in KEMailSettings is incomplete, no transport is created.
    */
    void createDefaultTransport();

    /// Describes when to show the transport creation dialog
    enum ShowCondition {
      Always,              ///< Show the transport creation dialog unconditionally
      IfNoTransportExists  ///< Only show the transport creation dialog if no transport currently
                           ///  exists. Ask the user if he wants to add a transport in
                           ///  the other case.
    };

    /**
      Shows a dialog for creating and configuring a new transport.
      @param parent Parent widget of the dialog.
      @param showCondition the condition under which the dialog is shown at all
      @return True if a new transport has been created and configured.
      @since 4.4
    */
    bool showTransportCreationDialog( QWidget *parent, ShowCondition showCondition = Always );

    /**
      Open a configuration dialog for an existing transport.
      @param transport The transport to configure.  It can be a new transport,
                       or one already managed by TransportManager.
      @param parent The parent widget for the dialog.
      @return True if the user clicked Ok, false if the user cancelled.
      @since 4.4
    */
    bool configureTransport( Transport *transport, QWidget *parent );

  public Q_SLOTS:
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

  Q_SIGNALS:
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
      If you made a deep copy of a transport, you should call updatePasswordState()
      for the cloned transport to ensure its password is updated as well.
    */
    void passwordsChanged();

    /**
      Emitted when a transport is deleted.
      @param id The identifier of the deleted transport.
      @param name The name of the deleted transport.
    */
    void transportRemoved( int id, const QString &name );

    /**
      Emitted when a transport has been renamed.
      @param id The identifier of the renamed transport.
      @param oldName The old name.
      @param newName The new name.
    */
    void transportRenamed( int id, const QString &oldName, const QString &newName );

  protected:
    /**
      Returns a pointer to an open wallet if available, 0 otherwise.
      The wallet is opened synchronously if necessary.
    */
    KWallet::Wallet *wallet();

    /**
      Loads all passwords synchronously.
    */
    void loadPasswords();

    /**
      Singleton class, the only instance resides in the static object sSelf.
    */
    TransportManager();

  private:

    // These are used by our friend, Transport
    void emitChangesCommitted();

  private:
    TransportManagerPrivate *const d;

    Q_PRIVATE_SLOT( d, void slotTransportsChanged() )
    Q_PRIVATE_SLOT( d, void slotWalletOpened( bool success ) )
    Q_PRIVATE_SLOT( d, void dbusServiceUnregistered() )
    Q_PRIVATE_SLOT( d, void agentTypeAdded( const Akonadi::AgentType &atype ) )
    Q_PRIVATE_SLOT( d, void agentTypeRemoved( const Akonadi::AgentType &atype ) )
    Q_PRIVATE_SLOT( d, void jobResult( KJob *job ) )
};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTMANAGER_H
