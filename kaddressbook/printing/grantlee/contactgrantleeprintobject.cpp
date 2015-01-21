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

#include "contactgrantleeprintobject.h"
#include "contactgrantleeprintaddressobject.h"
#include "contactgrantleeprintphoneobject.h"
#include "contactgrantleeprintimobject.h"
#include "contactgrantleeprintgeoobject.h"
#include "contactgrantleeprintcryptoobject.h"

#include <KContacts/Address>
#include <KContacts/PhoneNumber>
#include <KContacts/Lang>

#include <QBuffer>

#include <grantlee/metatype.h>
#include <QLocale>

using namespace KABPrinting;

ContactGrantleePrintObject::ContactGrantleePrintObject(const KContacts::Addressee &address, QObject *parent)
    : QObject(parent),
      mAddress(address)
{
    Q_FOREACH (const KContacts::Address &addr, address.addresses()) {
        mListAddress << new ContactGrantleePrintAddressObject(addr);
    }
    Q_FOREACH (const KContacts::PhoneNumber &phone, address.phoneNumbers()) {
        mListPhones << new ContactGrantleePrintPhoneObject(phone);
    }
    const QStringList customs = mAddress.customs();
    if (!customs.empty()) {
        Q_FOREACH (const QString &custom, customs) {
            if (custom.startsWith(QLatin1String("messaging/"))) {
                const int pos = custom.indexOf(QLatin1Char(':'));
                QString key = custom.left(pos);
                key.remove(QLatin1String("-All"));
                const QString value = custom.mid(pos + 1);
                mListIm << new ContactGrantleePrintImObject(key, value);
            }
        }
    }
    mGeoObject = new ContactGrantleePrintGeoObject(address.geo());
    mCryptoObject = new ContactGrantleePrintCryptoObject(address);
}

ContactGrantleePrintObject::~ContactGrantleePrintObject()
{
    delete mGeoObject;
    qDeleteAll(mListAddress);
    qDeleteAll(mListPhones);
    qDeleteAll(mListIm);
    delete mCryptoObject;
}

QString ContactGrantleePrintObject::realName() const
{
    return mAddress.realName();
}

QString ContactGrantleePrintObject::formattedName() const
{
    return mAddress.formattedName();
}

QString ContactGrantleePrintObject::prefix() const
{
    return mAddress.prefix();
}

QString ContactGrantleePrintObject::givenName() const
{
    return mAddress.givenName();
}

QString ContactGrantleePrintObject::additionalName() const
{
    return mAddress.additionalName();
}

QString ContactGrantleePrintObject::familyName() const
{
    return mAddress.familyName();
}

QString ContactGrantleePrintObject::suffix() const
{
    return mAddress.suffix();
}

QString ContactGrantleePrintObject::nickName() const
{
    return mAddress.nickName();
}

QString ContactGrantleePrintObject::name() const
{
    return mAddress.name();
}

QString ContactGrantleePrintObject::languages() const
{
    QString result;
    QStringList langList;
    Q_FOREACH ( const KContacts::Lang &lang, mAddress.langs() ) {
        langList.append(lang.language());
    }
    if (!langList.isEmpty()) {
        result = langList.join(QLatin1String(","));
    }
    return result;
}

QStringList ContactGrantleePrintObject::emails() const
{
    QStringList emails;
    Q_FOREACH (const QString &email, mAddress.emails()) {
        const QString fullEmail = QString::fromLatin1(QUrl::toPercentEncoding(mAddress.fullEmail(email)));

        const QString url = QString::fromLatin1("<a href=\"mailto:%1\">%2</a>")
                            .arg(fullEmail, email);
        emails << url;
    }
    return emails;
}

QString ContactGrantleePrintObject::organization() const
{
    return mAddress.organization();
}

QString ContactGrantleePrintObject::note() const
{
    return mAddress.note().replace(QLatin1Char('\n'), QLatin1String("<br>"));
}

QString ContactGrantleePrintObject::webPage() const
{
    return mAddress.url().toDisplayString();
}

QString ContactGrantleePrintObject::title() const
{
    return mAddress.title();
}

QString ContactGrantleePrintObject::preferredEmail() const
{
    return mAddress.preferredEmail();
}

QString ContactGrantleePrintObject::role() const
{
    return mAddress.role();
}

QString ContactGrantleePrintObject::birthday() const
{
    return QLocale().toString(mAddress.birthday().date(), QLocale::LongFormat);
}

QString ContactGrantleePrintObject::department() const
{
    return mAddress.department();
}

QVariant ContactGrantleePrintObject::addresses() const
{
    return QVariant::fromValue(mListAddress);
}

QVariant ContactGrantleePrintObject::phones() const
{
    return QVariant::fromValue(mListPhones);
}

QVariant ContactGrantleePrintObject::instantManging() const
{
    return QVariant::fromValue(mListIm);
}

QVariant ContactGrantleePrintObject::geo() const
{
    return QVariant::fromValue(mGeoObject);
}

QVariant ContactGrantleePrintObject::crypto() const
{
    return QVariant::fromValue(mCryptoObject);
}

QString ContactGrantleePrintObject::addressBookName() const
{
    const QString addressBookName = mAddress.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("AddressBook"));
    return addressBookName;
}

QString ContactGrantleePrintObject::photo() const
{
    if (mAddress.photo().isEmpty()) {
        return QString();
    } else {
        const QString photoStr = QString::fromLatin1("<img src=\"%1\" width=\"%2\" height=\"%3\"> &nbsp;")
                                 .arg(imgToDataUrl(mAddress.photo().data()), QString::number(60), QString::number(60));
        return photoStr;
    }
}

QString ContactGrantleePrintObject::imgToDataUrl(const QImage &image) const
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QString::fromLatin1("data:image/%1;base64,%2").arg(QString::fromLatin1("PNG"), QString::fromLatin1(ba.toBase64()));
}

QString ContactGrantleePrintObject::logo() const
{
    if (mAddress.logo().isEmpty()) {
        return QString();
    } else {
        const QString photoStr = QString::fromLatin1("<img src=\"%1\" width=\"%2\" height=\"%3\"> &nbsp;")
                                 .arg(imgToDataUrl(mAddress.logo().data()), QString::number(60), QString::number(60));
        return photoStr;
    }
}

QString ContactGrantleePrintObject::anniversary() const
{
    const QDate anniversary = QDate::fromString(mAddress.custom(QLatin1String("KADDRESSBOOK"),
                              QLatin1String("X-Anniversary")), Qt::ISODate);
    if (anniversary.isValid()) {
        return (QLocale().toString(anniversary));
    }
    return QString();
}

QString ContactGrantleePrintObject::profession() const
{
    return mAddress.custom(QLatin1String("KADDRESSBOOK"),
                           QLatin1String("X-Profession"));
}

QString ContactGrantleePrintObject::office() const
{
    return mAddress.custom(QLatin1String("KADDRESSBOOK"),
                           QLatin1String("X-Office"));
}

QString ContactGrantleePrintObject::manager() const
{
    return mAddress.custom(QLatin1String("KADDRESSBOOK"),
                           QLatin1String("X-ManagersName"));
}

QString ContactGrantleePrintObject::assistant() const
{
    return mAddress.custom(QLatin1String("KADDRESSBOOK"),
                           QLatin1String("X-AssistantsName"));
}

QString ContactGrantleePrintObject::spouse() const
{
    return mAddress.custom(QLatin1String("KADDRESSBOOK"),
                           QLatin1String("X-SpousesName"));
}
