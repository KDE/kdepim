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

#include "constants.h"
#include "generator.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Generator::Generator() : ElementWrapper()
{
}

Generator::Generator(const QDomElement& element) : ElementWrapper(element)
{
}

QString Generator::uri() const
{
    return completeURI(element().attribute(QString::fromUtf8("uri")));
    
}

QString Generator::name() const
{
    return element().text();
}

QString Generator::version() const
{
    return element().attribute(QString::fromUtf8("version"));
}

QString Generator::debugInfo() const
{
    QString info;
    info += "### Generator: ###################\n";
    if (!name().isEmpty())
        info += "name: #" + name() + "#\n";
    if (!uri().isEmpty())
        info += "uri: #" + uri() + "#\n";
    if (!version().isEmpty())
        info += "version: #" + version() + "#\n";
    info += "### Generator end ################\n";
    return info;
}

} // namespace Atom
} //namespace LibSyndication

