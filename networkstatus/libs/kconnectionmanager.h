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
Q_PROPERTY( ConnectionPolicy connectPolicy READ connectPolicy WRITE setConnectPolicy SCRIPTABLE true )
Q_PROPERTY( ConnectionPolicy disconnectPolicy READ disconnectPolicy WRITE setDisconnectPolicy SCRIPTABLE true )

public:
    /**
     * This defines application policy in response to networking connect/disconnect events
     */
    enum ConnectionPolicy {
        Manual, /**< Manual - the app should only disconnect when the user does so manually */
        OnNextChange, /**< the app should connect or disconnect the next time the network changes state, thereafter Manual */
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
    NetworkStatus::Status status();

    /**
     * Set a policy to manage the application's connect behaviour
     * @param the new connection policy
     */
    void setConnectPolicy( ConnectionPolicy );

    /**
     * Retrieve a policy managing the application's connect behaviour
     * @return the connection policy in use
     */
    ConnectionPolicy connectPolicy() const;

    /**
     * Set a policy to manage the application's disconnect behaviour
     * @param the new disconnection policy
     */
    void setDisconnectPolicy( ConnectionPolicy );

    /**
     * Retrieve a policy managing the application's disconnect behaviour
     * @return the disconnection policy in use
     */
    ConnectionPolicy disconnectPolicy() const;

    /**
     * Convenience method to set @ref NetworkStatus::Manual on both policies
     */
    void setManualConnectionPolicies();

    /**
     * Convenience method to set @ref NetworkStatus::Managed on both policies
     */
    void setManagedConnectionPolicies();

    /**
     * Record a slot to call on a given receiving QObject when
     * 1) the network connection is online,
     * 2) the policy mandates that the app connect
     *
     * Only one slot may be registered at any one time. If a second slot is
     * registered, the first slot is forgotten.
     * @param receiver the QObject where the slot is located
     * @param member the slot to call, format using the SLOT() macro.
     */
    void registerConnectSlot( QObject * receiver, const char * member );

    /**
     * Forget any connect slot previously registered
     */
    void forgetConnectSlot();

    /**
     * Has any slot been registered to be called on connect?
     * @return whether a slot is registered to be called on connect
     */
    bool isConnectSlotRegistered() const;

    /**
     * Record a slot to call on a given receiving QObject when
     * 1) the network connection goes offline (in any way ),
     * 2) the policy mandates that the app disconnect
     *
     * Only one slot may be registered at any one time. If a econd slot is
     * registered, the first slot is forgotten
     * @param receiver the QObject where the slot is located
     * @param member the slot to call, format using the SLOT() macro.
     */
    void registerDisconnectSlot( QObject * receiver, const char * member );

    /**
     * Forget any disconnect slot previously registered
     */
    void forgetDisconnectSlot();

    /**
     * Has any slot been registered to be called on disconnect?
     * @return whether a slot is registered to be called on disconnect
     */
    bool isDisconnectSlotRegistered() const;

    ~KConnectionManager();

Q_SIGNALS:
    /**
     * Signals that the network status has changed
     * @param status the new status of the network status service
     */
    void statusChanged( NetworkStatus::Status status );
protected Q_SLOTS:
    /**
     * Called on DBus signal from the network status service
     */
    void serviceStatusChanged( uint status );
private:
    // sets up internal state
    void initialize();
    // reread the desktop status from the daemon and update internal state
    KConnectionManager( QObject *parent );
    KConnectionManagerPrivate * d;
    static KConnectionManager * s_self;
};

#endif

