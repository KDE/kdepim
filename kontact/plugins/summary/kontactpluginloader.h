/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KONTACTPLUGINLOADER_H
#define KONTACTPLUGINLOADER_H

#include <plasma/pluginloader.h>

class KontactInterfacesService;

namespace Plasma {
class Applet;
class DataEngine;
class Service;
}

#include <KPluginInfo>

/**
 * This class provides applets running in Kontact access to the
 * KontactInterfaces class needed to control various bits of kontact
 * applications. This class will only work inside of Kontact though, so
 * applets placed on the desktop will lack functionalities of
 * KontactInterfaces.
 *
 * @author Ryan Rix <ry@n.rix.si>
 * @since 4.6
 **/
class KontactPluginLoader : public Plasma::PluginLoader
{
public:
    KontactPluginLoader();

protected:
    Plasma::Applet* internalLoadApplet(QString &name, uint appletId = 0, const QVariantList &args = QVariantList());

    Plasma::DataEngine* internalLoadEngine(const QString &name);

    Plasma::Service* internalLoadService(const QString &name, const QVariantList &args, QObject *parent);

private:
     KontactInterfacesService* m_service;
};

#endif
