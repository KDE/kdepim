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

#ifndef konnector_h
#define konnector_h

#include <qobject.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcstring.h>
#include <ksyncentry.h>
#include "koperations.h"


class Kapabilities;
class KDevice;
class KonnectorPlugin;

/**
 *  Konnector is a convience to class to load/ configure
 *  and communicate with a Konnector plugin. This way all
 *  communication is done at one place. You query for konnectors
 *  and then set say this class to load one. You can connect
 *  to the signals and wait for magic happen
 */

class Konnector : public QObject{
Q_OBJECT
public:
   /** c'tor
     *
     * @param obj the Parent QObject
     * @param name the name of this instance
     */
     Konnector(QObject * obj= 0, const char* name= 0);

    /**
     * d'tor will unload every plugins
     */
    ~Konnector();

    /**
     *  @param  category the Category where Konnector should look \
     *          for konnector descriptions
     *  @return QValueList<KDevice> this will return a list of \
     *          konnector descriptions
     */
    QValueList<KDevice> query( const QString &category= QString::null );

    /** registers a konnector. register means it will load
     *  the plugin with the DeviceIdentification
     *  @param DeviceIndentification The Identification of the KDevice
     *  @return a runtime unique device id or QString::null if could not be loaded
     */
    QString registerKonnector(const QString &DeviceIdentification );
    /** registers a konnector to with a KDevice
     *  @param Device will load the KDevice
     *  @return returns a unique id or QString::null if a failure occured
     */
    QString registerKonnector(const KDevice &Device );

    /**
     * @param udi the unique device id got when registering a device
     * @return Kapabilities of the konnector identified \
     *         by the udi
     */
    Kapabilities capabilities( const QString &udi ) ;
    /**
     *  sets the Kabilities of a konnector
     *  @param udi The unique device id
     *  @param cap the Kapabilities
     */
    void setCapabilities( const QString &udi, const Kapabilities& cap );

    /**
     * This will synchronos fetch the data from path and
     * returned as QByteArray
     * @param udi The id of the konnector
     * @param path The path from where to fetch the data
     * @returns a QByteArray with the file inside
     */
    QByteArray file( const QString &udi, const QString &path );

    /**
     * This will synchronus fetch data and tries to convert
     * it to known KSyncEntry derived class. If not able to convert
     *
     * @param udi the ID of the Konnector plugin
     * @param path The path where to fetch data from
     * @return returns a KSyncEntry
     */
    KSyncEntry* fileAsEntry( const QString &udi,  const QString &path );
    /**
     * This is a asynchronus mode to fetch data. On sync
     * the file will be fetched
     * @param udi DeviceId
     * @param path The path of the file
     */
    void retrieveFile(const QString &udi, const QString &path);
    /**
     * Is the konnector connected to the device
     * @param udi Device Id of the Konnector
     */
    bool isConnected(const QString &udi );
    /**
     *  Pushes a sync to udi
     * @param udi Device Id
     */
    bool startSync(const QString &udi);

public slots:
    /**
     * this will write a List of KSyncEntry
     * to udi
     * @param udi Device Id
     * @param param The list of KSyncEntries
     */
    void write(const QString &udi, QPtrList<KSyncEntry> param );
    /**
     * This will do the KOperations
     * @param udi Device Id
     * @param ops Operations like delete
     */
    void write(const QString &udi, QValueList<KOperations> );
    /**
     * Konnector will write to dest the array
     * @param udi Device Id
     * @param dest destination of the array
     * @param array The array
     */
    void write(const QString &udi, const QString &dest, const QByteArray& array );
signals:
    /**
     * When the Konnector fetched all data wantsToSync is emitted
     * @param udi Device Id
     * @param param the List of KSyncEntry
     */
    void wantsToSync(const QString &udi, QPtrList<KSyncEntry> param);
    /**
     * The connection state of udi changes
     * @param udi Device Id
     * @param connected is the konnector still connected
     */
    void stateChanged(const QString &udi, bool connected ); // udi + state
    /**
     * The konnector Error
     * @param udi Device id
     * @param error the error id
     * @param id the error string
     */
    void konnectorError(const QString &udi, int error , const QString& id );
private:
    class KonnectorPrivate;
    KonnectorPrivate *d;
    void allDevices();
    KonnectorPlugin* pluginByUDI(const QString &udi );

private slots:
    void slotSync(QString, QPtrList<KSyncEntry> entry );
    void slotError(QString, int, QString);
};

#endif

