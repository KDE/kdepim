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
#include <kapp.h>

#include <idhelper.h>

#include "libpv/pvdataentry.h"

#include "addressbook.h"

using namespace KSync;
using namespace PVHelper;

/**
  * Converts a QDomNode to an AddressBookSyncee*.
  * @param node The node (part of an XML document) to be converted
  * @return KSync::AddressBookSyncee* The converted addressbook
  */
AddressBookSyncee* AddressBook::toAddressBookSyncee(QDomNode& n)
{
  kdDebug(5205) << "Begin::AddressBook::toAddressBookSyncee" << endl;
  AddressBookSyncee *syncee = new AddressBookSyncee();
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QString id;

  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    if( e.tagName() == QString::fromLatin1("contact"))
    {
      // convert XML contact to addressee
      KABC::Addressee adr;

      helper.addId(e.attribute("category"), e.attribute("uid"), adr.uid());
      adr.insertCategory(e.attribute("category"));
      // Get subentries
      QDomNode n = e.firstChild();
      QDomElement el = n.toElement();

      while(!n.isNull()) // Get all sub entries of element contact
      {
        QDomElement el = n.toElement();
        // Get the entities and put them in the addressee
        if ((el.tagName() == QString::fromLatin1("name")) ||
             (el.tagName() == QString::fromLatin1("field1")))
        {
          adr.setFamilyName(el.text());
        }
        else if ((el.tagName() == QString::fromLatin1("homenumber")) ||
                  (el.tagName() == QString::fromLatin1("field2")))
        {
          KABC::PhoneNumber homePhoneNum( el.text(),
              KABC::PhoneNumber::Home );
          adr.insertPhoneNumber(homePhoneNum );
        }
        else if ((el.tagName() == QString::fromLatin1("businessnumber")) ||
                  (el.tagName() == QString::fromLatin1("field3")))
        {
          KABC::PhoneNumber businessPhoneNum(el.text(),
              KABC::PhoneNumber::Work);
          adr.insertPhoneNumber( businessPhoneNum );
        }
        else if ((el.tagName() == QString::fromLatin1("faxnumber")) ||
                  (el.tagName() == QString::fromLatin1("field4")))
        {
          KABC::PhoneNumber homeFax (el.text(),
              KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
          adr.insertPhoneNumber(homeFax );
        }
        else if ((el.tagName() == QString::fromLatin1("businessfax")) ||
                  (el.tagName() == QString::fromLatin1("field5")))
        {
          KABC::PhoneNumber businessFaxNum ( el.text(),
               KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax);
          adr.insertPhoneNumber( businessFaxNum );
        }
        else if ((el.tagName() == QString::fromLatin1("mobile")) ||
                  (el.tagName() == QString::fromLatin1("field6")))
        {
          KABC::PhoneNumber businessMobile ( el.text(),
              KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
          adr.insertPhoneNumber( businessMobile);
        }
        else if ((el.tagName() == QString::fromLatin1("address")) ||
                  (el.tagName() == QString::fromLatin1("field7")))
        {
          KABC::Address home ( KABC::Address::Home );
          home = strToAddress(el.text(), KABC::Address::Home);
          adr.insertAddress(home);
        }
        else if ((el.tagName() == QString::fromLatin1("email")) ||
                  (el.tagName() == QString::fromLatin1("field8")))
        {
          adr.insertEmail(el.text(), true); // preferred
        }
        else if ((el.tagName() == QString::fromLatin1("employer")) ||
                  (el.tagName() == QString::fromLatin1("field9")))
        {
          adr.setOrganization(el.text());
        }
        else if ((el.tagName() == QString::fromLatin1("businessaddress")) ||
                  (el.tagName() == QString::fromLatin1("field10")))
        {
          KABC::Address business( KABC::Address::Work );
          business = strToAddress(el.text(), KABC::Address::Work);
          adr.insertAddress( business );
        }
        else if ((el.tagName() == QString::fromLatin1("department")) ||
                  (el.tagName() == QString::fromLatin1("field11")))
        {
          adr.insertCustom("PV", "Department", el.text());
        }
        else if ((el.tagName() == QString::fromLatin1("position")) ||
                  (el.tagName() == QString::fromLatin1("field12")))
        {
          adr.setRole(el.text());
        }
        else if ((el.tagName() == QString::fromLatin1("note")) ||
                  (el.tagName() == QString::fromLatin1("field13")))
        {
          adr.setNote(el.text());
        }
        n = n.nextSibling();  // Go to the next entity
      }  // end of while
      adr.setRevision( QDateTime::currentDateTime() );
      KSync::AddressBookSyncEntry* entry = new KSync::AddressBookSyncEntry( adr );
      // add entry to syncee
      syncee->addEntry(entry);
      // Check state and set it in syncee
      switch (e.attribute("state").toUInt())
      {
        case CasioPV::PVDataEntry::UNDEFINED:
          entry->setState(KSync::SyncEntry::Undefined);
          break;
        case CasioPV::PVDataEntry::ADDED:
          entry->setState(KSync::SyncEntry::Added);
          break;
        case CasioPV::PVDataEntry::MODIFIED:
          entry->setState(KSync::SyncEntry::Modified);
          break;
        case CasioPV::PVDataEntry::REMOVED:
          entry->setState(KSync::SyncEntry::Removed);
          break;
        default:
          break;
      }
    }  // end of if contact
    else
    {
      kdDebug(5205) << "PVHelper::XML2Syncee -> contact not found" << endl;
      return 0l; // xxx bessere fehlerbehandlung!
    }
    n = n.nextSibling(); // jump to next element
  }  // end of while
  kdDebug(5205) << "End::AddressBook::toAddressBookSyncee" << endl;
  return syncee;
}

/**
  * Converts an AddressBookSyncee* to a QString which represents a
  * DOM node.
  * @param syncee The syncee to be converted
  * @return QString The converted addressbook
  */
QString AddressBook::toXML(AddressBookSyncee* syncee)
{
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QStringList categories;

  QString str;

  str.append("<contacts>\n");
  // for all entries
  KABC::Addressee ab;
  KSync::AddressBookSyncEntry *entry;

  for (entry = syncee->firstEntry(); entry != 0l; entry = syncee->nextEntry())
  {
    QString state;
    QString id;
    // Get addresse from entry
    ab = entry->addressee();

    categories = ab.categories(); // xxx only one category supported yet!

    // Check if id is new
    id =  helper.konnectorId(categories[0],  ab.uid());
    if (id.isEmpty())
    {
      // New entry -> set state = added
      state = "added";
    }
    else
    {
      state = "undefined";
    }

    // Convert to XML string
    str.append("<contact uid='" + id + "' category='" + categories[0] + "'");
    str.append(" state='" + state + "'>\n");

    if ((categories[0] == "Contact Business") || (categories[0] == "Contact Private"))
    {
      str.append("<name>" + ab.familyName() + "</name>\n");

      KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
      str.append("<homenumber>" + homePhoneNum.number() + "</homenumber>\n");

      KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
      str.append("<businessnumber>" + businessPhoneNum.number() + "</businessnumber>\n");

      KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
      str.append("<faxnumber>" + homeFax.number() + "</faxnumber>\n");

      KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
      str.append("<businessfax>" + businessFaxNum.number() + "</businessfax>\n");

      KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
      str.append("<mobile>" + businessMobile.number() + "</mobile>\n");

      KABC::Address home = ab.address( KABC::Address::Home );
      str.append("<address>" + addressToStr(home) + "</address>\n");

      QStringList list = ab.emails();
      if ( list.count() > 0 )
      {
        QString email = list[0];
        str.append("<email>" + email + "</email>\n");
      }
      else
      {
        str.append("<email></email>\n");
      }
      str.append("<employer>" + ab.organization() + "</employer>\n");

      KABC::Address business = ab.address(KABC::Address::Work);
      str.append("<businessaddress>" + addressToStr(business) + "</businessaddress>\n");

      if (!ab.custom("PV", "Department").isEmpty())
      {
        str.append("<department>" + ab.custom("PV", "Department") + "</department>\n");
      }
      else
      {
        str.append("<department></department>\n");
      }
      str.append("<position>" + ab.role() + "</position>\n");
      str.append("<note>" + ab.note() + "</note>\n");
      str.append("</contact>\n");
    } // if
    else
    {
      // Untitled contacts
      str.append("<field1>" + ab.familyName() + "</field1>\n");
      KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
      str.append("<field2>" + homePhoneNum.number() + "</field2>\n");

      KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
      str.append("<field3>" + businessPhoneNum.number() + "</field3>\n");

      KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
      str.append("<field4>" + homeFax.number() + "</field4>\n");

      KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
      str.append("<field5>" + businessFaxNum.number() + "</field5>\n");

      KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
      str.append("<field6>" + businessMobile.number() + "</field6>\n");

      KABC::Address home = ab.address( KABC::Address::Home );
      str.append("<field7>" + addressToStr(home) + "</field7>\n");

      QStringList list = ab.emails();
      if ( list.count() > 0 )
      {
        QString email = list[0];
        str.append("<field8>" + email + "</field8>\n");
      }
      else
      {
        str.append("<field8></field8>\n");
      }

      str.append("<field9>" + ab.organization() + "</field9>\n");

      KABC::Address business = ab.address(KABC::Address::Work  );
      str.append("<field10>" + addressToStr(business) + "</field10>\n");

      if (!ab.custom("PV", "Department").isEmpty())
      {
        str.append("<field11>" + ab.custom("PV", "Department") + "</field11>\n");
      }
      else
      {
        str.append("<field11></field11>\n");
      }
      str.append("<field12>" + ab.role() + "</field12>\n");
      str.append("<field13>" + ab.note() + "</field13>\n");
      str.append("</contact>\n");
    } // end else
  } // end for
  str.append("</contacts>\n");
  return str;
}

/**
  * Converts a QString representing the address to a KABC::Address
  * @param strAddr The QString address to be converted
  * @param type The type of the address (e.g. home, work)
  * @return KABC::Address The converted address
  */
KABC::Address AddressBook::strToAddress(const QString& strAddr, KABC::Address::Type type)
{
  KABC::Address addr(type);

  int posline = 0, pos = 0;
  posline = strAddr.find("\n");

  if (posline)
  {
    // found street
    addr.setStreet(strAddr.left(posline));
    // searching for postal code and city
    pos = strAddr.find(" ", posline);
    if (pos)
    {
      // found postal code and city
      addr.setPostalCode(strAddr.mid(posline + 1, pos - posline - 1));
      addr.setLocality(strAddr.right(strAddr.length() - pos - 1));
    }
  }
  else
  {
    // set the whole address in the street field
    addr.setStreet(strAddr);
  }
  return addr;
}

/**
  * Converts an KABC::Address to a QString
  * @param KABC::Address The address to be converted
  * @return QSting The converted address
  */
QString AddressBook::addressToStr(const KABC::Address& addr)
{
  QString strAddr;

  if (!addr.street().isEmpty())
  {
    strAddr.append(addr.street() + "\n");
  }
  if (!addr.postalCode().isEmpty())
  {
    strAddr.append(addr.postalCode());
  }
  if (!addr.locality().isEmpty())
  {
    // Add a space separator between postal code and locality
    if (!addr.postalCode().isEmpty())
    {
      strAddr.append(" ");
    }
    strAddr.append(addr.locality());
  }
  return strAddr;
}
