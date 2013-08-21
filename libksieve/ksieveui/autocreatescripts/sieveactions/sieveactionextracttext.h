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


#ifndef SIEVEACTIONEXTRACTTEXT_H
#define SIEVEACTIONEXTRACTTEXT_H

#include "sieveaction.h"
namespace KSieveUi {
class SieveActionExtractText : public SieveAction
{
    Q_OBJECT
public:
    SieveActionExtractText(QObject *parent = 0);
    static SieveAction* newAction();
    QWidget *createParamWidget( QWidget *parent ) const;
    QString code(QWidget *) const;
    QStringList needRequires(QWidget *parent) const;
    bool needCheckIfServerHasCapability() const;
    bool setParamWidgetValue(const QDomElement &element, QWidget *parent, QString &error );
    QString serverNeedsCapability() const;
    QString help() const;
};
}


#endif // SIEVEACTIONEXTRACTTEXT_H
