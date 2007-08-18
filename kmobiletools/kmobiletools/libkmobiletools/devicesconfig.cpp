/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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

#include "devicesconfig.h"

#include <qstring.h>
//Added by qt3to4:
#include <QPixmap>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kplugininfo.h>
#include <QtCore/QHash>
#include <ksharedconfig.h>

#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/kmobiletools_cfg.h>
#include <libkmobiletools/engine.h>


using namespace KMobileTools;

typedef QHash<QString,DevicesConfig*> DevsHash;


class DevicesConfigPrivate {
    public:
        DevicesConfigPrivate() {}
};

K_GLOBAL_STATIC( DevsHash, m_prefs)

DevicesConfig::DevicesConfig (const QString & DeviceName)
    : DevicesConfigBase(DeviceName), d(new DevicesConfigPrivate)
{
}

DevicesConfig::~DevicesConfig()
{
    delete d;
}


DevicesConfig *DevicesConfig::prefs(const QString &groupName)
{
//     if (!m_prefs)
//     {
//         m_prefs = new Q3Dict<DevicesConfig>();
//         m_prefs->setAutoDelete(true);
//     }
    DevicesConfig *devicesPrefs = m_prefs->value(groupName, NULL);
    if (devicesPrefs)
    {
/*        devicesPrefs->readConfig();*/
        return devicesPrefs;
    }

    devicesPrefs = new DevicesConfig(groupName);
    devicesPrefs->readConfig();

    /// @TODO port to EngineXP
    /*
    QString libname=devicesPrefs->engine();
    delete devicesPrefs;
    kDebug() <<"DevicesConfig::prefs(" << groupName <<"); loading engine" << libname;
    Engine *engine=Engine::load(libname);
    if(!engine) return NULL;
    devicesPrefs=engine->config(true, groupName);
    delete engine;
    m_prefs->insert(groupName, devicesPrefs);
    */

    return devicesPrefs;
}

bool DevicesConfig::hasPrefs(const QString &groupName)
{
    KConfig &config = *KGlobal::config();
    return config.hasGroup(groupName);
}

void DevicesConfig::deletePrefs(const QString &groupName)
{
    KConfig &config = *KGlobal::config();
    config.deleteGroup(groupName);
    if (!m_prefs)
        return;
    m_prefs->remove(groupName);
}

const QString DevicesConfig::deviceGroup( const QString &devicename)
{
    QStringList validDevices = MainConfig::devicelist();
    KConfig cfg/*=new KConfig*/("kmobiletoolsdevicesrc");
    QString cgroup;
    for(int i=0; i<cfg.groupList().size() ; i++)
    {
        cgroup=cfg.groupList().at(i);
        if( cfg.group(cgroup).readEntry("devicename") == devicename) {
      if ( ! validDevices.contains(cgroup) ) {
        DevicesConfig::deletePrefs(cgroup);
        kDebug() <<"Removed stale group" << cgroup;
      } else
        return cgroup;
    }
    }
    return NULL;
}

const QString DevicesConfig::firstFreeGroup()
{
    QString cgroup;
    for(int i=0; i<100; i++)
    {
        cgroup="device-%1";
        cgroup=cgroup.arg(i);
        if( ! DEVCFG(cgroup) || ! (DEVCFG( cgroup )->devicename().length() ) )
            return cgroup;
    }
    return NULL;
}

const QPixmap DevicesConfig::deviceTypeIcon(const QString &groupName, K3Icon::Group group, int size)
{
    kDebug() <<"deviceTypeIcon(); groupName=" << groupName <<", engine=" << DevicesConfig::prefs(groupName)->engine();

    KPluginInfo info = EnginesList::instance()->engineInfo(DevicesConfig::prefs(groupName)->engine() );
    if( !info.isValid() )
        return QPixmap();

    kDebug() <<"icon:" << info.icon();
    return KIconLoader::global()->loadIcon(info.icon(), group, size);
}

const QString DevicesConfig::engineTypeName(const QString &libName)
{
    KPluginInfo info=EnginesList::instance()->engineInfo(libName);
    if(!info.isValid()) return QString();
    return info.name();
}

const QString DevicesConfig::deviceTypeIconPath(const QString &groupName, int groupOrSize)
{
    KPluginInfo info=EnginesList::instance()->engineInfo(DevicesConfig::prefs(groupName)->engine() );
    if(!info.isValid()) return QString();
    return KIconLoader::global()->iconPath(info.icon(), groupOrSize);
}
