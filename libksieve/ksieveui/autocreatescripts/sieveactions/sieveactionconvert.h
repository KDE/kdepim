/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEACTIONCONVERT_H
#define SIEVEACTIONCONVERT_H

#include "sieveaction.h"

namespace KSieveUi {
class SieveActionConvert : public SieveAction
{
    Q_OBJECT
public:
    SieveActionConvert(QObject *parent = 0);
    static SieveAction* newAction();
    QString code(QWidget *) const;
    QWidget *createParamWidget( QWidget *parent ) const;
    QStringList needRequires(QWidget *parent) const;
    bool needCheckIfServerHasCapability() const;
    QString serverNeedsCapability() const;
    void setParamWidgetValue(const QDomDocument &doc, QWidget *parent ) const;
    QString help() const;
};
}

#endif // SIEVEACTIONCONVERT_H
