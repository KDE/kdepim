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

#include "sylpheedaddressbook.h"
#include <KABC/Addressee>
#include <kabc/contactgroup.h>

#include <KDebug>
#include <KLocale>

#include <QDir>
#include <QDebug>
#include <QDomDocument>

SylpheedAddressBook::SylpheedAddressBook(const QDir& dir, ImportWizard *parent)
    : AbstractAddressBook( parent )
{
    const QStringList files = dir.entryList(QStringList("addrbook-[0-9]*.xml" ), QDir::Files, QDir::Name);
    Q_FOREACH ( const QString& file, files ) {
        readAddressBook( dir.path() + QLatin1Char( '/' ) + file );
    }
    cleanUp();
}

SylpheedAddressBook::~SylpheedAddressBook()
{
}

void SylpheedAddressBook::readAddressBook( const QString& filename )
{
    QFile file(filename);
    //kDebug()<<" import filename :"<<filename;
    if ( !file.open( QIODevice::ReadOnly ) ) {
        kDebug()<<" We can't open file"<<filename;
        return;
    }
    QString errorMsg;
    int errorRow;
    int errorCol;
    QDomDocument doc;
    if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
        kDebug() << "Unable to load document.Parse error in line " << errorRow
                 << ", col " << errorCol << ": " << errorMsg;
        return;
    }
    QDomElement domElement = doc.documentElement();

    if ( domElement.isNull() ) {
        kDebug() << "addressbook not found";
        return;
    }

    QDomElement e = domElement.firstChildElement();
    if (e.isNull()) {
        addAddressBookImportError( i18n("No contact found in %1", filename) );
        return;
    }

    for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
        QString name;
        if ( e.hasAttribute( QLatin1String( "name" ) ) ) {
            name = e.attribute( QLatin1String( "name" ) );
        }

        const QString tag = e.tagName();
        if ( tag == QLatin1String( "person" ) ) {
            KABC::Addressee contact;
            //uid="333304265" first-name="dd" last-name="ccc" nick-name="" cn="laurent"
            QString uidPerson;
            if ( e.hasAttribute( QLatin1String( "uid" ) ) ) {
                uidPerson = e.attribute( QLatin1String( "uid" ) );
            }
            if ( e.hasAttribute( QLatin1String( "first-name" ) ) ) {
                contact.setName( e.attribute( QLatin1String( "first-name" ) ) );
            }
            if ( e.hasAttribute( QLatin1String( "last-name" ) ) ) {
                contact.setFamilyName( e.attribute( QLatin1String( "last-name" ) ) );

            }
            if ( e.hasAttribute( QLatin1String( "nick-name" ) ) ) {
                contact.setNickName( e.attribute(QLatin1String( "nick-name" )) );
            }
            if ( e.hasAttribute( QLatin1String( "cn" ) ) ) {
                contact.setFormattedName(e.attribute(QLatin1String( "cn" )));
            }
            QStringList uidAddress;
            for ( QDomElement addressElement = e.firstChildElement(); !addressElement.isNull(); addressElement = addressElement.nextSiblingElement() ) {
                const QString addressTag = addressElement.tagName();
                if ( addressTag == QLatin1String( "address-list" ) ) {
                    QStringList emails;
                    for ( QDomElement addresslist = addressElement.firstChildElement(); !addresslist.isNull(); addresslist = addresslist.nextSiblingElement() ) {
                        const QString tagAddressList = addresslist.tagName();
                        if ( tagAddressList == QLatin1String( "address" ) ) {
                            if ( addresslist.hasAttribute( QLatin1String( "email" ) ) ) {
                                emails<<addresslist.attribute( QLatin1String( "email" ) );
                            } else if (addresslist.hasAttribute(QLatin1String("alias"))) {
                                //TODO:
                            } else if (addresslist.hasAttribute(QLatin1String("uid"))) {
                                uidAddress<<addresslist.attribute(QLatin1String("uid"));
                            }
                        } else {
                            kDebug()<<" tagAddressList unknown :"<<tagAddressList;
                        }
                    }
                    if ( !emails.isEmpty() ) {
                        contact.setEmails( emails );
                    }

                } else if ( addressTag == QLatin1String( "attribute-list" ) ) {
                    for ( QDomElement attributelist = addressElement.firstChildElement(); !attributelist.isNull(); attributelist = attributelist.nextSiblingElement() ) {
                        const QString tagAttributeList = attributelist.tagName();
                        if ( tagAttributeList == QLatin1String( "attribute" ) ) {
                            if (attributelist.hasAttribute(QLatin1String("name"))) {
                                const QString name = attributelist.attribute(QLatin1String("name"));
                                const QString value = attributelist.text();
                                contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), name, value );
                            }
                        } else {
                            kDebug()<<"tagAttributeList not implemented "<<tagAttributeList;
                        }
                    }

                } else {
                    kDebug()<<" addressTag unknown :"<<addressTag;
                }
            }
            if (!mAddressBookUid.contains(uidPerson)) {
                mAddressBookUid.insert(uidPerson,uidAddress);
            } else {
                kDebug()<<" problem uidPerson already stored"<<uidPerson;
            }
            addImportNote(contact, QLatin1String("Sylpheed"));
            createContact( contact );
        } else if (tag == QLatin1String("group")) {
            QString name;
            if ( e.hasAttribute( QLatin1String( "name" ) ) ) {
                name = e.attribute( QLatin1String( "name" ) );
            }
            KABC::ContactGroup group(name);
            //TODO: create Group
            for ( QDomElement groupElement = e.firstChildElement(); !groupElement.isNull(); groupElement = groupElement.nextSiblingElement() ) {
                const QString groupTag = groupElement.tagName();
                if ( groupTag == QLatin1String( "member-list" ) ) {
                    for ( QDomElement memberlist = groupElement.firstChildElement(); !memberlist.isNull(); memberlist = memberlist.nextSiblingElement() ) {
                        const QString tagMemberList = memberlist.tagName();
                        if (tagMemberList == QLatin1String("member")) {
                            QString pid;
                            QString eid;
                            if (memberlist.hasAttribute(QLatin1String("pid"))) {
                                pid = memberlist.attribute(QLatin1String("pid"));
                            }
                            if (memberlist.hasAttribute(QLatin1String("eid"))) {
                                eid = memberlist.attribute(QLatin1String("eid"));
                            }
                            if (!pid.isEmpty()&&!eid.isEmpty()) {
                                //TODO
                            } else {
                                qDebug()<<" Problem with group"<<name;
                            }
                            //TODO
                        }
                    }
                }
            }
            createGroup(group);
        } else {
            kDebug()<<" SylpheedAddressBook::readAddressBook  tag unknown :"<<tag;
        }
    }
}
