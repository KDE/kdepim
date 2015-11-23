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

#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include "kdepim_export.h"

#include <QValidator>

namespace KPIM
{

/**
  A validator that enforces correct email addresses.
  @see KEmailAddress::isValidSimpleAddress
*/
class KDEPIM_EXPORT EmailValidator : public QValidator //krazy:exclude=dpointer
{
    Q_OBJECT
public:
    explicit EmailValidator(QObject *parent);

    State validate(QString &str, int &pos) const Q_DECL_OVERRIDE;

    void fixup(QString &str) const Q_DECL_OVERRIDE;
};

}

#endif
