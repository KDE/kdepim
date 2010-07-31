/**************************************************************************
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <tqfile.h>

#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmdcodec.h>

#include "debugdialog.h"
#include "xmlrpciface.h"

using namespace KXMLRPC;

namespace KXMLRPC
{
  class Result
  {
      friend class Query;
    public:
      Result()
      { }

      bool success() const
      {
        return m_success;
      }
      int errorCode() const
      {
        return m_errorCode;
      }
      TQString errorString() const
      {
        return m_errorString;
      }
      TQValueList<TQVariant> data() const
      {
        return m_data;
      }

    private:
      bool m_success;
      int m_errorCode;
      TQString m_errorString;
      TQValueList<TQVariant> m_data;
  };
}

Query *Query::create( const TQVariant &id, TQObject *parent, const char *name )
{
  return new Query( id, parent, name );
}

void Query::call( const TQString &server, const TQString &method,
                  const TQValueList<TQVariant> &args, const TQString &userAgent )
{
  const TQString xmlMarkup = markupCall( method, args );
  DebugDialog::addMessage( xmlMarkup, DebugDialog::Output );

  TQByteArray postData;
  TQDataStream stream( postData, IO_WriteOnly );
  stream.writeRawBytes( xmlMarkup.utf8(), xmlMarkup.utf8().length() );

  KIO::TransferJob *job = KIO::http_post( KURL( server ), postData, false );
  if ( !job ) {
    kdWarning() << "Unable to create KIO job for " << server << endl;
    return;
  }
  job->addMetaData( "UserAgent", userAgent );
  job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );

  connect( job, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
           this, TQT_SLOT( slotData( KIO::Job *, const TQByteArray & ) ) );
  connect( job, TQT_SIGNAL( result( KIO::Job * ) ),
           this, TQT_SLOT( slotResult( KIO::Job * ) ) );

  m_pendingJobs.append( job );
}

void Query::slotData( KIO::Job *, const TQByteArray &data )
{
  unsigned int oldSize = m_buffer.size();
  m_buffer.resize( oldSize + data.size() );
  memcpy( m_buffer.data() + oldSize, data.data(), data.size() );
}

void Query::slotResult( KIO::Job *job )
{
  m_pendingJobs.remove( job );

  if ( job->error() != 0 )
  {
    emit fault( job->error(), job->errorString(), m_id );
    emit finished( this );
    return ;
  }

  TQString data = TQString::fromUtf8( m_buffer.data(), m_buffer.size() );
  DebugDialog::addMessage( data, DebugDialog::Input );

  TQDomDocument doc;
  TQString errMsg;
  int errLine, errCol;
  if ( !doc.setContent( data, false, &errMsg, &errLine, &errCol  ) )
  {
    emit fault( -1, i18n( "Received invalid XML markup: %1 at %2:%3" )
                        .arg( errMsg ).arg( errLine ).arg( errCol ), m_id );
    emit finished( this );
    return ;
  }

  m_buffer.truncate( 0 );

  if ( isMessageResponse( doc ) )
    emit message( parseMessageResponse( doc ).data(), m_id );
  else if ( isFaultResponse( doc ) )
  {
    emit fault( parseFaultResponse( doc ).errorCode(), parseFaultResponse( doc ).errorString(), m_id );
  }
  else
  {
    emit fault( 1, i18n( "Unknown type of XML markup received" ), m_id );
  }

  emit finished( this );
}

bool Query::isMessageResponse( const TQDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().lower() == "params";
}

Result Query::parseMessageResponse( const TQDomDocument &doc ) const
{
  Result response;
  response.m_success = true;

  TQDomNode paramNode = doc.documentElement().firstChild().firstChild();
  while ( !paramNode.isNull() )
  {
    response.m_data << demarshal( paramNode.firstChild().toElement() );
    paramNode = paramNode.nextSibling();
  }

  return response;
}

bool Query::isFaultResponse( const TQDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().lower() == "fault";
}

Result Query::parseFaultResponse( const TQDomDocument &doc ) const
{
  Result response;
  response.m_success = false;

  TQDomNode errorNode = doc.documentElement().firstChild().firstChild();
  const TQVariant errorVariant = demarshal( errorNode.toElement() );
  response.m_errorCode = errorVariant.toMap() [ "faultCode" ].toInt();
  response.m_errorString = errorVariant.toMap() [ "faultString" ].toString();

  return response;
}

TQString Query::markupCall( const TQString &cmd,
                           const TQValueList<TQVariant> &args ) const
{
  TQString markup = "<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n";

  markup += "<methodName>" + cmd + "</methodName>\r\n";

  if ( !args.isEmpty() )
  {
    markup += "<params>\r\n";
    TQValueList<TQVariant>::ConstIterator it = args.begin();
    TQValueList<TQVariant>::ConstIterator end = args.end();
    for ( ; it != end; ++it )
      markup += "<param>\r\n" + marshal( *it ) + "</param>\r\n";
    markup += "</params>\r\n";
  }

  markup += "</methodCall>\r\n";

  return markup;
}

TQString Query::marshal( const TQVariant &arg ) const
{
  switch ( arg.type() )
  {
      case TQVariant::String:
      case TQVariant::CString:
        {
        TQString result = arg.toString();
        result = result.replace( "&", "&amp;" );
        result = result.replace( "\"", "&quot;" );
        result = result.replace( "<", "&lt;" );
        result = result.replace( ">", "&gt;" );
        return "<value><string>" + result + "</string></value>\r\n";
        }
      case TQVariant::Int:
      return "<value><int>" + TQString::number( arg.toInt() ) + "</int></value>\r\n";
      case TQVariant::Double:
      return "<value><double>" + TQString::number( arg.toDouble() ) + "</double></value>\r\n";
      case TQVariant::Bool:
      {
        TQString markup = "<value><boolean>";
        markup += arg.toBool() ? "1" : "0";
        markup += "</boolean></value>\r\n";
        return markup;
      }
      case TQVariant::ByteArray:
      return "<value><base64>" + KCodecs::base64Encode( arg.toByteArray() ) + "</base64></value>\r\n";
      case TQVariant::DateTime:
      return "<value><datetime.iso8601>" + arg.toDateTime().toString( Qt::ISODate ) + "</datetime.iso8601></value>\r\n";
      case TQVariant::List:
      {
        TQString markup = "<value><array><data>\r\n";
        const TQValueList<TQVariant> args = arg.toList();
        TQValueList<TQVariant>::ConstIterator it = args.begin();
        TQValueList<TQVariant>::ConstIterator end = args.end();
        for ( ; it != end; ++it )
          markup += marshal( *it );
        markup += "</data></array></value>\r\n";
        return markup;
      }
      case TQVariant::Map:
      {
        TQString markup = "<value><struct>\r\n";
        TQMap<TQString, TQVariant> map = arg.toMap();
        TQMap<TQString, TQVariant>::ConstIterator it = map.begin();
        TQMap<TQString, TQVariant>::ConstIterator end = map.end();
        for ( ; it != end; ++it )
        {
          markup += "<member>\r\n";
          markup += "<name>" + it.key() + "</name>\r\n";
          markup += marshal( it.data() );
          markup += "</member>\r\n";
        }
        markup += "</struct></value>\r\n";
        return markup;
      }
      default:
      kdWarning() << "Failed to marshal unknown variant type: " << arg.type() << endl;
  };
  return TQString::null;
}

TQVariant Query::demarshal( const TQDomElement &elem ) const
{
  Q_ASSERT( elem.tagName().lower() == "value" );

  const TQDomElement typeElement = elem.firstChild().toElement();
  const TQString typeName = typeElement.tagName().lower();

  if ( typeName == "string" )
    return TQVariant( typeElement.text() );
  else if ( typeName == "i4" || typeName == "int" )
    return TQVariant( typeElement.text().toInt() );
  else if ( typeName == "double" )
    return TQVariant( typeElement.text().toDouble() );
  else if ( typeName == "boolean" )
  {
    if ( typeElement.text().lower() == "true" || typeElement.text() == "1" )
      return TQVariant( true );
    else
      return TQVariant( false );
  }
  else if ( typeName == "base64" )
    return TQVariant( KCodecs::base64Decode( typeElement.text().latin1() ) );
  else if ( typeName == "datetime" || typeName == "datetime.iso8601" )
    return TQVariant( TQDateTime::fromString( typeElement.text(), Qt::ISODate ) );
  else if ( typeName == "array" )
  {
    TQValueList<TQVariant> values;
    TQDomNode valueNode = typeElement.firstChild().firstChild();
    while ( !valueNode.isNull() )
    {
      values << demarshal( valueNode.toElement() );
      valueNode = valueNode.nextSibling();
    }
    return TQVariant( values );
  }
  else if ( typeName == "struct" )
  {
    TQMap<TQString, TQVariant> map;
    TQDomNode memberNode = typeElement.firstChild();
    while ( !memberNode.isNull() )
    {
      const TQString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
      const TQVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
      map[ key ] = data;
      memberNode = memberNode.nextSibling();
    }
    return TQVariant( map );
  }
  else
    kdWarning() << "Cannot demarshal unknown type " << typeName << endl;

  return TQVariant();
}

Query::Query( const TQVariant &id, TQObject *parent, const char *name )
  : TQObject( parent, name ), m_id( id )
{}

Query::~Query()
{
  TQValueList<KIO::Job*>::Iterator it;
  for ( it = m_pendingJobs.begin(); it != m_pendingJobs.end(); ++it )
    (*it)->kill();
}

Server::Server( const KURL &url, TQObject *parent, const char *name )
    : TQObject( parent, name )
{
  if ( url.isValid() )
    m_url = url;

  m_userAgent = "KDE XMLRPC resources";

  DebugDialog::init();
}

Server::~Server()
{
  TQValueList<Query*>::Iterator it;
  for ( it = mPendingQueries.begin(); it !=mPendingQueries.end(); ++it )
    (*it)->deleteLater();

  mPendingQueries.clear();
}

void Server::queryFinished( Query *query )
{
  mPendingQueries.remove( query );
  query->deleteLater();
}

void Server::setUrl( const KURL &url )
{
  m_url = url.isValid() ? url : KURL();
}

void Server::call( const TQString &method, const TQValueList<TQVariant> &args,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot, const TQVariant &id )
{
  if ( m_url.isEmpty() )
    kdWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;

  Query *query = Query::create( id, this );
  connect( query, TQT_SIGNAL( message( const TQValueList<TQVariant> &, const TQVariant& ) ), msgObj, messageSlot );
  connect( query, TQT_SIGNAL( fault( int, const TQString&, const TQVariant& ) ), faultObj, faultSlot );
  connect( query, TQT_SIGNAL( finished( Query* ) ), this, TQT_SLOT( queryFinished( Query* ) ) );
  mPendingQueries.append( query );

  query->call( m_url.url(), method, args, m_userAgent );
}

void Server::call( const TQString &method, const TQVariant &arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << arg ;
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, int arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, bool arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, double arg ,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, const TQString &arg ,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, const TQCString &arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, const TQByteArray &arg ,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

void Server::call( const TQString &method, const TQDateTime &arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  args << TQVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const TQString &method, const TQStringList &arg,
                   TQObject* msgObj, const char* messageSlot,
                   TQObject* faultObj, const char* faultSlot,
                   const TQVariant &id )
{
  TQValueList<TQVariant> args;
  TQStringList::ConstIterator it = arg.begin();
  TQStringList::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << TQVariant( *it );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

#include "xmlrpciface.moc"
