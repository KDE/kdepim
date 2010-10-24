/*
    This file is part of KDE-PIM.

    Copyright (c) 2007 - 2010 Tobias Koenig <tokoe@kde.org>

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

const char* VcfEndAnalyzer::name() const
{
  return "VcfEndAnalyzer";
}

bool VcfEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  return headersize >= 11 && !strncmp( "BEGIN:VCARD", header, 11 );
}

// cannot use Address::formattedAddress because it requires KComponentData
static QString formatAddress( const KABC::Address &address )
{
  QStringList parts;
  if ( !address.country().isEmpty() )
    parts += address.country();

  if ( !address.region().isEmpty() )
    parts += address.region();

  if ( !address.postalCode().isEmpty() )
    parts += address.postalCode();

  if ( !address.locality().isEmpty() )
    parts += address.locality();

  if ( !address.street().isEmpty() )
    parts += address.street();

  if ( !address.postOfficeBox().isEmpty() )
    parts += address.postOfficeBox();

  if ( !address.extended().isEmpty() )
    parts += address.extended();

  return parts.join( QLatin1String( ", " ) );
}

/**
 * It's easier to use KABC::VCardConverter to extract the single fields from the vCard
 * than doing it manually.
 */
STRIGI_ENDANALYZER_RETVAL VcfEndAnalyzer::analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream )
{
  using namespace KABC;
  const char* data;

  int read = stream->read( data, stream->size(), stream->size() );
  if ( read < 0 )
    return Strigi::Error;

  const QByteArray text( data, read );

  VCardConverter converter;
  const Addressee contact = converter.parseVCard( text );
  if ( contact.isEmpty() )
    return Strigi::Error;

  Q_FOREACH ( const QString& email, contact.emails() )
    index.addValue( m_factory->emailField, email.toUtf8().data() );

  index.addValue( m_factory->givenNameField, contact.givenName().toUtf8().data() );
  index.addValue( m_factory->familyNameField, contact.familyName().toUtf8().data() );

  if ( contact.url().isValid() )
    index.addValue( m_factory->homepageField, contact.url().url().toUtf8().data() );

  if ( !contact.note().isEmpty() )
    index.addValue( m_factory->commentField, contact.note().toUtf8().data() );

  index.addValue( m_factory->typeField, "http://freedesktop.org/standards/xesam/1.0/core#Person" );

  Q_FOREACH ( const PhoneNumber &number, contact.phoneNumbers() ) {
    switch ( number.type() ) {
      case PhoneNumber::Cell: index.addValue( m_factory->cellPhoneField, number.number().toUtf8().data() ); break;
      case PhoneNumber::Home: index.addValue( m_factory->homePhoneField, number.number().toUtf8().data() ); break;
      case PhoneNumber::Work: index.addValue( m_factory->workPhoneField, number.number().toUtf8().data() ); break;
      case PhoneNumber::Fax: index.addValue( m_factory->faxPhoneField, number.number().toUtf8().data() ); break;
      default: index.addValue( m_factory->otherPhoneField, number.number().toUtf8().data() ); 
    }
  }

  Q_FOREACH ( const Address &address, contact.addresses() ) {
    switch ( address.type() ) {
      case Address::Home: index.addValue( m_factory->homeAddressField, formatAddress( address ).toUtf8().data() ); break;
      case Address::Work: index.addValue( m_factory->workAddressField, formatAddress( address ).toUtf8().data() ); break;
      default: index.addValue( m_factory->otherAddressField, formatAddress( address ).toUtf8().data() ); break;
    }
  }

  if ( !contact.photo().isEmpty() && !contact.photo().isIntern())
    index.addValue( m_factory->photoField, contact.photo().url().toUtf8().data() );

  if ( !contact.suffix().isEmpty() )
   index.addValue( m_factory->suffixField, contact.suffix().toUtf8().data() );

  if ( !contact.prefix().isEmpty() )
   index.addValue( m_factory->prefixField, contact.prefix().toUtf8().data() );

  return Strigi::Ok;
}

const char* VcfEndAnalyzerFactory::name() const
{
  return "VcfEndAnalyzer";
}

Strigi::StreamEndAnalyzer* VcfEndAnalyzerFactory::newInstance() const
{
  return new VcfEndAnalyzer( this );
}

void VcfEndAnalyzerFactory::registerFields( Strigi::FieldRegister &reg )
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

std::list<Strigi::StreamEndAnalyzerFactory*> VcfFactoryFactory::streamEndAnalyzerFactories() const
{
  std::list<Strigi::StreamEndAnalyzerFactory*> list;
  list.push_back( new VcfEndAnalyzerFactory );

  return list;
}
