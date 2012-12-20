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

#include <config-strigi.h>
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

  const QUrl url( QString::fromLatin1( index.path().data(), index.path().size() ) );
  if ( url.scheme() == QLatin1String( "akonadi" ) && url.hasQueryItem( "collection" ) )
    index.addValue( m_factory->isPartOfField, url.queryItemValue( "collection" ).toUtf8().data() );

  index.addValue( m_factory->typeField, "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#PersonContact" );

  if ( !contact.prefix().isEmpty() )
   index.addValue( m_factory->prefixField, contact.prefix().toUtf8().data() );

  if ( !contact.givenName().isEmpty() )
    index.addValue( m_factory->givenNameField, contact.givenName().toUtf8().data() );

  if ( !contact.additionalName().isEmpty() )
    index.addValue( m_factory->additionalNameField, contact.additionalName().toUtf8().data() );

  if ( !contact.familyName().isEmpty() )
    index.addValue( m_factory->familyNameField, contact.familyName().toUtf8().data() );

  if ( !contact.suffix().isEmpty() )
    index.addValue( m_factory->suffixField, contact.suffix().toUtf8().data() );

  if ( !contact.formattedName().isEmpty() )
    index.addValue( m_factory->fullnameField, contact.formattedName().toUtf8().data() );

  if ( !contact.nickName().isEmpty() )
    index.addValue( m_factory->nicknameField, contact.nickName().toUtf8().data() );

  Q_FOREACH ( const QString& email, contact.emails() ) {
    index.addValue( m_factory->emailField, email.toUtf8().data() );
  }

  if ( contact.url().isValid() )
    index.addValue( m_factory->websiteUrlField, contact.url().url().toUtf8().data() );

  if ( !contact.note().isEmpty() )
    index.addValue( m_factory->noteField, contact.note().toUtf8().data() );

  Q_FOREACH ( const PhoneNumber &number, contact.phoneNumbers() ) {
    index.addValue( m_factory->phoneNumberField, number.number().toUtf8().data() );
  }

  Q_FOREACH ( const Address &address, contact.addresses() ) {
    index.addValue( m_factory->addressCountryField, address.country().toUtf8().data() );
    index.addValue( m_factory->addressLocalityField, address.locality().toUtf8().data() );
    index.addValue( m_factory->addressPostOfficeBoxField, address.postOfficeBox().toUtf8().data() );
    index.addValue( m_factory->addressPostalCodeField, address.postalCode().toUtf8().data() );
    index.addValue( m_factory->addressRegionField, address.region().toUtf8().data() );
    index.addValue( m_factory->addressStreetField, address.street().toUtf8().data() );
  }

  foreach ( const QString &category, contact.categories() )
    index.addValue( m_factory->categoriesField, category.toUtf8().data() );

  if ( !contact.photo().isEmpty() && !contact.photo().isIntern())
    index.addValue( m_factory->photoField, contact.photo().url().toUtf8().data() );

  index.addValue( m_factory->uidField, contact.uid().toUtf8().data() );

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
  prefixField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameHonorificPrefix" );
  givenNameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameGiven" );
  additionalNameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameAdditional" );
  familyNameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameFamily" );
  suffixField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameHonorificSuffix" );
  fullnameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname" );
  nicknameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nickname" );

  emailField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress" );
  websiteUrlField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#websiteUrl" );
  noteField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#note" );

  phoneNumberField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#phoneNumber" );

  addressCountryField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country" );
  addressLocalityField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#locality" );
  addressPostOfficeBoxField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#pobox" );
  addressPostalCodeField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#postalcode" );
  addressRegionField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#region" );
  addressStreetField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#streetAddress" );

  uidField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contactUID" );
  photoField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#photo" );
  isPartOfField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf" );
  categoriesField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#categories" );

  typeField = reg.typeField;
}

std::list<Strigi::StreamEndAnalyzerFactory*> VcfFactoryFactory::streamEndAnalyzerFactories() const
{
  std::list<Strigi::StreamEndAnalyzerFactory*> list;
  list.push_back( new VcfEndAnalyzerFactory );

  return list;
}
