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

#ifndef CONTACTGRANTLEEPRINTPHONEOBJECT_H
#define CONTACTGRANTLEEPRINTPHONEOBJECT_H
#include <QObject>
#include <KABC/PhoneNumber>
namespace KABPrinting
{
class ContactGrantleePrintPhoneObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString number READ number)

public:
    explicit ContactGrantleePrintPhoneObject(const KABC::PhoneNumber &phone, QObject *parent = 0);
    ~ContactGrantleePrintPhoneObject();

    QString type() const;
    QString number() const;
private:
    KABC::PhoneNumber mPhoneNumber;
};
}

#endif // CONTACTGRANTLEEPRINTPHONEOBJECT_H
