/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef kapabilities_h
#define kapabilities_h

#include <qmemarray.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qhostaddress.h>
#include <qpair.h>
#include <qmap.h>
#include <qvariant.h>
#include <qglobal.h>

/** Kapabilities stores all capabilities a konnector is capable of. This is
 *  a list that I could imagine a konnector could do for different
 *  devices. If a Konnector is capable of more he can supply extraOptions
 *  The Kapabilities are used to configure the konnector
 *
 */
namespace KSync {
class Kapabilities {
public:
    /**
       empty c'tor
     */
    Kapabilities();
    /**
     * copy c'tor
     */
    Kapabilities(const Kapabilities & );
    /**
     * d'tor
     */
    ~Kapabilities();

    /**
     *  If the Konnector supports metasyncing
     */
    bool supportsMetaSyncing() const;
    void setSupportMetaSyncing( bool meta );
    /**
     *
     */
    void setMetaSyncingEnabled( bool  enable);
    bool isMetaSyncingEnabled() const;

    /**
     * If the device is capable of initializing the sync itself
     * @return bool
     */
    bool supportsPushSync() const;

    /**
     * Set if the device is capable to initialize the sync itself
     * @param bool
     */
    void setSupportsPushSync(bool push);

    /**
     * If the device is _not_ capable to establish a connection itself.
     * The user needs to take care of making an connection.
     * @return bool
     */
    bool needsConnection() const;

    /**
     * Set if the device is _not_ capable to establish a connection itself
     * @param is or is not capable
     */
    void setNeedsConnection(bool connection );

    bool supportsListDir() const;
    void setSupportsListDir(bool );

    /**
     * Which ports are possible.
     * @return Array with possible ports
     */
    QMemArray<int> ports()const;

    /**
     * Set the possible ports
     * @param the possible ports
     */
    void setPorts(const  QMemArray<int>& );

    /**
     * Which port is actually used.
     * @return the port in use
     */
    int currentPort() const;

    /**
     * Set the port which will be used.
     * @param the port to be used
     */
    void setCurrentPort( int );

    /**
     * If this konnector needs a network konnection at all
     */
    bool needsNetworkConnection()const;
    void setNeedsNetworkConnection( bool );
    /**
     *  Does the Konnector need an IP Address
     *  to work?
     *  @return if we need to give an IP Address
     */
    bool needsIPs()const;
    /**
     *  Do we need to supply a source address?
     */
    bool needsSrcIP()const;
    /**
     * Do we need to supply a destination or can the
     * konnector handle that
     */
    bool needsDestIP()const;
    /**
     *  Set if we need IP Addresses
     */
    void setNeedsIPs(bool ip);
    /**
     * Set if we need source IPs
     */
    void setNeedsSrcIP( bool srcIp );
    /**
     * Set if we need a dest ip
     */
    void setNeedsDestIP(bool srcIp );

    /**
     * If we need a source IP we can set it
     * with that method
     * @param source ip
     */
    void setSrcIP( const QString & ); // FIXME use QString FIXED

    /**
     * Returns the src IP
     * @return source ip
     */
    QString srcIP()const;

    /**
     * Set the destination IP
     * @param set destination ip
     */
    void setDestIP(const QString &);

    /**
     * @return destination ip
     */
    QString destIP()const;

    /**
     * The device can act on its own to establish an connection
     * @return bool
     */
    bool canAutoHandle() const;

    /**
     * Set if the device can take care of establishing the connection on its own.
     * For example if the device is plugged in, it brings up the connection on its own.
     * The device can also start syncing.
     * @param bool.
     */
    void setAutoHandle(bool);

    // wrong....
    /**
     * ipPproposals gives you a pair of ip Addresses which the Konnector
     * think could work
     */
    QStringList ipProposals() const;
    void setIpProposals( const QStringList& );


    /**
     * If the device need some kind of authentification first.
     * @return bool
     */
    bool needAuthentication()const;

    /**
     * Set if the device needs authentification or not.
     * @param if it need auth or not.
     */
    void setNeedAuthentication(bool need);

    /**
     * Sets the username for authentification
     * @param username for auth
     */
    void setUser(const QString &);

    /**
     * The username used for authentification.
     * @return the username.
     */
    QString user() const;

    /**
     * Sets the password for auth.
     * @param the password for auth.
     */
    void setPassword(const QString & );

    /**
     * The password used in auth.
     * @return the password used in auth.
     */
    QString password() const;

    /**
     * this is a proposal of possible user and passwords
     * @return the pair username , password
     */
    QValueList< QPair<QString, QString> > userProposals() const;

    /**
     * If the device has a "shipping" username and password, the connector can
     * propose it.
     * @param the proposed username and password.
     */
    void setUserProposals( QValueList< QPair<QString, QString> > );

    /**
     * Copy operator
     */
    Kapabilities &operator=(const Kapabilities & );
    /**
     * Some Options for palm and others using a  lib as backend
     * supporting more devices
     * and does not need network
     */
    /**
     * A list of models isEmpty() if no models
     */
    QStringList models()const;
    /**
     * set the models
     */
    void setModels( const QStringList & );
    /**
     *
     */
    QString currentModel()const;
    void setCurrentModel( const QString & );
    void setConnectionMode( const QStringList & );
    bool needsModelName()const;
    void setNeedsModelName(bool );
    QString modelName()const;
    void setModelName( const QString& );

    QStringList connectionModes()const;
    QString currentConnectionMode()const;
    void setCurrentConnectionMode(const QString &);
    /**
     *
     */
    void setExtraOption( const QString &, const QString & );
    QMap<QString, QString> extras()const { return m_extras; };
//    void dump()const;
private:
    class KapabilitiesPrivate;
    KapabilitiesPrivate *d;
    bool m_needsNet:1;
    bool m_push:1;
    bool m_needConnection:1;
    bool m_listdir:1;
    bool m_needsIp:1;
    bool m_needsSrcIp:1;
    bool m_needsDestIp:1;
    bool m_needsAuthent:1;
    bool m_supMeta:1; // supports Meta
    bool m_meta:1;
    bool m_needsName;
    QString m_src;
    QString m_dest;
    QStringList m_propsIPs;
    QValueList< QPair<QString, QString> > m_propAuth;
    bool m_canHandle;
    QMemArray<int> m_ports;
    int m_current;
    QString m_user;
    QString m_pass;
    QMap<QString,QString> m_extras;
    QString m_currModell;
    QStringList m_models;
    QString m_currMode;
    QStringList m_modes;
    QString m_name;
};
};
#endif


