/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KDE_CONNECTION_MANAGER_H
#define KDE_CONNECTION_MANAGER_H

//TODO own macro file?
#include <kdemacros.h>

#include <networkstatuscommon.h>

class KConnectionManagerPrivate;
/**
 * KConnectionManager is a high level class for monitoring and managing the system's network
 * connection state.  It is useful for applications which need to react to changes in network
 * connection state, for example, to enter an offline reading mode during periods of no
 * connectivity.
 *
 * As well as state accessors and signals, KConnectionManager provides an easy way for an
 * application to be notified of connect and disconnect events.  By registering slots to be called
 * when this happens with KConnectionManager, you provide handlers for these events so that your
 * application can react in the appropriate way:
 *
 * @code
 * KConnectionManager::self()->registerConnectSlot( this, SLOT( doConnect() ) );
 * @endcode
 *
 * [Dis]connection policies allow the application to specify the circumstances when these registered
 * slots are called.  The default policy is Managed, meaning the connect and disconnect slots, if
 * registered, will always be called on status change, but it is also possible to set them to @ref
 * OnNextChange, which provides for single shot connection event handling, or to disable connection
 * management with @ref Manual.
 *
 * @author Will Stephenson <wstephenson@kde.org>
 * @since 4.0
 */
class KDE_EXPORT KConnectionManager : public QObject
{
Q_OBJECT
// TODO Q_PROPERTY doesn't like namespaced enums, as far as I can see. Should go away when this is
// all together in Solid::Networking
//Q_ENUMS( Status )
//Q_PROPERTY( Status status READ status SCRIPTABLE true )
Q_PROPERTY( ManagementPolicy connectPolicy READ connectPolicy WRITE setConnectPolicy SCRIPTABLE true )
Q_PROPERTY( ManagementPolicy disconnectPolicy READ disconnectPolicy WRITE setDisconnectPolicy SCRIPTABLE true )

public:
    /**
     * This defines application policy in response to networking connect/disconnect events
     */
    enum ManagementPolicy {
        Manual, /**< Manual - the app should only disconnect when the user does so manually */
        OnNextStatusChange, /**< the app should connect or disconnect the next time the network changes status, thereafter Manual */
        Managed /**< the app should connect or disconnect whenever the KConnectionManager reports a state change */
    };
    /**
     * The KConnectionManager instance used globally in an application.
     * It is deleted automatically when the application exits.
     * This function returns an instance of KConnectionManager.  One will
     * be created if none exists already.
     * @return a KConnectionManager instance
     */
    static KConnectionManager* self();

    /**
     * Access the current status of the network status service.
     * @return the current network status according to the network status service
     */
    NetworkStatus::Status status() const;

    /**
     * Set a policy to manage the application's connect behaviour
     * @param the new connection policy
     */
    void setConnectPolicy( ManagementPolicy );

    /**
     * Retrieve a policy managing the application's connect behaviour
     * @return the connection policy in use
     */
    ManagementPolicy connectPolicy() const;

    /**
     * Set a policy to manage the application's disconnect behaviour
     * @param the new disconnection policy
     */
    void setDisconnectPolicy( ManagementPolicy );

    /**
     * Retrieve a policy managing the application's disconnect behaviour
     * @return the disconnection policy in use
     */
    ManagementPolicy disconnectPolicy() const;

    ~KConnectionManager();

Q_SIGNALS:
    /**
     * Signals that the network status has changed
     * @param status the new status of the network status service
     */
    void statusChanged( NetworkStatus::Status status );
    /**
     * Signals that the system's network has become connected, so receivers
     * should connect their sockets, ioslaves etc.
     *
     * This signal is emitted according to the active connectPolicy.
     */
    void shouldConnect();
    /**
     * Signals that the system's network has become disconnected,
     * so receivers should adjust application state appropriately.
     *
     * This signal is emitted according to the active disconnectPolicy.
     */
    void shouldDisconnect();

protected Q_SLOTS:
    /**
     * Called on DBus signal from the network status service
     */
    void serviceStatusChanged( uint status );
    /**
     * Detects when kded restarts, and sets status to NoNetworks so that apps
     * may proceed
     */
    void serviceOwnerChanged( const QString &, const QString &, const QString & );
private:
    // sets up internal state
    void initialize();
    // reread the desktop status from the daemon and update internal state
    KConnectionManager( QObject *parent );
    KConnectionManagerPrivate * d;
    static KConnectionManager * s_self;
};

#endif

