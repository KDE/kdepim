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

#ifndef CONTACTGRANTLEEPRINTOBJECT_H
#define CONTACTGRANTLEEPRINTOBJECT_H
#include <QObject>
#include <KABC/Addressee>


namespace KABPrinting {
class ContactGrantleePrintAddressObject;
class ContactGrantleePrintPhoneObject;
class ContactGrantleePrintImObject;
class ContactGrantleePrintGeoObject;
class ContactGrantleePrintCryptoObject;
class ContactGrantleePrintObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ realName)
    Q_PROPERTY(QString name READ formattedName)
    Q_PROPERTY(QString name READ prefix)
    Q_PROPERTY(QString name READ givenName)
    Q_PROPERTY(QString name READ additionalName)
    Q_PROPERTY(QString name READ familyName)
    Q_PROPERTY(QString name READ suffix)
    Q_PROPERTY(QString name READ nickName)
    Q_PROPERTY(QStringList emails READ emails)
    Q_PROPERTY(QString organization READ organization)
    Q_PROPERTY(QString note READ note)
    Q_PROPERTY(QString webPage READ webPage)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString preferredEmail READ preferredEmail)
    Q_PROPERTY(QString role READ role)
    Q_PROPERTY(QString birthday READ birthday)
    Q_PROPERTY(QString department READ department)
    Q_PROPERTY(QVariant addresses READ addresses)
    Q_PROPERTY(QVariant phones READ phones)
    Q_PROPERTY(QVariant instantManging READ instantManging)
    Q_PROPERTY(QString addressBookName READ addressBookName)
    Q_PROPERTY(QString photo READ photo)
    Q_PROPERTY(QString logo READ logo)
    Q_PROPERTY(QVariant crypto READ crypto)

    //Add more functions
public:
    explicit ContactGrantleePrintObject(const KABC::Addressee &address, QObject *parent=0);
    ~ContactGrantleePrintObject();

    QString realName() const;
    QString formattedName() const;
    QString prefix() const;
    QString givenName() const;
    QString additionalName() const;
    QString familyName() const;
    QString suffix() const;
    QString nickName() const;
    QStringList emails() const;
    QString organization() const;
    QString note() const;
    QString webPage() const;
    QString title() const;
    QString preferredEmail() const;
    QString role() const;
    QString birthday() const;
    QString department() const;    
    QVariant addresses() const;
    QVariant phones() const;
    QString addressBookName() const;
    QVariant instantManging() const;
    QVariant geo() const;
    QString photo() const;
    QString logo() const;
    QVariant crypto() const;
private:
    QString imgToDataUrl( const QImage &image ) const;
    QList<ContactGrantleePrintAddressObject*> mListAddress;
    QList<ContactGrantleePrintPhoneObject*> mListPhones;
    QList<ContactGrantleePrintImObject*> mListIm;
    ContactGrantleePrintGeoObject *mGeoObject;
    ContactGrantleePrintCryptoObject *mCryptoObject;
    KABC::Addressee mAddress;
};
}
Q_DECLARE_METATYPE(QList<KABPrinting::ContactGrantleePrintObject*>)
Q_DECLARE_METATYPE(QList<KABPrinting::ContactGrantleePrintAddressObject*>)
Q_DECLARE_METATYPE(QList<KABPrinting::ContactGrantleePrintPhoneObject*>)
Q_DECLARE_METATYPE(QList<KABPrinting::ContactGrantleePrintImObject*>)
Q_DECLARE_METATYPE(KABPrinting::ContactGrantleePrintGeoObject*)
Q_DECLARE_METATYPE(KABPrinting::ContactGrantleePrintCryptoObject*)
#endif // CONTACTGRANTLEEPRINTOBJECT_H
