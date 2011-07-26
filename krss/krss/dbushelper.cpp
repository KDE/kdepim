/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "dbushelper_p.h"

#include <QtDBus/QDBusAbstractInterface>

#include <cassert>

using namespace KRss;
using namespace KRss::DBusHelper;

bool DBusHelper::callWithCallback( QDBusAbstractInterface* iface, const QString& method, const QList<QVariant>& args, QObject* receiver, const char* slot, const char* errorMethod, Timeout timeout ) {

    QDBusMessage msg = QDBusMessage::createMethodCall( iface->service(), iface->path(), iface->interface(), method );
    msg.setArguments( args );
    return iface->connection().callWithCallback( msg, receiver, slot, static_cast<int>( timeout ) );
}
