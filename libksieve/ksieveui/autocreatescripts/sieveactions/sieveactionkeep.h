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

#ifndef SIEVEACTIONKEEP_H
#define SIEVEACTIONKEEP_H
#include "sieveaction.h"
namespace KSieveUi
{
class SieveActionKeep : public SieveAction
{
    Q_OBJECT
public:
    SieveActionKeep(QObject *parent = 0);
    static SieveAction *newAction();

    QString code(QWidget *) const;
    QString help() const;
    QWidget *createParamWidget(QWidget *parent) const;
    bool setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error);
    QStringList needRequires(QWidget *) const;
    QString href() const;

private:
    bool mHasFlagSupport;
    bool mHasImapFlag4Support;
};
}
#endif // SIEVEACTIONKEEP_H
