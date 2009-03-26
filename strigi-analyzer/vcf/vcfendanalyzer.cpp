/*
    This file is part of KDE-PIM.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "vcfendanalyzer.h"

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

VcfEndAnalyzer::VcfEndAnalyzer( const VcfEndAnalyzerFactory *factory )
  : m_factory( factory )
{
}

bool VcfEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  return headersize >= 11 && !strncmp( "BEGIN:VCARD", header, 11 );
}

// cannot use Address::formattedAddress because it requires KComponentData
QString VcfEndAnalyzer::formatAddress(const KABC::Address& a) const
{
  QStringList parts;
  if (!a.country().isEmpty()) parts+=a.country();
  if (!a.region().isEmpty()) parts+=a.region();
  if (!a.postalCode().isEmpty()) parts+=a.postalCode();
  if (!a.locality().isEmpty()) parts+=a.locality();
  if (!a.street().isEmpty()) parts+=a.street();
  if (!a.postOfficeBox().isEmpty()) parts+=a.postOfficeBox();
  if (!a.extended().isEmpty()) parts+=a.extended();
  return parts.join(", ");
}

/**
 * It's easier to use KABC::VCardConverter to extract the single fields from the vCard
 * than doing it manually.
 */
STRIGI_ENDANALYZER_RETVAL VcfEndAnalyzer::analyze( Strigi::AnalysisResult& idx, Strigi::InputStream* in )
{
  using namespace KABC;
  const char* data;

  int read=in->read( data, in->size(), in->size() );
  if ( read < 0 )
    return Strigi::Error;
  const QByteArray text( data, read );

  VCardConverter converter;
  Addressee addr = converter.parseVCard( text );
  if ( addr.isEmpty() )
    return Strigi::Error;

  Q_FOREACH (const QString& email, addr.emails() )
    idx.addValue( m_factory->emailField, email.toUtf8().data() );
  idx.addValue( m_factory->givenNameField, addr.givenName().toUtf8().data() );
  idx.addValue( m_factory->familyNameField, addr.familyName().toUtf8().data() );

  if (addr.url().isValid()) idx.addValue( m_factory->homepageField, addr.url().url().toUtf8().data() );
  if (!addr.note().isEmpty()) idx.addValue( m_factory->commentField, addr.note().toUtf8().data() );
  idx.addValue( m_factory->typeField, "http://freedesktop.org/standards/xesam/1.0/core#Person" );

  Q_FOREACH (const PhoneNumber& pn, addr.phoneNumbers() )
      switch (pn.type()) {
        case PhoneNumber::Cell: idx.addValue( m_factory->cellPhoneField, pn.number().toUtf8().data() ); break;
        case PhoneNumber::Home: idx.addValue( m_factory->homePhoneField, pn.number().toUtf8().data() ); break;
        case PhoneNumber::Work: idx.addValue( m_factory->workPhoneField, pn.number().toUtf8().data() ); break;
        case PhoneNumber::Fax: idx.addValue( m_factory->faxPhoneField, pn.number().toUtf8().data() ); break;
        default: idx.addValue( m_factory->otherPhoneField, pn.number().toUtf8().data() ); 
    }

  Q_FOREACH (const Address& a, addr.addresses() )
      switch (a.type()) {
        case Address::Home: idx.addValue( m_factory->homeAddressField, formatAddress(a).toUtf8().data() ); break;
        case Address::Work: idx.addValue( m_factory->workAddressField, formatAddress(a).toUtf8().data() ); break;
        default: idx.addValue( m_factory->otherAddressField, formatAddress(a).toUtf8().data() ); break;
    }

  if (!addr.photo().isEmpty() && !addr.photo().isIntern())
    idx.addValue( m_factory->photoField, addr.photo().url().toUtf8().data() );

  if (!addr.suffix().isEmpty())
   idx.addValue( m_factory->suffixField, addr.suffix().toUtf8().data() );
  if (!addr.prefix().isEmpty())
   idx.addValue( m_factory->prefixField, addr.prefix().toUtf8().data() );
    
  return Strigi::Ok;
}

void VcfEndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg )
{
  givenNameField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#givenName" );
  familyNameField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#familyName" );

  emailField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#emailAddress" );
  homepageField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#homepageContactURL" );
  commentField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#contentComment" );

  cellPhoneField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#cellPhoneNumber" );
  homePhoneField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#homePhoneNumber" );
  workPhoneField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#workPhoneNumber" );
  faxPhoneField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#faxPhoneNumber" );
  otherPhoneField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#phoneNumber" );

  homeAddressField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#homePostalAddress" );
  workAddressField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#workPostalAddress" );
  otherAddressField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#postalAddress" );

  prefixField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#honorificPrefix" );
  suffixField = reg.registerField( "http://freedesktop.org/standards/xesam/1.0/core#honorificSuffix" );
   
  typeField = reg.typeField;
}

