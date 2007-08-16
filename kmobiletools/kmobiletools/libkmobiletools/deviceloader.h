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

#ifndef DEVICELOADER_H
#define DEVICELOADER_H

#include <QtCore/QObject>
#include <KDE/KPluginInfo>

#include "kmobiletools_export.h"

namespace KMobileTools {

class EngineXP;
class DeviceLoaderPrivate;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT DeviceLoader : public QObject {
    Q_OBJECT
friend class DeviceLoaderInstance;
public:
    /**
     * Returns a DeviceLoader instance
     *
     * @return a device loader instance
     */
    static DeviceLoader* instance();

    /**
     * Returns the engine object associated with the given @p deviceName
     *
     * @param deviceName the device name
     *
     * @return a pointer to the engine if successful, else null
     */
    EngineXP* engine( const QString& deviceName ) const;

    /**
     * Returns information about the engine object associated with the
     * device with given @p deviceName
     *
     * @param deviceName the device name
     *
     * @return information about the engine
     */
    KPluginInfo engineInformation( const QString& deviceName ) const;

    ~DeviceLoader();

public Q_SLOTS:
    /**
     * Tries to load an engine that gets associated with a given device name
     *
     * @param deviceName a unique device name
     * @param engineName the engine to load
     *
     * @returns true if the device was successfully loaded
     */
    bool loadDevice( const QString& deviceName, const QString& engineName );

    /**
     * Tries to unload the device with the given @p deviceName
     *
     * @param deviceName the device name
     *
     * @returns true if the device was successfully unloaded
     */
    bool unloadDevice( const QString& deviceName );

Q_SIGNALS:
    /**
     * This signal is emitted when the device was successfully loaded.
     */
    void deviceLoaded( const QString& );

    /**
     * This signal is emitted when the device was successfully unloaded.
     */
    void deviceUnloaded( const QString& );

private:
    DeviceLoader();
    DeviceLoaderPrivate* d;

};

}

#endif
