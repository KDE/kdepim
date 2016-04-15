/*
  Copyright (c) 2012-2016 Montel Laurent <montel@kde.org>

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

#include "sylpheedaddressbook.h"
#include <KContacts/Addressee>
#include <kcontacts/contactgroup.h>

#include "importwizard_debug.h"
#include <KLocalizedString>

#include <QDir>
#include <QDomDocument>

SylpheedAddressBook::SylpheedAddressBook(const QDir &dir, ImportWizard *parent)
    : AbstractAddressBook(parent)
{
    const QStringList files = dir.entryList(QStringList(QStringLiteral("addrbook-[0-9]*.xml")), QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        addAddressBookImportInfo(i18n("No addressbook found"));
    }

    Q_FOREACH (const QString &file, files) {
        readAddressBook(dir.path() + QLatin1Char('/') + file);
    }
    cleanUp();
}

SylpheedAddressBook::~SylpheedAddressBook()
{
}

void SylpheedAddressBook::readAddressBook(const QString &filename)
{
    QFile file(filename);
    //qCDebug(IMPORTWIZARD_LOG)<<" import filename :"<<filename;
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(IMPORTWIZARD_LOG) << " We can't open file" << filename;
        return;
    }
    QString errorMsg;
    int errorRow;
    int errorCol;
    QDomDocument doc;
    if (!doc.setContent(&file, &errorMsg, &errorRow, &errorCol)) {
        qCDebug(IMPORTWIZARD_LOG) << "Unable to load document.Parse error in line " << errorRow
                                  << ", col " << errorCol << ": " << errorMsg;
        return;
    }
    QDomElement domElement = doc.documentElement();

    if (domElement.isNull()) {
        qCDebug(IMPORTWIZARD_LOG) << "addressbook not found";
        return;
    }

    QDomElement e = domElement.firstChildElement();
    if (e.isNull()) {
        addAddressBookImportError(i18n("No contact found in %1", filename));
        return;
    }

    for (; !e.isNull(); e = e.nextSiblingElement()) {
        QString name;
        if (e.hasAttribute(QStringLiteral("name"))) {
            name = e.attribute(QStringLiteral("name"));
        }

        const QString tag = e.tagName();
        if (tag == QLatin1String("person")) {
            KContacts::Addressee contact;
            //uid="333304265" first-name="dd" last-name="ccc" nick-name="" cn="laurent"
            QString uidPerson;
            if (e.hasAttribute(QStringLiteral("uid"))) {
                uidPerson = e.attribute(QStringLiteral("uid"));
            }
            if (e.hasAttribute(QStringLiteral("first-name"))) {
                contact.setName(e.attribute(QStringLiteral("first-name")));
            }
            if (e.hasAttribute(QStringLiteral("last-name"))) {
                contact.setFamilyName(e.attribute(QStringLiteral("last-name")));

            }
            if (e.hasAttribute(QStringLiteral("nick-name"))) {
                contact.setNickName(e.attribute(QStringLiteral("nick-name")));
            }
            if (e.hasAttribute(QStringLiteral("cn"))) {
                contact.setFormattedName(e.attribute(QStringLiteral("cn")));
            }
            QStringList uidAddress;
            for (QDomElement addressElement = e.firstChildElement(); !addressElement.isNull(); addressElement = addressElement.nextSiblingElement()) {
                const QString addressTag = addressElement.tagName();
                if (addressTag == QLatin1String("address-list")) {
                    QStringList emails;
                    for (QDomElement addresslist = addressElement.firstChildElement(); !addresslist.isNull(); addresslist = addresslist.nextSiblingElement()) {
                        const QString tagAddressList = addresslist.tagName();
                        if (tagAddressList == QLatin1String("address")) {
                            if (addresslist.hasAttribute(QStringLiteral("email"))) {
                                emails << addresslist.attribute(QStringLiteral("email"));
                            } else if (addresslist.hasAttribute(QStringLiteral("alias"))) {
                                //TODO:
                            } else if (addresslist.hasAttribute(QStringLiteral("uid"))) {
                                uidAddress << addresslist.attribute(QStringLiteral("uid"));
                            }
                        } else {
                            qCDebug(IMPORTWIZARD_LOG) << " tagAddressList unknown :" << tagAddressList;
                        }
                    }
                    if (!emails.isEmpty()) {
                        contact.setEmails(emails);
                    }

                } else if (addressTag == QLatin1String("attribute-list")) {
                    for (QDomElement attributelist = addressElement.firstChildElement(); !attributelist.isNull(); attributelist = attributelist.nextSiblingElement()) {
                        const QString tagAttributeList = attributelist.tagName();
                        if (tagAttributeList == QLatin1String("attribute")) {
                            if (attributelist.hasAttribute(QStringLiteral("name"))) {
                                const QString name = attributelist.attribute(QStringLiteral("name"));
                                const QString value = attributelist.text();
                                contact.insertCustom(QStringLiteral("KADDRESSBOOK"), name, value);
                            }
                        } else {
                            qCDebug(IMPORTWIZARD_LOG) << "tagAttributeList not implemented " << tagAttributeList;
                        }
                    }

                } else {
                    qCDebug(IMPORTWIZARD_LOG) << " addressTag unknown :" << addressTag;
                }
            }
            if (!mAddressBookUid.contains(uidPerson)) {
                mAddressBookUid.insert(uidPerson, uidAddress);
            } else {
                qCDebug(IMPORTWIZARD_LOG) << " problem uidPerson already stored" << uidPerson;
            }
            addImportContactNote(contact, QStringLiteral("Sylpheed"));
            createContact(contact);
        } else if (tag == QLatin1String("group")) {
            QString name;
            if (e.hasAttribute(QStringLiteral("name"))) {
                name = e.attribute(QStringLiteral("name"));
            }
            KContacts::ContactGroup group(name);
            //TODO: create Group
            for (QDomElement groupElement = e.firstChildElement(); !groupElement.isNull(); groupElement = groupElement.nextSiblingElement()) {
                const QString groupTag = groupElement.tagName();
                if (groupTag == QLatin1String("member-list")) {
                    for (QDomElement memberlist = groupElement.firstChildElement(); !memberlist.isNull(); memberlist = memberlist.nextSiblingElement()) {
                        const QString tagMemberList = memberlist.tagName();
                        if (tagMemberList == QLatin1String("member")) {
                            QString pid;
                            QString eid;
                            if (memberlist.hasAttribute(QStringLiteral("pid"))) {
                                pid = memberlist.attribute(QStringLiteral("pid"));
                            }
                            if (memberlist.hasAttribute(QStringLiteral("eid"))) {
                                eid = memberlist.attribute(QStringLiteral("eid"));
                            }
                            if (!pid.isEmpty() && !eid.isEmpty()) {
                                //TODO
                            } else {
                                qCDebug(IMPORTWIZARD_LOG) << " Problem with group" << name;
                            }
                            //TODO
                        }
                    }
                }
            }
            createGroup(group);
        } else {
            qCDebug(IMPORTWIZARD_LOG) << " SylpheedAddressBook::readAddressBook  tag unknown :" << tag;
        }
    }
}
