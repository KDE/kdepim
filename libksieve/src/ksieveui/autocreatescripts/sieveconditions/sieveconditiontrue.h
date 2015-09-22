/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVECONDITIONTRUE_H
#define SIEVECONDITIONTRUE_H

#include "sievecondition.h"

namespace KSieveUi
{
class SieveConditionTrue : public SieveCondition
{
    Q_OBJECT
public:
    explicit SieveConditionTrue(QObject *parent = Q_NULLPTR);

    static SieveCondition *newAction();

    QWidget *createParamWidget(QWidget *parent) const Q_DECL_OVERRIDE;

    QString code(QWidget *w) const Q_DECL_OVERRIDE;

    QString help() const Q_DECL_OVERRIDE;

    bool setParamWidgetValue(const QDomElement &element, QWidget *parent, bool notCondition, QString &error) Q_DECL_OVERRIDE;
    QUrl href() const Q_DECL_OVERRIDE;
};
}

#endif // SIEVECONDITIONTRUE_H
