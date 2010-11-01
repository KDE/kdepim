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

#include "mailendanalyzer.h"

#include "messageanalyzer.h"

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/mailinputstream.h>
#include <strigi/streamendanalyzer.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <QtCore/QEventLoop>

static int s_fakeArgumentCount = 1;
static char* s_fakeArgumentVector[2] = { "mailendanalyzer", NULL };

MailEndAnalyzer::MailEndAnalyzer( const MailEndAnalyzerFactory *factory )
  : m_factory( factory ),
    m_index( 0 )
{
  if ( !QCoreApplication::instance() ) {
    KCmdLineArgs::init( s_fakeArgumentCount, s_fakeArgumentVector,
                        "mailendanalyzer", "mailendanalyzer", ki18n( "mailendanalyzer" ), "0.1" );
    new KApplication( false );
  }
}

MailEndAnalyzer::~MailEndAnalyzer()
{
}

const char* MailEndAnalyzer::name() const
{
  return "MailEndAnalyzer";
}

bool MailEndAnalyzer::checkHeader( const char* header, qint32 headersize ) const
{
  return Strigi::MailInputStream::checkHeader( header, headersize );
}

STRIGI_ENDANALYZER_RETVAL MailEndAnalyzer::analyze( Strigi::AnalysisResult &index, Strigi::InputStream *stream )
{
  const char* data;

  int read = stream->read( data, stream->size(), stream->size() );
  if ( read < 0 )
    return Strigi::Error;

  m_index = &index;

  const QByteArray text( data, read );

  KMime::Message::Ptr message( new KMime::Message() );
  message->setContent( text );
  message->parse();

  MessageAnalyzer analyzer( message, this );

  QEventLoop loop;
  QObject::connect( &analyzer, SIGNAL( finished() ), &loop, SLOT( quit() ), Qt::QueuedConnection );
  analyzer.analyze();
  loop.exec();

  m_index = 0;

  return Strigi::Ok;
}

void MailEndAnalyzer::addValue( Field field, const QString &value )
{
  switch ( field ) {
    case SubjectField: m_index->addValue( m_factory->subjectField, value.toUtf8().data() ); break;
    case FromField: m_index->addValue( m_factory->fromField, value.toUtf8().data() ); break;
    case SenderField: m_index->addValue( m_factory->senderField, value.toUtf8().data() ); break;
    case ToField: m_index->addValue( m_factory->toField, value.toUtf8().data() ); break;
    case CcField: m_index->addValue( m_factory->ccField, value.toUtf8().data() ); break;
    case BccField: m_index->addValue( m_factory->bccField, value.toUtf8().data() ); break;
    case MessageIdField: m_index->addValue( m_factory->messageIdField, value.toUtf8().data() ); break;
    case ReferencesField: m_index->addValue( m_factory->referencesField, value.toUtf8().data() ); break;
    case InReplyToField: m_index->addValue( m_factory->inReplyToField, value.toUtf8().data() ); break;
    case ContentTypeField: m_index->addValue( m_factory->contentTypeField, value.toUtf8().data() ); break;
    case MessageContentField: m_index->addValue( m_factory->messageContentField, value.toUtf8().data() ); break;
    case TypeField: m_index->addValue( m_factory->typeField, value.toUtf8().data() ); break;
  }
}

const char* MailEndAnalyzerFactory::name() const
{
  return "MailEndAnalyzer";
}

Strigi::StreamEndAnalyzer* MailEndAnalyzerFactory::newInstance() const
{
  return new MailEndAnalyzer( this );
}

void MailEndAnalyzerFactory::registerFields( Strigi::FieldRegister &reg )
{
  subjectField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject" );
  fromField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#from" );
  senderField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender" );
  toField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#to" );
  ccField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#cc" );
  bccField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#bcc" );
  messageIdField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#messageId" );
  referencesField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#references" );
  inReplyToField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#inReplyTo" );
  messageContentField = reg.registerField( "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent" );
  contentTypeField = reg.mimetypeField;
  typeField = reg.typeField;
}

MailFactoryFactory::MailFactoryFactory()
  : componentData( "IcsFactoryFactory" )
{
}

std::list<Strigi::StreamEndAnalyzerFactory*> MailFactoryFactory::streamEndAnalyzerFactories() const
{
  std::list<Strigi::StreamEndAnalyzerFactory*> list;
  list.push_back( new MailEndAnalyzerFactory );

  return list;
}

STRIGI_ANALYZER_FACTORY( MailFactoryFactory )

