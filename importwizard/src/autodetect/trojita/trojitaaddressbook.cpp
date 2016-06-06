/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "trojitaaddressbook.h"
#include <KContacts/Addressee>

#include <QUrl>

#include <QSettings>

TrojitaAddressBook::TrojitaAddressBook(const QString &filename, ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    settings = new QSettings(filename, QSettings::IniFormat, this);
    settings->setIniCodec("UTF-8");
    readAddressBook();
}

TrojitaAddressBook::~TrojitaAddressBook()
{
    delete settings;
}

void TrojitaAddressBook::readAddressBook()
{
    const QStringList contacts = settings->childGroups();
    Q_FOREACH (const QString &contact, contacts) {
        KContacts::Addressee contactABC;
        settings->beginGroup(contact);
        contactABC.setEmails(QStringList() << settings->value(QStringLiteral("email")).toStringList());

        KContacts::Address homeAddr = KContacts::Address(KContacts::Address::Home);
        homeAddr.setLocality(settings->value(QStringLiteral("city")).toString());
        homeAddr.setRegion(settings->value(QStringLiteral("state")).toString());
        homeAddr.setPostalCode(settings->value(QStringLiteral("zip")).toString());
        homeAddr.setCountry(settings->value(QStringLiteral("country")).toString());
        homeAddr.setStreet(settings->value(QStringLiteral("address")).toString());
        if (!homeAddr.isEmpty()) {
            contactABC.insertAddress(homeAddr);
        }

        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QStringLiteral("phone")).toString(), KContacts::PhoneNumber::Home));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QStringLiteral("workphone")).toString(), KContacts::PhoneNumber::Work));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QStringLiteral("fax")).toString(), KContacts::PhoneNumber::Fax));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QStringLiteral("mobile")).toString(), KContacts::PhoneNumber::Cell));
        contactABC.setNickName(settings->value(QStringLiteral("nick")).toString());
        KContacts::ResourceLocatorUrl url;
        url.setUrl(QUrl(settings->value(QStringLiteral("url")).toString()));
        contactABC.setUrl(url);

        const QDateTime birthDate(QDate::fromString(settings->value(QStringLiteral("anniversary")).toString()));
        if (birthDate.isValid()) {
            contactABC.setBirthday(birthDate);
        }
        //TODO
        //ADD(Photo, "photo");
        addImportContactNote(contactABC, QStringLiteral("Trojita"));
        createContact(contactABC);
        settings->endGroup();
    }
}
