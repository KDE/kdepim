/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "enclosure.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace RSS2 {


Enclosure Enclosure::fromXML(const QDomElement& e)
{
    return Enclosure(e);
}

Enclosure::Enclosure() : ElementWrapper()
{
}

Enclosure::Enclosure(const QDomElement& element) : ElementWrapper(element)
{
}

QString Enclosure::url() const
{
    return element().attribute(QString::fromLatin1("url"));
}

int Enclosure::length() const
{
    int length = -1;

    if (element().hasAttribute(QString::fromLatin1("length")))
    {
        bool ok;
        int c = element().attribute(QString::fromLatin1("length")).toInt(&ok);
        length = ok ? c : -1;
    }
    return length;
}

QString Enclosure::type() const
{
    return element().attribute(QString::fromLatin1("type"));
}

QString Enclosure::debugInfo() const
{
    QString info;
    info += "### Enclosure: ###################\n";
    info += "url: #" + url() + "#\n";
    info += "type: #" + type() + "#\n";
    info += "length: #" + QString::number(length()) + "#\n";
    info += "### Enclosure end ################\n";
    return info;
}

} // namespace RSS2
}  // namespace LibSyndication
