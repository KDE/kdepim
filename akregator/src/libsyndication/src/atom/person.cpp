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
#include "person.h"
#include "tools.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Person::Person() : ElementWrapper()
{
}

Person::Person(const QDomElement& element) : ElementWrapper(element)
{
}

QString Person::name() const
{
    return Tools::extractElementTextNS(element(), Constants::atom1NameSpace(),
                                QString::fromLatin1("name"));
}

QString Person::uri() const
{
    return Tools::extractElementTextNS(element(), Constants::atom1NameSpace(),
                                QString::fromLatin1("uri"));
}

QString Person::email() const
{
    return Tools::extractElementTextNS(element(), Constants::atom1NameSpace(),
                                QString::fromLatin1("email"));
}

QString Person::debugInfo() const
{
    QString info;
    info += "### Person: ###################\n";
    info += "name: #" + name() + "#\n";
    info += "email: #" + email() + "#\n";
    info += "uri: #" + uri() + "#\n";
    info += "### Person end ################\n";

    return info;
}

} // namespace Atom
} //namespace LibSyndication
