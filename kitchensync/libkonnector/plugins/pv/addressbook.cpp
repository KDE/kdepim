/***************************************************************************
                        addressbook.cpp  -  description
                             -------------------
    begin                : Wed Oct 23 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qbuffer.h>
#include <qtextstream.h>
#include <qdir.h>

#include <kdebug.h>

#include <kabc/resourcefile.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

#include <idhelper.h>

#include "addressbook.h"

using namespace KSync;
using namespace PVHelper;

AddressBookSyncee* AddressBook::toAddressBookSyncee(QDomNode& n)
{
  kdDebug() << "Begin::AddressBook::toAddressBookSyncee" << endl;
  AddressBookSyncee *syncee = new AddressBookSyncee();
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/" + "55");

  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    if( e.tagName() == QString::fromLatin1("contact"))
    {
      kdDebug() << "contact found!" << endl;
      // convert XML contact to addressee
      KABC::Addressee adr;
//      kdDebug() << "addressee uid: " << adr.uid() << endl;
//      helper.addId("AddressBookSyncEntry", e.attribute("uid"), adr.uid());
      adr.setUid(helper.kdeId("AddressBookSyncEntry", "Konnector-" + e.attribute("uid"), "Konnector-" + e.attribute("uid")));
//      kdDebug() << "addressee uid after kdeId: " << adr.uid() << endl;
      adr.insertCategory(e.attribute("category"));
      // Get subentries
      QDomNode n = e.firstChild();
      QDomElement el = n.toElement();

      while(!n.isNull()) // get all sub entries of element contact
      {
        QDomElement el = n.toElement();
        if (el.tagName() == QString::fromLatin1("name"))
        {
          adr.setFamilyName(el.text());
        }
        else if (el.tagName() == QString::fromLatin1("homenumber"))
        {
          KABC::PhoneNumber homePhoneNum( el.text(),
              KABC::PhoneNumber::Home );
          adr.insertPhoneNumber(homePhoneNum );
        }
        else if (el.tagName() == QString::fromLatin1("businessnumber"))
        {
          KABC::PhoneNumber businessPhoneNum(el.text(),
              KABC::PhoneNumber::Work);
          adr.insertPhoneNumber( businessPhoneNum );
        }
        else if (el.tagName() == QString::fromLatin1("faxnumber"))
        {
          KABC::PhoneNumber homeFax (el.text(),
              KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
          adr.insertPhoneNumber(homeFax );
        }
        else if (el.tagName() == QString::fromLatin1("businessfax"))
        {
          KABC::PhoneNumber businessFaxNum ( el.text(),
               KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax);
          adr.insertPhoneNumber( businessFaxNum );
        }
        else if (el.tagName() == QString::fromLatin1("mobile"))
        {
          KABC::PhoneNumber businessMobile ( el.text(),
              KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
          adr.insertPhoneNumber( businessMobile);
        }
        else if (el.tagName() == QString::fromLatin1("address"))
        {
          KABC::Address home( KABC::Address::Home );
          home.setStreet(el.text());
          adr.insertAddress(home);
        }
        else if (el.tagName() == QString::fromLatin1("email"))
        {
          adr.insertEmail(el.text(), true); // preferred
        }
        else if (el.tagName() == QString::fromLatin1("employer"))
        {
          adr.setOrganization(el.text());
        }
        else if (el.tagName() == QString::fromLatin1("businessaddress"))
        {
          KABC::Address business( KABC::Address::Work );
          business.setStreet(el.text());
          adr.insertAddress( business );
        }
        else if (el.tagName() == QString::fromLatin1("department"))
        {
          adr.setOrganization(el.text());
        }
        else if (el.tagName() == QString::fromLatin1("position"))
        {
          adr.setRole(el.text());
        }
        else if (el.tagName() == QString::fromLatin1("note"))
        {
          adr.setNote(el.text());
        }
        n = n.nextSibling();
      }  // end of while
      adr.setRevision( QDateTime::currentDateTime() );
      KSync::AddressBookSyncEntry* entry = new KSync::AddressBookSyncEntry( adr );
      syncee->addEntry(entry);  // add entry to syncee
    }  // end of if contact
    else
    {
      kdDebug() << "PVHelper::XML2Syncee -> contact not found" << endl;
      return 0l; // xxx bessere fehlerbehandlung!
    }
    n = n.nextSibling(); // jump to next element
  }  // end of while
  kdDebug() << "End::AddressBook::toAddressBookSyncee" << endl;
  return syncee;
}

QByteArray AddressBook::toXML(AddressBookSyncee* syncee)
{
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/" + "55");

  QByteArray array;
  QBuffer buffer(array);
  QTextStream stream( &buffer );
  if (buffer.open( IO_WriteOnly))
  {
    stream.setEncoding( QTextStream::UnicodeUTF8 );
    stream << "<contacts>" << endl;
// for all entries
    KABC::Addressee ab;
    KSync::AddressBookSyncEntry *entry;
    QString state;
    for (entry = syncee->firstEntry(); entry != 0l; entry = syncee->nextEntry())
    {
      kdDebug() << "Cycle entries" << endl;
      switch (entry->state())
      {
        case KSync::SyncEntry::Removed:
          state = "removed";
          kdDebug() << "Entry was removed" << endl;
          break;
        case KSync::SyncEntry::Modified:
          state = "modified";
          kdDebug() << "Entry was modified" << endl;
          break;
        case KSync::SyncEntry::Added:
          state = "added";
          kdDebug() << "Entry was added" << endl;
          break;
        case KSync::SyncEntry::Undefined:
          state = "undefined";
          kdDebug() << "Entry undefined" << endl;
          // xxx wieso sind alle undefined?? continue;
          break;
      }
      ab = entry->addressee();

      QStringList categories = ab.categories(); // xxx only one category supported yet!
      // Check if new entry (not in uid map)
      QString konnectorId = helper.konnectorId("AddressBookSyncEntry", ab.uid(), "newId");
      if (konnectorId == "newId")
      {
        state = "added";
      }
      stream << "<contact uid='" << helper.konnectorId("AddressBookSyncEntry", ab.uid())
              << "' category='" << categories[0] << "' state='" << state << "'>" << endl;

      stream << "<name>" << ab.familyName() << "</name>" << endl;

      KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
      stream << "<homenumber>" << homePhoneNum.number() << "</homenumber>" << endl;

      KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
      stream << "<businessnumber>" << businessPhoneNum.number() << "</businessnumber>" << endl;

      KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
      stream << "<faxnumber>" << homeFax.number() << "</faxnumber>" << endl;

      KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
      stream << "<businessfax>" << businessFaxNum.number() << "</businessfax>" << endl;

      KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
      stream << "<mobile>" << businessMobile.number() << "</mobile>" << endl;

      KABC::Address home = ab.address( KABC::Address::Home );
      stream << "<address>" << home.street() << "</address>" << endl;

      QStringList list = ab.emails();
      if ( list.count() > 0 )
      {
        stream << "<email>" << list[0] << "</email>" << endl;
      }
      else
      {
        stream << "<email></email>" << endl;
      }

      stream << "<employer>" << ab.organization() << "</employer>" << endl;

      KABC::Address business = ab.address(KABC::Address::Work  );
      stream << "<businessaddress>" << business.street() << "</businessaddress>" << endl;

      stream << "<department>" << ab.organization() << "</department>" << endl
             << "<position>" << ab.role() << "</position>" << endl
             << "<note>" << ab.note() << "</note>" << endl
            << "</contact>" << endl;
    } // end for
    stream << "</contacts>" << endl;
    // now replace the UIDs for us -> xxx for what??
//    m_helper->replaceIds( "AddressBookSyncEntry",  m_kde2opie ); // to keep the use small

  }  // end if (buffer.open( IO_WriteOnly))
  return array;
}
