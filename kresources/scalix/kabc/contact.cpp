/*
 *   This file is part of the scalix resource.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qdom.h>

#include <libkdepim/distributionlist.h>
#include <kglobal.h>
#include <kdeversion.h>
#include "contact.h"

using namespace Scalix;

typedef QMap<QString, QString> StringMap;

K_GLOBAL_STATIC( StringMap, s_distListMap )

static QString custom( const QString &name, const KABC::Addressee &addr, const QString &defaultValue = QString() )
{
  const QString value = addr.custom( "Scalix", name );
  if ( value.isEmpty() )
    return defaultValue;
  else
    return value;
}

static void setCustom( const QString &name, const QString &value, KABC::Addressee &addr )
{
  addr.insertCustom( "Scalix", name, value );
}

QString Contact::toXml( const KABC::Addressee &addr )
{
  /**
   * Handle distribution lists.
   */
  if ( KPIM::DistributionList::isDistributionList( addr ) ) {
    if ( s_distListMap )
      return (*s_distListMap)[ addr.uid() ];
    else
      return QString();
  }

  /**
   * Handle normal contacts.
   */
  QString xml;
  xml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  xml += "<contact>\n";

  xml += "<direct_ref>" + addr.uid() + "</direct_ref>\n";
  xml += "<sensitivity>" + custom( "sensitivity", addr, "0" ) + "</sensitivity>\n";

  xml += "<message_class>IPM.Contact</message_class>\n";
  xml += "<is_recurring>" + custom( "is_recurring", addr, "false" ) + "</is_recurring>\n";
  xml += "<reminder_set>" + custom( "reminder_set", addr, "false" ) + "</reminder_set>\n";
  xml += "<send_rich_info>" + custom( "send_rich_info", addr, "false" ) + "</send_rich_info>\n";
  xml += "<subject>" + addr.formattedName() + "</subject>\n";
  xml += "<last_modification_time>" + addr.revision().toString( Qt::ISODate ) + "</last_modification_time>\n";

  xml += "<display_name_prefix>" + addr.prefix() + "</display_name_prefix>\n";
  xml += "<first_name>" + addr.givenName() + "</first_name>\n";
  xml += "<middle_name>" + addr.additionalName() + "</middle_name>\n";
  xml += "<last_name>" + addr.familyName() + "</last_name>\n";
  xml += "<suffix>" + addr.suffix() + "</suffix>\n";
  xml += "<display_name>" + addr.assembledName() + "</display_name>\n";
  xml += "<file_as>" + addr.formattedName() + "</file_as>\n";
  xml += "<nickname>" + addr.nickName() + "</nickname>\n";

  xml += "<web_page_address>" + addr.url().url() + "</web_page_address>\n";
  xml += "<company_name>" + addr.organization() + "</company_name>\n";
  xml += "<job_title>" + addr.title() + "</job_title>\n";

  QStringList emails = addr.emails();
  for ( int i = 0; i < 3; ++i ) {
    QString type, address, comment, display;

    if ( i < emails.count() ) {
      type = "SMTP";
      address = emails[ i ];

      /**
       * If the contact was created by kontact use the email address as
       * display name and the formatted name as comment, otherwise we use
       * the values from the server.
       */
      if ( custom( "comes_from_scalix", addr ) != "true" ) {
        comment = addr.formattedName();
        display = emails[ i ];
      } else {
        comment = custom( QString( "email%1_address_with_comment" ).arg( i + 1 ), addr );
        display = custom( QString( "email%1_display_name" ).arg( i + 1 ), addr );
      }
    }

    xml += QString( "<email%1_address_type>" ).arg( i + 1 ) + type +
           QString( "</email%1_address_type>" ).arg( i + 1 ) + '\n';
    xml += QString( "<email%1_address>" ).arg( i + 1 ) + address +
           QString( "</email%1_address>" ).arg( i + 1 ) + '\n';
    xml += QString( "<email%1_address_with_comment>" ).arg( i + 1 ) + comment +
           QString( "</email%1_address_with_comment>" ).arg( i + 1 ) + '\n';
    xml += QString( "<email%1_display_name>" ).arg( i + 1 ) + display +
           QString( "</email%1_display_name>" ).arg( i + 1 ) + '\n';
  }

  KABC::PhoneNumber phone = addr.phoneNumber( KABC::PhoneNumber::Home );
  xml += "<home_phone_number>" + phone.number() + "</home_phone_number>\n";

  phone = addr.phoneNumber( KABC::PhoneNumber::Work );
  xml += "<work_phone_number>" + phone.number() + "</work_phone_number>\n";

  phone = addr.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
  xml += "<work_fax_number>" + phone.number() + "</work_fax_number>\n";

  phone = addr.phoneNumber( KABC::PhoneNumber::Cell );
  xml += "<mobile_phone_number>" + phone.number() + "</mobile_phone_number>\n";

  const KABC::Address workAddress = addr.address( KABC::Address::Work );
  xml += "<work_address_street>" + workAddress.street() + "</work_address_street>\n";
  xml += "<work_address_zip>" + workAddress.postalCode() + "</work_address_zip>\n";
  xml += "<work_address_city>" + workAddress.locality() + "</work_address_city>\n";
  xml += "<work_address_state>" + workAddress.region() + "</work_address_state>\n";
  xml += "<work_address_country>" + workAddress.country() + "</work_address_country>\n";

  const KABC::Address homeAddress = addr.address( KABC::Address::Home );
  xml += "<home_address_street>" + homeAddress.street() + "</home_address_street>\n";
  xml += "<home_address_zip>" + homeAddress.postalCode() + "</home_address_zip>\n";
  xml += "<home_address_city>" + homeAddress.locality() + "</home_address_city>\n";
  xml += "<home_address_state>" + homeAddress.region() + "</home_address_state>\n";
  xml += "<home_address_country>" + homeAddress.country() + "</home_address_country>\n";

  const KABC::Address otherAddress = addr.address( KABC::Address::Dom );
  xml += "<other_address_street>" + otherAddress.street() + "</other_address_street>\n";
  xml += "<other_address_zip>" + otherAddress.postalCode() + "</other_address_zip>\n";
  xml += "<other_address_city>" + otherAddress.locality() + "</other_address_city>\n";
  xml += "<other_address_state>" + otherAddress.region() + "</other_address_state>\n";
  xml += "<other_address_country>" + otherAddress.country() + "</other_address_country>\n";

  if ( homeAddress.type() & KABC::Address::Pref )
    xml += "<selected_mailing_address>1</selected_mailing_address>\n";
  else if ( workAddress.type() & KABC::Address::Pref )
    xml += "<selected_mailing_address>2</selected_mailing_address>\n";
  else if ( otherAddress.type() & KABC::Address::Pref )
    xml += "<selected_mailing_address>3</selected_mailing_address>\n";

  xml += "<im_address>" + addr.custom( "KADDRESSBOOK", "X-IMAddress" ) + "</im_address>\n";
  xml += "<manager>" + addr.custom( "KADDRESSBOOK", "X-ManagersName" ) + "</manager>\n";
  xml += "<department>" + addr.department() + "</department>\n";
  xml += "<assistant>" + addr.custom( "KADDRESSBOOK", "X-AssistantsName" ) + "</assistant>\n";
  xml += "<profession>" + addr.custom( "KADDRESSBOOK", "X-Profession" ) + "</profession>\n";
  xml += "<office_location>" + addr.custom( "KADDRESSBOOK", "X-Office" ) + "</office_location>\n";
  xml += "<spouse>" + addr.custom( "KADDRESSBOOK", "X-SpousesName" ) + "</spouse>\n";

  xml += "<bday>" + addr.birthday().toString( Qt::ISODate ) + "</bday>\n";
  xml += "<anniversary>" + addr.custom( "KADDRESSBOOK", "X-Anniversary" ) + "</anniversary>\n";

  xml += "<mapi_charset>" + custom( "mapi_charset", addr, "UTF8" ) + "</mapi_charset>";

  xml += "</contact>\n";

  return xml;
}

