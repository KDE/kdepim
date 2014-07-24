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

#include "operaaddressbook.h"

#include <KABC/Addressee>
#include <QUrl>
#include <QDebug>
#include <QFile>

OperaAddressBook::OperaAddressBook(const QString &filename, ImportWizard *parent)
    : AbstractAddressBook( parent )
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qDebug()<<" We can't open file"<<filename;
        return;
    }

    QTextStream stream(&file);
    bool foundContact = false;
    KABC::Addressee *contact = 0;
    while ( !stream.atEnd() ) {
        QString line = stream.readLine();
        if (line == QLatin1String("#CONTACT")) {
            appendContact(contact);
            foundContact = true;
        } else if (line == QLatin1String("#FOLDER")) {
            appendContact(contact);
            foundContact = false;
            //TODO
        } else if ( foundContact ) {
            line = line.trimmed();
            if (!contact) {
                contact = new KABC::Addressee;
            }
            if (line.startsWith(QLatin1String("ID"))) {
                //Nothing
            } else if (line.startsWith(QLatin1String("NAME"))) {
                contact->setName(line.remove(QLatin1String("NAME=")));
            } else if (line.startsWith(QLatin1String("URL"))) {
                contact->setUrl(QUrl(line.remove(QLatin1String("URL="))));
            } else if (line.startsWith(QLatin1String("DESCRIPTION"))) {
                contact->setNote(line.remove(QLatin1String("DESCRIPTION=")));
            } else if (line.startsWith(QLatin1String("PHONE"))) {
                contact->insertPhoneNumber( KABC::PhoneNumber( line.remove(QLatin1String("PHONE=")), KABC::PhoneNumber::Home ) );
            } else if (line.startsWith(QLatin1String("FAX"))) {
                contact->insertPhoneNumber( KABC::PhoneNumber( line.remove(QLatin1String("FAX=")), KABC::PhoneNumber::Fax ) );
            } else if (line.startsWith(QLatin1String("POSTALADDRESS"))) {
                //TODO
            } else if (line.startsWith(QLatin1String("PICTUREURL"))) {
                //TODO
            } else if (line.startsWith(QLatin1String("ICON"))) {
                //TODO
            } else if (line.startsWith(QLatin1String("SHORT NAME"))) {
                contact->setNickName(line.remove(QLatin1String("SHORT NAME=")));
            }
        }
    }
    appendContact(contact);
}

OperaAddressBook::~OperaAddressBook()
{

}

void OperaAddressBook::appendContact(KABC::Addressee *contact)
{
    if (contact) {
        addImportNote( *contact, QLatin1String("Opera"));
        createContact( *contact );
        delete contact;
        contact = 0;
    }
}
