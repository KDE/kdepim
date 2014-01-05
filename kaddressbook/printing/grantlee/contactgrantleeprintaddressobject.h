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

#ifndef CONTACTGRANTLEEPRINTADDRESSOBJECT_H
#define CONTACTGRANTLEEPRINTADDRESSOBJECT_H
#include <QObject>
#include <KABC/Addressee>
namespace KABPrinting {
class ContactGrantleePrintAddressObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ type)
    Q_PROPERTY(QString name READ street)
    Q_PROPERTY(QString name READ postOfficeBox)
    Q_PROPERTY(QString name READ locality)
    Q_PROPERTY(QString name READ region)
    Q_PROPERTY(QString name READ postalCode)
    Q_PROPERTY(QString name READ country)
    Q_PROPERTY(QString name READ label)
    Q_PROPERTY(QString name READ formattedAddress)

public:
    explicit ContactGrantleePrintAddressObject(const KABC::Address &address, QObject *parent=0);
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
    KABC::Address mAddress;
};
}
#endif // CONTACTGRANTLEEPRINTADDRESSOBJECT_H