KABC::Addressee Contact::fromXml( const QString &xml )
{
  QDomDocument document;

  QString errorMsg;
  int errorLine, errorColumn;
  if ( !document.setContent( xml, true, &errorMsg, &errorLine, &errorColumn ) ) {
    qDebug( "Error parsing XML in Scalix::Contact::fromXml: %s (%d,%d)", qPrintable( errorMsg ), errorLine, errorColumn );
    return KABC::Addressee();
  }

  QDomElement contactElement = document.documentElement();
  if ( contactElement.tagName() != "contact" ) {
    if ( contactElement.tagName() == "distlist" ) {
      const QDomNodeList names = contactElement.elementsByTagName( "display_name" );
      const QString listName = ( names.count() == 1 ? names.item( 0 ).toElement().text() : "Scalix Dummy List" );

      /**
       * As we can't provide distribution list functionality we store the entry
       * here and return it on save.
       */
      KPIM::DistributionList list;
      list.setName( listName );

      s_distListMap->insert( list.uid(), xml );

      return list;
    } else {
      qDebug( "Error interpreting XML in Scalix::Contact::fromXml: no 'contact' or 'distlist' tag found" );
      return KABC::Addressee();
    }
  }

  QString emails[ 3 ];
  KABC::Address homeAddress( KABC::Address::Home );
  KABC::Address workAddress( KABC::Address::Work );
  KABC::Address otherAddress( KABC::Address::Dom );

  KABC::Addressee addr;
  setCustom( "comes_from_scalix", "true", addr );

  QDomNode node = contactElement.firstChild();
  while ( !node.isNull() ) {
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "direct_ref" )
        addr.setUid( element.text() );
      else if ( element.tagName() == "sensitivity" )
        setCustom( "sensitivity", element.text(), addr );
      else if ( element.tagName() == "is_recurring" )
        setCustom( "is_recurring", element.text(), addr );
      else if ( element.tagName() == "reminder_set" )
        setCustom( "reminder_set", element.text(), addr );
      else if ( element.tagName() == "send_rich_info" )
        setCustom( "send_rich_info", element.text(), addr );
      else if ( element.tagName() == "last_modification_time" )
        addr.setRevision( QDateTime::fromString( element.text(), Qt::ISODate ) );

      // name
      else if ( element.tagName() == "display_name_prefix" )
        addr.setPrefix( element.text() );
      else if ( element.tagName() == "first_name" )
        addr.setGivenName( element.text() );
      else if ( element.tagName() == "middle_name" )
        addr.setAdditionalName( element.text() );
      else if ( element.tagName() == "last_name" )
        addr.setFamilyName( element.text() );
      else if ( element.tagName() == "suffix" )
        addr.setSuffix( element.text() );
      else if ( element.tagName() == "file_as" )
        addr.setFormattedName( element.text() );
      else if ( element.tagName() == "nickname" )
        addr.setNickName( element.text() );

      // job
      else if ( element.tagName() == "web_page_address" )
        addr.setUrl( element.text() );
      else if ( element.tagName() == "company_name" )
        addr.setOrganization( element.text() );
      else if ( element.tagName() == "job_title" )
        addr.setTitle( element.text() );

      // emails
      else if ( element.tagName().startsWith( "email" ) ) {
        if ( element.tagName() == "email1_address" )
          emails[ 0 ] = element.text();
        else if ( element.tagName() == "email2_address" )
          emails[ 1 ] = element.text();
        else if ( element.tagName() == "email3_address" )
          emails[ 2 ] = element.text();
        else
          setCustom( element.tagName(), element.text(), addr );
      }

      // phone numbers
      else if ( element.tagName() == "home_phone_number" )
        addr.insertPhoneNumber( KABC::PhoneNumber( element.text(), KABC::PhoneNumber::Home ) );
      else if ( element.tagName() == "work_phone_number" )
        addr.insertPhoneNumber( KABC::PhoneNumber( element.text(), KABC::PhoneNumber::Work ) );
      else if ( element.tagName() == "work_fax_number" )
        addr.insertPhoneNumber( KABC::PhoneNumber( element.text(), KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax ) );
      else if ( element.tagName() == "mobile_phone_number" )
        addr.insertPhoneNumber( KABC::PhoneNumber( element.text(), KABC::PhoneNumber::Cell ) );

      // address (work)
      else if ( element.tagName() == "work_address_street" )
        workAddress.setStreet( element.text() );
      else if ( element.tagName() == "work_address_zip" )
        workAddress.setPostalCode( element.text() );
      else if ( element.tagName() == "work_address_city" )
        workAddress.setLocality( element.text() );
      else if ( element.tagName() == "work_address_state" )
        workAddress.setRegion( element.text() );
      else if ( element.tagName() == "work_address_country" )
        workAddress.setCountry( element.text() );

      // address (home)
      else if ( element.tagName() == "home_address_street" )
        homeAddress.setStreet( element.text() );
      else if ( element.tagName() == "home_address_zip" )
        homeAddress.setPostalCode( element.text() );
      else if ( element.tagName() == "home_address_city" )
        homeAddress.setLocality( element.text() );
      else if ( element.tagName() == "home_address_state" )
        homeAddress.setRegion( element.text() );
      else if ( element.tagName() == "home_address_country" )
        homeAddress.setCountry( element.text() );

      // address (other)
      else if ( element.tagName() == "other_address_street" )
        otherAddress.setStreet( element.text() );
      else if ( element.tagName() == "other_address_zip" )
        otherAddress.setPostalCode( element.text() );
      else if ( element.tagName() == "other_address_city" )
        otherAddress.setLocality( element.text() );
      else if ( element.tagName() == "other_address_state" )
        otherAddress.setRegion( element.text() );
      else if ( element.tagName() == "other_address_country" )
        otherAddress.setCountry( element.text() );

      else if ( element.tagName() == "selected_mailing_address" )
        switch ( element.text().toInt() ) {
          case 1:
            homeAddress.setType( homeAddress.type() | KABC::Address::Pref );
            break;
          case 2:
            workAddress.setType( workAddress.type() | KABC::Address::Pref );
            break;
          case 3:
            otherAddress.setType( otherAddress.type() | KABC::Address::Pref );
            break;
          default:
            Q_ASSERT( !"Unknown selected_mailing_address enum" );
            break;
        }

      // misc
      else if ( element.tagName() == "im_address" )
        addr.insertCustom( "KADDRESSBOOK", "X-IMAddress", element.text() );
      else if ( element.tagName() == "manager" )
        addr.insertCustom( "KADDRESSBOOK", "X-ManagersName", element.text() );
      else if ( element.tagName() == "department" )
      {
#if KDE_IS_VERSION(3,5,8)
        addr.setDepartment(element.text());
#else
        addr.insertCustom( "KADDRESSBOOK", "X-Department", element.text() );
#endif
      }
      else if ( element.tagName() == "assistant" )
        addr.insertCustom( "KADDRESSBOOK", "X-AssistantsName", element.text() );
      else if ( element.tagName() == "profession" )
        addr.insertCustom( "KADDRESSBOOK", "X-Profession", element.text() );
      else if ( element.tagName() == "office_location" )
        addr.insertCustom( "KADDRESSBOOK", "X-Office", element.text() );
      else if ( element.tagName() == "spouse" )
        addr.insertCustom( "KADDRESSBOOK", "X-SpousesName", element.text() );

      else if ( element.tagName() == "bday" )
        addr.setBirthday( QDateTime::fromString( element.text(), Qt::ISODate ) );
      else if ( element.tagName() == "anniversary" )
        addr.insertCustom( "KADDRESSBOOK", "X-Anniversary", element.text() );
      else
        setCustom( element.tagName(), element.text(), addr );
    }

    node = node.nextSibling();
  }

  for ( int i = 0; i < 3; ++i )
    if ( !emails[ i ].isEmpty() )
      addr.insertEmail( emails[ i ] );

  if ( !homeAddress.isEmpty() )
    addr.insertAddress( homeAddress );
  if ( !workAddress.isEmpty() )
    addr.insertAddress( workAddress );
  if ( !otherAddress.isEmpty() )
    addr.insertAddress( otherAddress );

  return addr;
}
