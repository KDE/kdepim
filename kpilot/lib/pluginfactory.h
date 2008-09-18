#ifndef KPILOT_PLUGINFACTORY_H
#define KPILOT_PLUGINFACTORY_H
/* KPilot
**
** Copyright (C) 2005-2007 by Adriaan de Groot <groot@kde.org>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// KPilot headers
#include "kpilotlink.h"
#include "options.h"

// KDE headers
#include <kpluginfactory.h>
#include <kpluginloader.h>

/**
 * A conduit has a name -- which must match the name of the library
 * that it lives in -- and two classes: a configure widget which derives
 * from ConduitConfigBase and a conduit action that derives from
 * ConduitAction. The boilerplate needed to handle the plugin
 * factory name and special symbols as well as the factory
 * is hidden in this macro.
 *
 * @param a The name of the conduit.
 * @param b The class name for the config widget.
 * @param c The class name for the conduit action.
 *
 * @note No quotes around the name.
 * @example DECLARE_KPILOT_PLUGIN(null, NullConfigWidget, ConduitNull)
 */
#define DECLARE_KPILOT_PLUGIN(a,b,c) \
K_PLUGIN_FACTORY(a##factory, registerPlugin<b>(QString(), &createConduitConfigInstance<b>); registerPlugin<c>(QString(), &createConduitActionInstance<c>);) \
K_EXPORT_PLUGIN(a##factory(#a)) \
K_EXPORT_PLUGIN_VERSION(Pilot::PLUGIN_API)

template<class impl>
QObject *createConduitActionInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
{
    Q_UNUSED(parentWidget);
    KPilotLink *link = qobject_cast<KPilotLink *>(parent);
    Q_ASSERT(link || !parent);

    return new impl(link, args);
}

template<class impl>
QObject *createConduitConfigInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
{
    Q_UNUSED(parent);
    return new impl(parentWidget, args);
}

#endif

