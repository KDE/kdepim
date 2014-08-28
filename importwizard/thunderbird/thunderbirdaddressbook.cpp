/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include <KABC/Addressee>
#include <QUrl>
#include <QDebug>

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
        qDebug() << " error during read file " << filename << " Error type " << mork.error();
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
                            KABC::Addressee contact;
                            MorkCells cells = rowIter.value();
                            MorkCells::iterator endCellIter = cells.end();
                            KABC::Address homeAddr = KABC::Address(KABC::Address::Home);
                            KABC::Address workAddr = KABC::Address(KABC::Address::Work);
                            int birthday = -1;
                            int birthmonth = -1;
                            int birthyear = -1;

                            for (MorkCells::iterator cellsIter = cells.begin(); cellsIter != endCellIter; ++cellsIter) {
                                const QString value = mork.getValue(cellsIter.value());
                                const QString column = mork.getColumn(cellsIter.key());
                                qDebug() << "column :" << column << " value :" << value;
                                if (column == QLatin1String("LastModifiedDate")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("RecordKey")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AddrCharSet")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("LastRecordKey")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:table:kind:pab")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListNickName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListDescription")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ListTotalAddresses")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("LowercaseListName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:table:kind:deleted")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhotoType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PreferDisplayName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhotoURI")) {
                                    KABC::Picture photo;
                                    photo.setUrl(value);
                                    contact.setLogo(photo);
                                } else if (column == QLatin1String("PhotoName")) {
                                    //TODO: verify it
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("DbRowID")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:card:all")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:list:all")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("ns:addrbk:db:row:scope:data:all")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FirstName")) {
                                    contact.setName(value);
                                } else if (column == QLatin1String("LastName")) {
                                    contact.setFamilyName(value);
                                } else if (column == QLatin1String("PhoneticFirstName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PhoneticLastName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("DisplayName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("NickName")) {
                                    contact.setNickName(value);
                                } else if (column == QLatin1String("PrimaryEmail")) {
                                    contact.setEmails(QStringList() << value);
                                } else if (column == QLatin1String("LowercasePrimaryEmail")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("SecondEmail")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PreferMailFormat")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("MailPreferedFormatting"), value);
                                } else if (column == QLatin1String("PopularityIndex")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AllowRemoteContent")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("MailAllowToRemoteContent"), value);
                                } else if (column == QLatin1String("WorkPhone")) {
                                    contact.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Work));
                                } else if (column == QLatin1String("HomePhone")) {
                                    contact.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Home));
                                } else if (column == QLatin1String("FaxNumber")) {
                                    contact.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Fax));
                                } else if (column == QLatin1String("PagerNumber")) {
                                    contact.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Pager));
                                } else if (column == QLatin1String("CellularNumber")) {
                                    contact.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Cell));
                                } else if (column == QLatin1String("WorkPhoneType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("HomePhoneType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FaxNumberType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("PagerNumberType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("CellularNumberType")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("HomeAddress")) {
                                    homeAddr.setStreet(value);
                                } else if (column == QLatin1String("HomeAddress2")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
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
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
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
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("_AimScreenName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryYear")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryMonth")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("AnniversaryDay")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("SpouseName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("FamilyName")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("WebPage1")) {
                                    contact.setUrl(QUrl(value));
                                } else if (column == QLatin1String("WebPage2")) {
                                    qDebug() << " column " << column << " found but not imported. Need to look at how to import it";
                                } else if (column == QLatin1String("BirthYear")) {
                                    birthyear = value.toInt();
                                } else if (column == QLatin1String("BirthMonth")) {
                                    birthmonth = value.toInt();
                                } else if (column == QLatin1String("BirthDay")) {
                                    birthday = value.toInt();
                                } else if (column == QLatin1String("Custom1")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("Custom1"), value);
                                } else if (column == QLatin1String("Custom2")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("Custom2"), value);
                                } else if (column == QLatin1String("Custom3")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("Custom3"), value);
                                } else if (column == QLatin1String("Custom4")) {
                                    contact.insertCustom(QLatin1String("KADDRESSBOOK"), QLatin1String("Custom4"), value);
                                } else if (column == QLatin1String("Notes")) {
                                    contact.setNote(value);
                                } else {
                                    qDebug() << " Columnn not implemented " << column;
                                }
                                //qDebug()<<" value :"<<value<<" column"<<column;
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
                            addImportNote(contact, QLatin1String("Thunderbird"));
                            createContact(contact);
                            qDebug() << "-----------------------";
                        }
                    }
                }
            }
        }
    }
}
