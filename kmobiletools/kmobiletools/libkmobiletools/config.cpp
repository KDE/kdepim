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

#include "config.h"

#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <KDE/KLocale>
#include <KDE/KTar>
#include <KDE/KMessageBox>
#include <KDE/KStandardDirs>

#include <libkmobiletools/kmobiletools_devices.h>
#include <libkmobiletools/kmobiletools_cfg.h>

namespace KMobileTools {

class ConfigInstance {
public:
    Config m_uniqueInstance;
};

class ConfigPrivate {
public:
    const static uint currentVersion = 20070908;

    QMutex mutex;
};

K_GLOBAL_STATIC(ConfigInstance, configInstance)

Config::Config()
    : d( new ConfigPrivate ) {
    // check if the user's configuration is outdated
    if( d->currentVersion != MainConfig::configversion() ) {
        QDir configDir( KGlobal::dirs()->saveLocation("config") );
        QStringList entries = configDir.entryList( QStringList() << "*kmobiletools*", QDir::Files );
        if( !entries.isEmpty() ) {
            QString archiveName = KGlobal::dirs()->saveLocation("tmp") + "kmobiletools-" +
                    QDate::currentDate().toString( Qt::ISODate ) + ".tar.gz";

            KMessageBox::information( 0, i18n("<qt><p>KMobileTools has found an old or invalid "
                                              "configuration file.</p><p>To work correctly, it needs "
                                              "to delete your configuration files. Your old files will "
                                              "be saved in <b>%1</b></p></qt>", archiveName ) );
            KTar arch( archiveName );
            if( !arch.open( QIODevice::WriteOnly) ) {
                KMessageBox::error( 0, i18n("<qt><p>KMobileTools could not archive your config files.</p><p>Please remove them manually.</p></qt>") );
            } else {
                for( QStringList::Iterator it=entries.begin(); it!=entries.end(); ++it ) {
                    arch.addLocalFile( configDir.path() + QDir::separator() + (*it), (*it));
                    QFile::remove( configDir.path() + QDir::separator() + (*it) );
                }
                arch.close();

                KMobileTools::MainConfig::self()->readConfig();
                KMobileTools::MainConfig::self()->setConfigversion( d->currentVersion );
                KMobileTools::MainConfig::self()->writeConfig();
            }
        }
    }
}

Config::~Config()
{
    delete d;
}

Config* Config::instance() {
    // instance is automatically created
    return &configInstance->m_uniqueInstance;
}

void Config::addDevice( const QString& name, const QString& engine ) {
    QMutexLocker( &d->mutex );

    QStringList deviceList = MainConfig::devicelist();
    if( !deviceList.contains( name ) ) {
        deviceList.append( name );

        // write configuration entries
        MainConfig::self()->readConfig();
        MainConfig::self()->setDevicelist( deviceList );
        MainConfig::self()->writeConfig();

        DevicesConfigBase devicesConfig( name );
        devicesConfig.readConfig();
        devicesConfig.setEngine( engine );
        devicesConfig.writeConfig();
    } else {
        /// @todo add call to error handler here
        ;
    }
}

void Config::removeDevice( const QString& name ) {
    QMutexLocker( &d->mutex );

    QStringList deviceList = MainConfig::devicelist();
    if( deviceList.contains( name ) ) {
        // retrieve KConfig object from devices config
        DevicesConfigBase devicesConfig( name );
        KConfig* config = devicesConfig.config();

        // take the device from the device list
        deviceList.removeAll( name );

        // write config entries
        config->deleteGroup( name );
        MainConfig::self()->readConfig();
        MainConfig::self()->setDevicelist( deviceList );
        MainConfig::self()->writeConfig();
    } else {
        /// @todo add call to error handler here
        ;
    }
}

QStringList Config::deviceList() const {
    return MainConfig::devicelist();
}

QString Config::engine( const QString& deviceName ) {
    DevicesConfigBase deviceConfig( deviceName );
    return deviceConfig.engine();
}

}
