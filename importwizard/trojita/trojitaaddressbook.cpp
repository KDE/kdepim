/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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
        contactABC.setEmails(QStringList() << settings->value(QLatin1String("email")).toStringList());

        KContacts::Address homeAddr = KContacts::Address(KContacts::Address::Home);
        homeAddr.setLocality(settings->value(QLatin1String("city")).toString());
        homeAddr.setRegion(settings->value(QLatin1String("state")).toString());
        homeAddr.setPostalCode(settings->value(QLatin1String("zip")).toString());
        homeAddr.setCountry(settings->value(QLatin1String("country")).toString());
        homeAddr.setStreet(settings->value(QLatin1String("address")).toString());
        if (!homeAddr.isEmpty()) {
            contactABC.insertAddress(homeAddr);
        }

        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QLatin1String("phone")).toString(), KContacts::PhoneNumber::Home));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QLatin1String("workphone")).toString(), KContacts::PhoneNumber::Work));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QLatin1String("fax")).toString(), KContacts::PhoneNumber::Fax));
        contactABC.insertPhoneNumber(KContacts::PhoneNumber(settings->value(QLatin1String("mobile")).toString(), KContacts::PhoneNumber::Cell));
        contactABC.setNickName(settings->value(QLatin1String("nick")).toString());
        contactABC.setUrl(QUrl(settings->value(QLatin1String("url")).toString()));

        const QDateTime birthDate(QDate::fromString(settings->value(QLatin1String("anniversary")).toString()));
        if (birthDate.isValid()) {
            contactABC.setBirthday(birthDate);
        }
        //TODO
        //ADD(Photo, "photo");
        addImportNote(contactABC, QLatin1String("Trojita"));
        createContact(contactABC);
        settings->endGroup();
    }
}
