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

class KDE_EXPORT KConnectionManager : public QObject
{
Q_OBJECT
public:
    /**
     * This defines application policy in response to networking connect/disconnect events
     * Manual - the app only disconnects when the user does so
     * OnNextChange - the app should connect or disconnect the next time the network changes state, thereafter
     * Manual
     * Managed - the app should disconnect when the KConnectionManager thinks the system is
     * offline
     */
    enum ConnectionPolicy { Manual, OnNextChange, Managed };
    /**
     * Set a policy to manage the application's connect behaviour
     */
    void setConnectPolicy( ConnectionPolicy );
    /**
     * Retrieve a policy managing the application's connect behaviour
     */
    ConnectionPolicy connectPolicy() const;

    /**
     * Set a policy to manage the application's disconnect behaviour
     */
    void setDisconnectPolicy( ConnectionPolicy );

    /**
     * Retrieve a policy managing the application's disconnect behaviour
     */
    ConnectionPolicy disconnectPolicy() const;

    /*
     * We'll get logic of the form
     * onStatusChange() {
     *   if ( KConnectionManager::self()->policy( KConnectionManager::ConnectBehaviour ) == KConnectionManager::OnNextChange || 
     *        KConnectionManager::self()->policy( KConnectionManager::ConnectBehaviour ) == KConnectionManager::Managed ) 
     *   {
     *        // do connect
     *
     *        // reset the policy
     *        if ( KConnectionManager::self()->policy( KConnectionManager::ConnectBehaviour ) == KConnectionManager::OnNextChange )
     *          KConnectionManager::self()->setPolicy( KConnectionManager::KConnectionManager,
     *          KConnectionManager::Manual );
     *   }
     * 
     * Do we just use the CM for policy storage, or do we try to factor the logic to implement the
     * policy into the CM too?
     *
     * could signal doConnect(), then reset the policy
     * or could register a connect slot
     *  registerConnectMethod( QObject * receiver, const char * member );
     *  unregisterConnectMethod();
     * etc.
     * 
     * The problem with automatically controlled behaviour, where policy may change as a result of a
     * connect, is that if it is also manually altered, the CM needs to be updated.  But the CM needs to
     * be updated in any case.
     * CM need
     */
    /**
     * Lazy-method to set Manual on both policies
     */
    void setManualConnectionPolicies();
    /**
     * Lazy-method to set Managed on both policies
     */
    void setManagedConnectionPolicies();

    /**
     * Record a slot to call on a given receiving QObject when 
     * 1) the network connection is online,
     * 2) the policy mandates that the app connect
     *
     * Only one slot may be registered at any one time. If a second slot is 
     * registered, the first slot is forgotten
     * @param receiver the QObject where the slot is located
     * @param member the slot to call. Set up member using the SLOT() macro.
     */
    void registerConnectSlot( QObject * receiver, const char * member );

    /**
     * Forget any connect slot previously registered
     */
    void forgetConnectSlot();

    /**
     * Has any slot been registered to be called on connect?
     */
    bool isConnectSlotRegistered() const;

    /**
     * Record a slot to call on a given receiving QObject when 
     * 1) the network connection goes offline (in any way ),
     * 2) the policy mandates that the app disconnect
     *
     * Only one slot may be registered at any one time. If a second slot is 
     * registered, the first slot is forgotten
     * @param receiver the QObject where the slot is located
     * @param member the slot to call. Set up member using the SLOT() macro.
     */
    void registerDisconnectSlot( QObject * receiver, const char * member );

    /**
     * Forget any disconnect slot previously registered
     */
    void forgetDisconnectSlot();

    /**
     * Has any slot been registered to be called on disconnect?
     */
    bool isDisconnectSlotRegistered() const;

    static KConnectionManager* self();
    virtual ~KConnectionManager();
    NetworkStatus::Status status();
public Q_SLOTS:
    void serviceStatusChanged( uint status );
Q_SIGNALS:
    // signal that the network for a hostname is up/down
    void statusChanged( NetworkStatus::Status status );
private:
    // sets up internal state
    void initialise();
    // reread the desktop status from the daemon and update internal state
    KConnectionManager( QObject *parent );
    KConnectionManagerPrivate * d;
    static KConnectionManager * s_self;
};

#endif

