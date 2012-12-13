/*
    This file is part of KDE-PIM.

    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
       a KDAB Group company, info@kdab.net,
       author Tobias Koenig <tokoe@kde.org>

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
#include "ctgendanalyzer.h"

#include <kabc/contactgroup.h>
#include <kabc/contactgrouptool.h>

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamendanalyzer.h>

// FIXME: Remove when we require strigi > 0.7.2
#ifdef HAVE_KMPSEARCHER_H
#include <strigi/kmpsearcher.h>
#else
#include "kmpsearcher.h"
#endif

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QUrl>

using namespace Strigi;

CtgEndAnalyzer::CtgEndAnalyzer( const CtgEndAnalyzerFactory *factory )
  : m_factory( factory )
{
}

const char* CtgEndAnalyzer::name() const
{
  return "CtgEndAnalyzer";
}

bool CtgEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  // initialize a searcher with the string we are looking for
  KmpSearcher ctgSearcher( "<contactGroup" );

  return headersize >= 54 && ctgSearcher.search ( header, headersize );
}

STRIGI_ENDANALYZER_RETVAL CtgEndAnalyzer::analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream )
{
  using namespace KABC;
  const char* data;

  int read = stream->read( data, stream->size(), stream->size() );
  if ( read < 0 )
    return Strigi::Error;

  QByteArray text( data, read );
  QBuffer buffer( &text );
  buffer.open( QIODevice::ReadOnly );

  ContactGroup group;
  const bool result = ContactGroupTool::convertFromXml( &buffer, group );
  if ( !result )
    return Strigi::Error;

  const QUrl url( QString::fromLatin1( index.path().data(), index.path().size() ) );
  if ( url.scheme() == QLatin1String( "akonadi" ) && url.hasQueryItem( "collection" ) )
    index.addValue( m_factory->isPartOfField, url.queryItemValue( "collection" ).toUtf8().data() );

  index.addValue( m_factory->typeField, "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#ContactGroup" );

  index.addValue( m_factory->nameField, group.name().toUtf8().data() );

  return Strigi::Ok;
}

const char* CtgEndAnalyzerFactory::name() const
{
  return "CtgEndAnalyzer";
}

Strigi::StreamEndAnalyzer* CtgEndAnalyzerFactory::newInstance() const
{
  return new CtgEndAnalyzer( this );
}

void CtgEndAnalyzerFactory::registerFields( Strigi::FieldRegister &reg )
{
  nameField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contactGroupName" );
  isPartOfField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf" );

  typeField = reg.typeField;
}

std::list<Strigi::StreamEndAnalyzerFactory*> CtgFactoryFactory::streamEndAnalyzerFactories() const
{
  std::list<Strigi::StreamEndAnalyzerFactory*> list;
  list.push_back( new CtgEndAnalyzerFactory );

  return list;
}
