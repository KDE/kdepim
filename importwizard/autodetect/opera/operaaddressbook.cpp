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

#include "operaaddressbook.h"

#include <KContacts/Addressee>
#include <QUrl>
#include "importwizard_debug.h"
#include <QFile>

OperaAddressBook::OperaAddressBook(const QString &filename, ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(IMPORTWIZARD_LOG) << " We can't open file" << filename;
        return;
    }

    QTextStream stream(&file);
    bool foundContact = false;
    KContacts::Addressee *contact = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line == QLatin1String("#CONTACT")) {
            appendContact(contact);
            foundContact = true;
        } else if (line == QLatin1String("#FOLDER")) {
            appendContact(contact);
            foundContact = false;
            //TODO
        } else if (foundContact) {
            line = line.trimmed();
            if (!contact) {
                contact = new KContacts::Addressee;
            }
            if (line.startsWith(QStringLiteral("ID"))) {
                //Nothing
            } else if (line.startsWith(QStringLiteral("NAME"))) {
                contact->setName(line.remove(QStringLiteral("NAME=")));
            } else if (line.startsWith(QStringLiteral("URL"))) {
                contact->setUrl(QUrl(line.remove(QStringLiteral("URL="))));
            } else if (line.startsWith(QStringLiteral("DESCRIPTION"))) {
                contact->setNote(line.remove(QStringLiteral("DESCRIPTION=")));
            } else if (line.startsWith(QStringLiteral("PHONE"))) {
                contact->insertPhoneNumber(KContacts::PhoneNumber(line.remove(QStringLiteral("PHONE=")), KContacts::PhoneNumber::Home));
            } else if (line.startsWith(QStringLiteral("FAX"))) {
                contact->insertPhoneNumber(KContacts::PhoneNumber(line.remove(QStringLiteral("FAX=")), KContacts::PhoneNumber::Fax));
            } else if (line.startsWith(QStringLiteral("POSTALADDRESS"))) {
                //TODO
            } else if (line.startsWith(QStringLiteral("PICTUREURL"))) {
                //TODO
            } else if (line.startsWith(QStringLiteral("ICON"))) {
                //TODO
            } else if (line.startsWith(QStringLiteral("SHORT NAME"))) {
                contact->setNickName(line.remove(QStringLiteral("SHORT NAME=")));
            }
        }
    }
    appendContact(contact);
}

OperaAddressBook::~OperaAddressBook()
{

}

void OperaAddressBook::appendContact(KContacts::Addressee *contact)
{
    if (contact) {
        addImportContactNote(*contact, QStringLiteral("Opera"));
        createContact(*contact);
        delete contact;
        contact = 0;
    }
}
