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

#ifndef CONTACTGRANTLEEPRINTADDRESSOBJECT_H
#define CONTACTGRANTLEEPRINTADDRESSOBJECT_H
#include <QObject>
#include <KContacts/Addressee>
namespace KABPrinting
{
class ContactGrantleePrintAddressObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString street READ street)
    Q_PROPERTY(QString postOfficeBox READ postOfficeBox)
    Q_PROPERTY(QString locality READ locality)
    Q_PROPERTY(QString region READ region)
    Q_PROPERTY(QString postalCode READ postalCode)
    Q_PROPERTY(QString country READ country)
    Q_PROPERTY(QString label READ label)
    Q_PROPERTY(QString formattedAddress READ formattedAddress)

public:
    explicit ContactGrantleePrintAddressObject(const KContacts::Address &address, QObject *parent = Q_NULLPTR);
    ~ContactGrantleePrintAddressObject();

    QString type() const;
    QString street() const;
    QString postOfficeBox() const;
    QString locality() const;
    QString region() const;
    QString postalCode() const;
    QString country() const;
    QString label() const;
    QString formattedAddress() const;

private:
    KContacts::Address mAddress;
};
}
#endif // CONTACTGRANTLEEPRINTADDRESSOBJECT_H
