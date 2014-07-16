/*
  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef CALENDARSUPPORT_PLUGIN_H
#define CALENDARSUPPORT_PLUGIN_H

#include <KLocalizedString>
#include <KPluginFactory>

namespace CalendarSupport {

/**
   @class Plugin

   @brief Specifies the plugin interface.

   This class is shared between korganizer's print plugins and
   calendarview's decoration plugins.
*/
class Plugin
{

  enum {
    INTERFACE_VERSION = 2
  };

  public:
    static int interfaceVersion() { return INTERFACE_VERSION; }

    static QString serviceType() { return QLatin1String( "Calendar/Plugin" ); }

    Plugin() {}
    virtual ~Plugin() {}

    virtual QString info() const = 0;

    virtual void configure( QWidget * ) {}
};

class PluginFactory : public KPluginFactory
{
  public:
    virtual Plugin *createPluginFactory() = 0;

  protected:
    virtual QObject *createObject( QObject *, const char *, const QStringList & )
    { return 0; }
};

}

#endif
