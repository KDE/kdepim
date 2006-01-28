/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "tools.h"

#include <kcodecs.h> 

#include <QByteArray>
#include <QDateTime>
#include <QString>

namespace LibSyndication {

KMD5 md5Machine;

unsigned int calcHash(const QString& str)
{
    return calcHash(str.utf8());
}

unsigned int calcHash(const QByteArray& array)
{
    if (array.isEmpty())
    {
        return 0;
    }
    else
    {
        const char* s = array.data();
        unsigned int hash = 5381;
        int c;
        while ( ( c = *s++ ) ) hash = ((hash << 5) + hash) + c; // hash*33 + c
        return hash;
    }
}

time_t parseISODate(const QString& str)
{
    QDateTime dt = QDateTime::fromString(str, Qt::ISODate);
    time_t time = dt.toTime_t() ;
    if (time == -1)
        return 0;
    return time;
}

time_t parseRFCDate(const QString& str)
{
    QDateTime dt = QDateTime::fromString(str, Qt::TextDate);
    time_t time = dt.toTime_t() ;
    if (time == -1)
        return 0;
    return time;
}

QString calcMD5Sum(const QString& str)
{
    md5Machine.reset();
    md5Machine.update(str.utf8());
    return QString(md5Machine.hexDigest().data());
}

} // namespace LibSyndication


