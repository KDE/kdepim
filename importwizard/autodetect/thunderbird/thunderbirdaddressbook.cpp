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

#include "thunderbirdaddressbook.h"
#include "addressbook/MorkParser.h"

#include <KLocalizedString>
#include <KContacts/Addressee>
#include <QUrl>
#include "importwizard_debug.h"

ThunderBirdAddressBook::ThunderBirdAddressBook(const QDir &dir, ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    readAddressBook(dir.path() + QLatin1String("/impab.mab"));
    const QStringList filesimportab = dir.entryList(QStringList(QLatin1String("impab-[0-9]*.map")), QDir::Files, QDir::Name);
    Q_FOREACH (const QString &file, filesimportab) {
        readAddressBook(dir.path() + QLatin1Char('/') + file);
    }
    readAddressBook(dir.path() + QLatin1String("/abook.mab"));

    const QStringList files = dir.entryList(QStringList(QLatin1String("abook-[0-9]*.map")), QDir::Files, QDir::Name);
    Q_FOREACH (const QString &file, files) {
        readAddressBook(dir.path() + QLatin1Char('/') + file);
    }
    readAddressBook(dir.path() + QLatin1String("/history.mab"));

    cleanUp();
}

ThunderBirdAddressBook::~ThunderBirdAddressBook()
{

}

