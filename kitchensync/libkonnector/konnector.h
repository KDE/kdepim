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

#include <qiconset.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qcstring.h>



#include <syncer.h>

#include "kdevice.h"
#include "koperations.h"


namespace KSync {

class Kapabilities;
class Device;
class KonnectorPlugin;
class ConfigWidget;
/**
 *  KonnectorManager is a convience to class to load/ configure
 *  and communicate with a Konnector plugin. This way all
 *  communication is done at one place. You query for konnectors
 *  and then set say this class to load one. You can connect
 *  to the signals and wait for magic happen
 */

class KonnectorManager : public QObject{
Q_OBJECT
public:
   /** c'tor
     *
     * @param obj the Parent QObject
     * @param name the name of this instance
     */
     KonnectorManager(QObject * obj= 0, const char* name= 0);

    /**
     * d'tor will unload every plugins
     */
    ~KonnectorManager();

    /**
     *  @param  category the Category where Konnector should look \
     *          for konnector descriptions
     *  @return Device::ValueList this will return a list of \
     *          konnector descriptions
     */
    Device::ValueList query( const QString &category= QString::null );

    /** registers a konnector. register means it will load
     *  the plugin with the DeviceIdentification
     *  @param DeviceIndentification The Identification of the Device
     *  @return a runtime unique device id or QString::null if could not be loaded
     */
    QString registerKonnector(const QString &DeviceIdentification );

    /** registers a konnector to with a Device
     *  @param Device will load the Device
     *  @return returns a unique id or QString::null if a failure occurred
     */
    QString registerKonnector(const Device &device );

    /**
     * Tries to unregister the Konnector with the udi
     */
    bool unregisterKonnector( const QString& udi );

    /**
     * @param udi the unique device id got when registering a device
     * @return Kapabilities of the konnector identified \
     *         by the udi
     */
    Kapabilities capabilities( const QString &udi ) ;

    /**
     * Another way to configure a Konnector is to request a GUI
     * Note this only works if a QApplication with type Gui*
     * was created
     *
     * @param parent the QWidget parent
     * @return 0 if no QWidget could be created
     *
     */
    ConfigWidget* configWidget( const QString& udi,
                                QWidget* parent,
                                const char* name );

    /**
     *
     * same as above but uses a Kapability for the initial setup
     */
    ConfigWidget* configWidget( const QString& udi,
                                const Kapabilities&,
                                QWidget* parent,
                                const char* name );


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
     * This willfetch data and tries to convert
     * it to a known Syncee derived class. If not able to convert
     * it will return a UnknownSyncee
     *
     * @param udi the ID of the Konnector plugin
     * @param path The path where to fetch data from
     * @return returns a KSyncEntry
     */
    Syncee* fileAsEntry( const QString &udi,  const QString &path );
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
    bool connectDevice(const QString &udi );
    void disconnectDevice( const QString &udi );
    /**
     *  Pushes a sync to udi
     * @param udi Device Id
     */
    bool startSync(const QString &udi);
    /**
     *  Pushes backup to udi
     * @param udi Device Id
     * @param path Path to the backup file
     */
    bool startBackup(const QString &udi, const QString& path);
    /**
     *  Pushes a restore to udi
     * @param udi Device Id
     * @param path Path to the backup file
     */
    bool startRestore(const QString &udi, const QString& path);

    //bool canPush(const QString& udi)const;
    /**
     * Returns an IconSet for the given udi
     */
    QIconSet iconSet(const QString& udi )const;
    QString id(const QString& udi )const;
    QString metaId( const QString& udi )const;
    QString iconName(const QString& udi )const;
public slots:
    /**
     * this will write a List of Syncee
     * to udi
     * @param udi Device Id
     * @param param The list of Syncee
     */
    void write(const QString &udi, Syncee::PtrList param );
    /**
     * This will do the KOperations
     * @param udi Device Id
     * @param ops Operations like delete
     */
    void write(const QString &udi, KOperations::ValueList );
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
     * @param param the List of Syncee
     */
    void wantsToSync(const QString &, Syncee::PtrList);
    /**
     * The connection state of udi changes
     * @param udi Device Id
     * @param connected is the konnector still connected
     */
    void stateChanged(const QString &, bool); // udi + state
    /**
     * The konnector Error
     * @param udi Device id
     * @param error the error id
     * @param id the error string
     */
    void konnectorError(const QString &, int , const QString& );
private:
    class KonnectorPrivate;
    KonnectorPrivate *d;
    void allDevices();
    Device find( const QString& name );
    QString generateUID();
    void addDevice( const QString& path );
    KonnectorPlugin* pluginByUDI(const QString &udi );
    KonnectorPlugin* pluginByUDI(const QString &udi )const;

private slots:
    void slotSync(const QString&, Syncee::PtrList entry);
    void slotError(const QString&, int, const QString&);
    void slotChanged(const QString&,  bool );
};
};
#endif

