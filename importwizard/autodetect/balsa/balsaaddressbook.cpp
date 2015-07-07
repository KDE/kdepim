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

#include "balsaaddressbook.h"
#include "importwizardutil.h"

#include <KContacts/Addressee>
#include <KContacts/LDIFConverter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QUrl>

#include "importwizard_debug.h"
#include <QFile>
#include <QFileInfo>

BalsaAddressBook::BalsaAddressBook(const QString &filename, ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    KConfig config(filename);
    const QStringList addressBookList = config.groupList().filter(QRegExp("address-book-\\+d"));
    if (addressBookList.isEmpty()) {
        addAddressBookImportInfo(i18n("No addressbook found"));
    } else {
        Q_FOREACH (const QString &addressbook, addressBookList) {
            KConfigGroup grp = config.group(addressbook);
            readAddressBook(grp);
        }
    }
}

BalsaAddressBook::~BalsaAddressBook()
{

}

void BalsaAddressBook::readAddressBook(const KConfigGroup &grp)
{
    const QString type = grp.readEntry(QStringLiteral("Type"));
    if (type.isEmpty()) {
        addAddressBookImportInfo(i18n("No addressbook found"));
        return;
    }
    const QString name = grp.readEntry(QStringLiteral("Name"));

    if (type == QLatin1String("LibBalsaAddressBookLdap")) {
        ldapStruct ldap;
        ldap.dn = grp.readEntry(QStringLiteral("BaseDN"));
        ldap.useTLS = (grp.readEntry(QStringLiteral("EnableTLS")) == QLatin1String("true"));
        ldap.ldapUrl = QUrl(grp.readEntry(QStringLiteral("Host")));
        ldap.port = ldap.ldapUrl.port();
        //TODO: verify
        const QString bookDN  = grp.readEntry(QStringLiteral("BookDN")); //TODO ?
        ImportWizardUtil::mergeLdap(ldap);
        addAddressBookImportInfo(i18n("Ldap created"));
    } else if (type == QLatin1String("LibBalsaAddressBookGpe")) {
        qCDebug(IMPORTWIZARD_LOG) << " Import it !";
    } else if (type == QLatin1String("LibBalsaAddressBookLdif")) {
        const QString path = grp.readEntry(QStringLiteral("Path"));
        if (!path.isEmpty()) {
            KContacts::Addressee::List contacts;
            KContacts::ContactGroup::List contactsGroup;
            QFile file(path);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream.setCodec("ISO 8859-1");

                const QString wholeFile = stream.readAll();
                const QDateTime dtDefault = QFileInfo(file).lastModified();
                file.close();

                KContacts::LDIFConverter::LDIFToAddressee(wholeFile, contacts, contactsGroup, dtDefault);
                Q_FOREACH (KContacts::Addressee contact, contacts) {
                    addImportContactNote(contact, QStringLiteral("Balsa"));
                    createContact(contact);
                }
            }
        }
    } else if (type == QLatin1String("LibBalsaAddressBookVcard")) {
        const QString path = grp.readEntry(QStringLiteral("Path"));
        if (!path.isEmpty()) {
            QMap<QString, QVariant> settings;
            settings.insert(QStringLiteral("Path"), path);
            settings.insert(QStringLiteral("DisplayName"), name);
            addAddressBookImportInfo(i18n("New addressbook created: %1", createResource(QStringLiteral("akonadi_vcard_resource") , name, settings)));
        }
    } else {
        qCDebug(IMPORTWIZARD_LOG) << " unknown addressbook type :" << type;
    }
}
