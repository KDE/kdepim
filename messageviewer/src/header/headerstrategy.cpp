/*  -*- c++ -*-
    headerstrategy.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "headerstrategy.h"

#include <qdebug.h>

//
// HeaderStrategy abstract base:
//
namespace MessageViewer
{
HeaderStrategy::HeaderStrategy()
{

}

HeaderStrategy::~HeaderStrategy()
{

}

QStringList HeaderStrategy::headersToDisplay() const
{
    return QStringList();
}

QStringList HeaderStrategy::headersToHide() const
{
    return QStringList();
}

bool HeaderStrategy::showHeader(const QString &header) const
{
    const QString headerLower(header.toLower());
    if (headersToDisplay().contains(headerLower)) {
        return true;
    }
    if (headersToHide().contains(headerLower)) {
        return false;
    }
    return defaultPolicy() == Display;
}

QStringList HeaderStrategy::stringList(const char *const headers[], int numHeaders)
{
    QStringList sl;
    sl.reserve(numHeaders);
    for (int i = 0; i < numHeaders; ++i) {
        sl.push_back(QLatin1String(headers[i]));
    }
    return sl;
}

}