void ThunderBirdAddressBook::readAddressBook(const QString &filename)
{
    MorkParser mork;
    if (!mork.open(filename)) {
        if (mork.error() == FailedToOpen) {
            addAddressBookImportError(i18n("Contacts file '%1' not found", filename));
        }
        qCDebug(IMPORTWIZARD_LOG) << " error during read file " << filename << " Error type " << mork.error();
        return;
    }
    MorkTableMap *tables = mork.getTables(0x80);
    if (tables) {
        MorkTableMap::iterator tableIterEnd(tables->end());
        MorkRowMap *rows = 0;
        for (MorkTableMap::iterator tableIter = tables->begin(); tableIter != tableIterEnd; ++tableIter) {
            if (tableIter.key() != 0) {
                rows = mork.getRows(0x80, &tableIter.value());
                if (rows) {
                    MorkRowMap::iterator endRow(rows->end());
                    for (MorkRowMap::iterator rowIter = rows->begin(); rowIter != endRow; ++rowIter) {
                        if (rowIter.key() != 0) {
                            KContacts::Addressee contact;
                            MorkCells cells = rowIter.value();
                            MorkCells::iterator endCellIter = cells.end();
                            KContacts::Address homeAddr = KContacts::Address(KContacts::Address::Home);
                            KContacts::Address workAddr = KContacts::Address(KContacts::Address::Work);
                            int birthday = -1;
                            int birthmonth = -1;
                            int birthyear = -1;

                            for (MorkCells::iterator cellsIter = cells.begin(); cellsIter != endCellIter; ++cellsIter) {
                                const QString value = mork.getValue(cellsIter.value());
                                const QString column = mork.getColumn(cellsIter.key());
                                qCDebug(IMPORTWIZARD_LOG) << "column :" << column << " value :" << value;
                                if (column == QLatin1String("LastModifiedDate")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("RecordKey")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AddrCharSet")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("LastRecordKey")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:table:kind:pab")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListNickName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListDescription")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListTotalAddresses")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("LowercaseListName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:table:kind:deleted")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhotoType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PreferDisplayName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhotoURI")) {
                                    KContacts::Picture photo;
                                    photo.setUrl(value);
                                    contact.setLogo(photo);
                                } else if (column == QLatin1String("PhotoName")) {
                                    //TODO: verify it
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("DbRowID")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:card:all")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:list:all")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:data:all")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FirstName")) {
                                    contact.setName(value);
                                } else if (column == QLatin1String("LastName")) {
                                    contact.setFamilyName(value);
                                } else if (column == QLatin1String("PhoneticFirstName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhoneticLastName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("DisplayName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("NickName")) {
                                    contact.setNickName(value);
                                } else if (column == QLatin1String("PrimaryEmail")) {
                                    contact.setEmails(QStringList() << value);
                                } else if (column == QLatin1String("LowercasePrimaryEmail")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("SecondEmail")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PreferMailFormat")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("MailPreferedFormatting"), value);
                                } else if (column == QLatin1String("PopularityIndex")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AllowRemoteContent")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("MailAllowToRemoteContent"), value);
                                } else if (column == QLatin1String("WorkPhone")) {
                                    contact.insertPhoneNumber(KContacts::PhoneNumber(value, KContacts::PhoneNumber::Work));
                                } else if (column == QLatin1String("HomePhone")) {
                                    contact.insertPhoneNumber(KContacts::PhoneNumber(value, KContacts::PhoneNumber::Home));
                                } else if (column == QLatin1String("FaxNumber")) {
                                    contact.insertPhoneNumber(KContacts::PhoneNumber(value, KContacts::PhoneNumber::Fax));
                                } else if (column == QLatin1String("PagerNumber")) {
                                    contact.insertPhoneNumber(KContacts::PhoneNumber(value, KContacts::PhoneNumber::Pager));
                                } else if (column == QLatin1String("CellularNumber")) {
                                    contact.insertPhoneNumber(KContacts::PhoneNumber(value, KContacts::PhoneNumber::Cell));
                                } else if (column == QLatin1String("WorkPhoneType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("HomePhoneType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FaxNumberType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PagerNumberType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("CellularNumberType")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("HomeAddress")) {
                                    homeAddr.setStreet(value);
                                } else if (column == QLatin1String("HomeAddress2")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("HomeCity")) {
                                    homeAddr.setLocality(value);
                                } else if (column == QLatin1String("HomeState")) {
                                    homeAddr.setRegion(value);
                                } else if (column == QLatin1String("HomeZipCode")) {
                                    homeAddr.setPostalCode(value);
                                } else if (column == QLatin1String("HomeCountry")) {
                                    homeAddr.setCountry(value);
                                } else if (column == QLatin1String("WorkAddress")) {
                                    workAddr.setStreet(value);
                                } else if (column == QLatin1String("WorkAddress2")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("WorkCity")) {
                                    workAddr.setLocality(value);
                                } else if (column == QLatin1String("WorkState")) {
                                    workAddr.setRegion(value);
                                } else if (column == QLatin1String("WorkZipCode")) {
                                    workAddr.setPostalCode(value);
                                } else if (column == QLatin1String("WorkCountry")) {
                                    workAddr.setCountry(value);
                                } else if (column == QLatin1String("JobTitle")) {
                                    contact.setTitle(value);
                                } else if (column == QLatin1String("Department")) {
                                    contact.setDepartment(value);
                                } else if (column == QLatin1String("Company")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("_AimScreenName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryYear")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryMonth")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryDay")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("SpouseName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FamilyName")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("WebPage1")) {
                                    KContacts::ResourceLocatorUrl url;
                                    url.setUrl(QUrl(value));
                                    contact.setUrl(url);
                                } else if (column == QLatin1String("WebPage2")) {
                                    qCDebug(IMPORTWIZARD_LOG) << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("BirthYear")) {
                                    birthyear = value.toInt();
                                } else if (column == QLatin1String("BirthMonth")) {
                                    birthmonth = value.toInt();
                                } else if (column == QLatin1String("BirthDay")) {
                                    birthday = value.toInt();
                                } else if (column == QLatin1String("Custom1")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("Custom1"), value);
                                } else if (column == QLatin1String("Custom2")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("Custom2"), value);
                                } else if (column == QLatin1String("Custom3")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("Custom3"), value);
                                } else if (column == QLatin1String("Custom4")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QStringLiteral("Custom4"), value);
                                } else if (column == QLatin1String("Notes")) {
                                    contact.setNote(value);
                                } else {
                                    qCDebug(IMPORTWIZARD_LOG) << " Columnn not implemented " << column;
                                }
                                //qCDebug(IMPORTWIZARD_LOG)<<" value :"<<value<<" column"<<column;
                            }

                            if (!homeAddr.isEmpty()) {
                                contact.insertAddress(homeAddr);
                            }
                            if (!workAddr.isEmpty()) {
                                contact.insertAddress(workAddr);
                            }

                            const QDateTime birthDate(QDate(birthyear, birthmonth, birthday));
                            if (birthDate.isValid()) {
                                contact.setBirthday(birthDate);
                            }
                            addImportContactNote(contact, QStringLiteral("Thunderbird"));
                            createContact(contact);
                            qCDebug(IMPORTWIZARD_LOG) << "-----------------------";
                        }
                    }
                }
            }
        }
    }
}
