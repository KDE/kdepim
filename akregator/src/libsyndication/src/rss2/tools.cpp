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

#include "tools.h"
#include "../constants.h"
#include "../elementwrapper.h"

#include <QDomElement>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace RSS2 {

QString extractContent(const ElementWrapper& wrapper)
{
    if (wrapper.isNull())
        return QString::null;
    
    QList<QDomElement> list = wrapper.elementsByTagNameNS(contentNameSpace(), QString::fromUtf8("encoded"));

    if (!list.isEmpty())
        return list.first().text().simplified();

    list = wrapper.elementsByTagNameNS(xhtmlNamespace(), QString::fromUtf8("body"));

    if (!list.isEmpty())
        return ElementWrapper::childNodesAsXML(list.first()).simplified();

    list = wrapper.elementsByTagNameNS(xhtmlNamespace(), QString::fromUtf8("div"));

    if (!list.isEmpty())
        return ElementWrapper::childNodesAsXML(list.first()).simplified();

    return QString::null;
}

} // namespace RSS2
} // namespace LibSyndication
