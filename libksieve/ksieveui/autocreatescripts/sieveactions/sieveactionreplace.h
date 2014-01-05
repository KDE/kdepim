/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SIEVEACTIONREPLACE_H
#define SIEVEACTIONREPLACE_H

#include "sieveaction.h"

namespace KSieveUi {
class SieveActionReplace : public SieveAction
{
    Q_OBJECT
public:
    SieveActionReplace(QObject *parent = 0);
    static SieveAction* newAction();
    QString code(QWidget *) const;
    QWidget *createParamWidget( QWidget *parent ) const;
    bool setParamWidgetValue(const QDomElement &element, QWidget *parent, QString &error );
    QStringList needRequires(QWidget *parent) const;
    bool needCheckIfServerHasCapability() const;
    QString serverNeedsCapability() const;
    QString help() const;
    QString href() const;
};
}

#endif // SIEVEACTIONREPLACE_H
