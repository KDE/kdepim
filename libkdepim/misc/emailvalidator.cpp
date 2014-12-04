/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "emailvalidator.h"

#include <KEmailAddress>

using namespace KPIM;

EmailValidator::EmailValidator(QObject *parent) : QValidator(parent)
{
}

QValidator::State EmailValidator::validate(QString &str, int &pos) const
{
    Q_UNUSED(pos);

    if (KEmailAddress::isValidSimpleAddress(str)) {
        return QValidator::Acceptable;
    }

    // we'll say any string that doesn't have whitespace
    // is an intermediate email string
    if (QRegExp(QLatin1String("\\s")).indexIn(str) > -1) {
        return QValidator::Invalid;
    }

    return QValidator::Intermediate;
}

void EmailValidator::fixup(QString &str) const
{
    str = str.trimmed();
}
