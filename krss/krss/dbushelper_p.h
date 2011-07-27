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
#ifndef KRSS_DBUSHELPER_H
#define KRSS_DBUSHELPER_H

#include <climits>

class QDBusAbstractInterface;
class QObject;
class QString;

template <typename T> class QList;
class QVariant;

namespace KRss {

    namespace DBusHelper {
        enum Timeout {
            DefaultTimeout=-1,
            NoTimeout=INT_MAX
        };

        bool callWithCallback( QDBusAbstractInterface* iface, const QString& method, const QList<QVariant>& args, QObject* receiver, const char* slot, const char* errorMethod, Timeout timeout=DefaultTimeout );
    }
}

#endif // KRSS_DBUSHELPER_H
