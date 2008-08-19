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

#include <kabc/vcardconverter.h>

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

VcfEndAnalyzer::VcfEndAnalyzer( const VcfEndAnalyzerFactory *factory )
  : m_factory( factory )
{
}

bool VcfEndAnalyzer::checkHeader( const char* header, int32_t headersize ) const
{
  const char* magic = "BEGIN:VCARD";
  int32_t magicLength = strlen( magic );

  return headersize >= magicLength && !strncmp( magic, header, magicLength );
}

/**
 * It's easier to use KABC::VCardConverter to extract the single fields from the vCard
 * than doing it manually.
 */
signed char VcfEndAnalyzer::analyze( Strigi::AnalysisResult& idx, Strigi::InputStream* in )
{
  const char* data;
  if ( in->read( data, 1, in->size() ) < 0 )
    return Strigi::Error;

  const QByteArray text( data, in->size() );

  KABC::VCardConverter converter;
  KABC::Addressee addr = converter.parseVCard( text );

  if ( addr.isEmpty() )
    return Strigi::Error;

  idx.addValue( m_factory->field( Name ), addr.assembledName().toUtf8().data() );
  idx.addValue( m_factory->field( Email ), addr.preferredEmail().toUtf8().data() );
  if (addr.phoneNumbers().size()) {
    idx.addValue( m_factory->field( Telephone ),
      addr.phoneNumbers().first().number().toUtf8().data() );
  }

  return Strigi::Ok;
}

const Strigi::RegisteredField* VcfEndAnalyzerFactory::field( VcfEndAnalyzer::Field field ) const
{
  switch ( field ) {
    default:
    case VcfEndAnalyzer::Name:
      return nameField;
      break;
    case VcfEndAnalyzer::Email:
      return emailField;
      break;
    case VcfEndAnalyzer::Telephone:
      return telephoneField;
      break;
  }
}

void VcfEndAnalyzerFactory::registerFields( Strigi::FieldRegister& reg )
{
  nameField = reg.registerField( "vcf.name", Strigi::FieldRegister::stringType, 1, 0 );
  emailField = reg.registerField( "vcf.email", Strigi::FieldRegister::stringType, 1, 0 );
  telephoneField = reg.registerField( "vcf.telephone", Strigi::FieldRegister::stringType, 1, 0 );
}

