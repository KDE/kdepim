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

#ifndef KMOBILETOOLSCONFIG_H
#define KMOBILETOOLSCONFIG_H

#include "kmobiletools_export.h"

#include <QtCore/QStringList>

namespace KMobileTools {

class ConfigPrivate;
/**
 * Use this class to access any configuration option
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class KMOBILETOOLS_EXPORT Config
{
friend class ConfigInstance;
public:
    /**
     * Returns a configuration instance
     *
     * @return a config instance
     */
    static Config* instance();

    /**
     * Adds a new device with given @p name and associated @p engine to the
     * configuration
     *
     * @param name the device name
     * @param engine the engine name
     */
    void addDevice( const QString& name, const QString& engine );

    /**
     * Removes the device with given @p name from the configuration
     *
     * @param name the device name
     */
    void removeDevice( const QString& name );

    /**
     * Returns the list of configured devices
     *
     * @return the device list
     */
    QStringList deviceList() const;

    /**
     * Returns the engine associated with the given @p deviceName
     *
     * @param deviceName the device name
     * @return the engine name or an empty string if not available
     */
    QString engine( const QString& deviceName );

    ~Config();

private:
    Config();
    ConfigPrivate* d;
};

}

#endif
