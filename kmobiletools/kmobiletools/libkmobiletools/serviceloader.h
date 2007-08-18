/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include <QtCore/QObject>
#include <KPluginInfo>

#include "kmobiletools_export.h"

namespace KMobileTools {

class ServiceLoaderPrivate;
class CoreService;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT ServiceLoader : public QObject {
    Q_OBJECT
friend class ServiceLoaderInstance;
public:
    /**
     * Returns a ServiceLoader instance
     *
     * @return a service loader instance
     */
    static ServiceLoader* instance();

    /**
     * Returns a list of services associated with the device
     * which implement a given list of @p interfaces
     *
     * Please note that the services must have been loaded in advance.
     *
     * @param deviceName the device name
     * @param interfaces a list of service interfaces that the service should implement
     *
     * @return a list of pointers to the services if available, else an empty QList
     */
    QList<KMobileTools::CoreService*> service( const QString& deviceName, const QStringList& interfaces = QStringList() ) const;

    ~ServiceLoader();

public Q_SLOTS:
    /**
     * Tries to load services for the device with given @p deviceName
     * Please note that loading services requires an already loaded device.
     *
     * @param deviceName a unique device name
     */
    void loadServices( const QString& deviceName );

    /**
     * Tries to unload the services for the device with given @p deviceName
     *
     * @param deviceName the device name
     */
    void unloadServices( const QString& deviceName );

Q_SIGNALS:
    /**
     * This signal is emitted whenever a service for the device was loaded
     *
     * @param deviceName the device for which the service was loaded
     * @param service the service that was loaded
     */
    void serviceLoaded( const QString& deviceName, KMobileTools::CoreService* service );

    /**
     * This signal is emitted when a service was unloaded
     * @warning The returned service pointer is invalid! Use it to identify any reference
     * to a service by address
     *
     * @param deviceName the device for which the service was unloaded
     * @param service the service that was unloaded
     */
    void serviceUnloaded( const QString& deviceName, KMobileTools::CoreService* service );

    void aboutToUnloadService( const QString& deviceName, KMobileTools::CoreService* service );

private:
    ServiceLoader();
    ServiceLoaderPrivate* d;

};

}

#endif
