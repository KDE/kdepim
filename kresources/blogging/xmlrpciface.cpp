/**************************************************************************
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <qfile.h>

#include <kdebug.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <qregexp.h>

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
      QString errorString() const
      {
        return m_errorString;
      }
      QValueList<QVariant> data() const
      {
        return m_data;
      }

    private:
      bool m_success;
      int m_errorCode;
      QString m_errorString;
      QValueList<QVariant> m_data;
  };
};

Query *Query::create( const QVariant &id, QObject *parent, const char *name )
{
  return new Query( id, parent, name );
}

void Query::call( const QString &server, const QString &method,
                  const QValueList<QVariant> &args, const QString &userAgent )
{
  const QString xmlMarkup = markupCall( method, args );
  kdDebug() << "Message: " << xmlMarkup << endl;
  QByteArray postData;
  QDataStream stream( postData, IO_WriteOnly );
  stream.writeRawBytes( xmlMarkup.utf8(), xmlMarkup.utf8().length() );

  KIO::TransferJob *job = KIO::http_post( KURL( server ), postData, false );
  job->addMetaData( "UserAgent", userAgent );
  job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );

  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           this, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotResult( KIO::Job * ) ) );

  m_pendingJobs.append( job );
}

void Query::slotData( KIO::Job *, const QByteArray &data )
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

  QString data = QString::fromUtf8( m_buffer.data(), m_buffer.size() );
  qDebug( "\033[35;40mResult: %s\033[0;0m", data.latin1() );

  QDomDocument doc;
  QString errMsg;
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

bool Query::isMessageResponse( const QDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().lower() == "params";
}

Result Query::parseMessageResponse( const QDomDocument &doc ) const
{
  Result response;
  response.m_success = true;

  QDomNode paramNode = doc.documentElement().firstChild().firstChild();
  while ( !paramNode.isNull() )
  {
    response.m_data << demarshal( paramNode.firstChild().toElement() );
    paramNode = paramNode.nextSibling();
  }

  return response;
}

bool Query::isFaultResponse( const QDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().lower() == "fault";
}

Result Query::parseFaultResponse( const QDomDocument &doc ) const
{
  Result response;
  response.m_success = false;

  QDomNode errorNode = doc.documentElement().firstChild().firstChild();
  const QVariant errorVariant = demarshal( errorNode.toElement() );
  response.m_errorCode = errorVariant.toMap() [ "faultCode" ].toInt();
  response.m_errorString = errorVariant.toMap() [ "faultString" ].toString();

  return response;
}

QString Query::markupCall( const QString &cmd,
                           const QValueList<QVariant> &args ) const
{
  QString markup = "<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n";

  markup += "<methodName>" + cmd + "</methodName>\r\n";

  if ( !args.isEmpty() )
  {
    markup += "<params>\r\n";
    QValueList<QVariant>::ConstIterator it = args.begin();
    QValueList<QVariant>::ConstIterator end = args.end();
    for ( ; it != end; ++it )
      markup += "<param>\r\n" + marshal( *it ) + "</param>\r\n";
    markup += "</params>\r\n";
  }

  markup += "</methodCall>\r\n";

  return markup;
}

QString Query::marshal( const QVariant &arg ) const
{
  switch ( arg.type() )
  {
      case QVariant::String:
      case QVariant::CString:
      return "<value><string>" + arg.toString() + "</string></value>\r\n";
      case QVariant::Int:
      return "<value><int>" + QString::number( arg.toInt() ) + "</int></value>\r\n";
      case QVariant::Double:
      return "<value><double>" + QString::number( arg.toDouble() ) + "</double></value>\r\n";
      case QVariant::Bool:
      {
        QString markup = "<value><boolean>";
        markup += arg.toBool() ? "1" : "0";
        markup += "</boolean></value>\r\n";
        return markup;
      }
      case QVariant::ByteArray:
      return "<value><base64>" + KCodecs::base64Encode( arg.toByteArray() ) + "</base64></value>\r\n";
      case QVariant::DateTime:
      return "<value><datetime.iso8601>" + arg.toDateTime().toString( Qt::ISODate ) + "</datetime.iso8601></value>\r\n";
      case QVariant::List:
      {
        QString markup = "<value><array><data>\r\n";
        const QValueList<QVariant> args = arg.toList();
        QValueList<QVariant>::ConstIterator it = args.begin();
        QValueList<QVariant>::ConstIterator end = args.end();
        for ( ; it != end; ++it )
          markup += marshal( *it );
        markup += "</data></array></value>\r\n";
        return markup;
      }
      case QVariant::Map:
      {
        QString markup = "<value><struct>\r\n";
        QMap<QString, QVariant> map = arg.toMap();
        QMap<QString, QVariant>::ConstIterator it = map.begin();
        QMap<QString, QVariant>::ConstIterator end = map.end();
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
  return QString::null;
}

QVariant Query::demarshal( const QDomElement &elem ) const
{
  Q_ASSERT( elem.tagName().lower() == "value" );

        if ( !elem.hasChildNodes() ) {
          // it doesn't have child nodes, so no explicit type name was given, 
          // i.e. <value>here comes the value</value> instead of
          //      <value><string>here comes the value</string></value>
          // Assume <string> in that case:
          // Actually, the element will still have a child node, so this will not help here. 
          // The dirty hack is at the end of this method. 
kdDebug()<<"KXMLRPC::Query::demarshal: No child nodes, assume type=string. Text: "<<elem.text()<<endl;
          return QVariant( elem.text() );
        }

kdDebug()<<"Demarshalling element \"" << elem.text() <<"\"" << endl;

  const QDomElement typeElement = elem.firstChild().toElement();
  const QString typeName = typeElement.tagName().lower();

  if ( typeName == "string" )
    return QVariant( typeElement.text() );
  else if ( typeName == "i4" || typeName == "int" )
    return QVariant( typeElement.text().toInt() );
  else if ( typeName == "double" )
    return QVariant( typeElement.text().toDouble() );
  else if ( typeName == "boolean" )
  {
    if ( typeElement.text().lower() == "true" || typeElement.text() == "1" )
      return QVariant( true );
    else
      return QVariant( false );
  }
  else if ( typeName == "base64" )
    return QVariant( KCodecs::base64Decode( typeElement.text().latin1() ) );
  else if ( typeName == "datetime" || typeName == "datetime.iso8601" ) {
    QString text( typeElement.text() );
    if ( text.find( QRegExp("^[0-9]{8,8}T") ) >= 0 ) {
      // It's in the format 20041120T...., so adjust it to correct 
      // ISO 8601 Format 2004-11-20T...:
      text = text.insert( 6, '-' );
      text = text.insert( 4, '-' );
    }
    return QVariant( QDateTime::fromString( text, Qt::ISODate ) );
  } else if ( typeName == "array" )
  {
    QValueList<QVariant> values;
    QDomNode valueNode = typeElement.firstChild().firstChild();
    while ( !valueNode.isNull() )
    {
      values << demarshal( valueNode.toElement() );
      valueNode = valueNode.nextSibling();
    }
    return QVariant( values );
  }
  else if ( typeName == "struct" )
  {
    QMap<QString, QVariant> map;
    QDomNode memberNode = typeElement.firstChild();
    while ( !memberNode.isNull() )
    {
      const QString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
      const QVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
      map[ key ] = data;
      memberNode = memberNode.nextSibling();
    }
    return QVariant( map );
  }
  else {
    kdWarning() << "Cannot demarshal unknown type " << typeName << ", text= " << typeElement.text() << endl;
    // FIXME: This is just a workaround, for the issue mentioned at the beginning of this method. 
    return QVariant( elem.text() );
  }

  return QVariant();
}

Query::Query( const QVariant &id, QObject *parent, const char *name )
  : QObject( parent, name ), m_id( id )
{}

Query::~Query()
{
  QValueList<KIO::Job*>::Iterator it;
  for ( it = m_pendingJobs.begin(); it != m_pendingJobs.end(); ++it )
    (*it)->kill();
}

Server::Server( const KURL &url, QObject *parent, const char *name )
    : QObject( parent, name )
{
  if ( url.isValid() )
    m_url = url;

  m_userAgent = "KDE XMLRPC resources";
}

Server::~Server()
{
  QValueList<Query*>::Iterator it;
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

void Server::call( const QString &method, const QValueList<QVariant> &args,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot, const QVariant &id )
{
  if ( m_url.isEmpty() )
  {
    kdWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;
    return ;
  }

  Query *query = Query::create( id, this );
  connect( query, SIGNAL( message( const QValueList<QVariant> &, const QVariant& ) ), msgObj, messageSlot );
  connect( query, SIGNAL( fault( int, const QString&, const QVariant& ) ), faultObj, faultSlot );
  connect( query, SIGNAL( finished( Query* ) ), this, SLOT( queryFinished( Query* ) ) );
  mPendingQueries.append( query );

  query->call( m_url.url(), method, args, m_userAgent );
}

void Server::call( const QString &method, const QVariant &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << arg ;
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, int arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, bool arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, double arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QString &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QCString &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QByteArray &arg ,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, faultObj, faultSlot, msgObj, messageSlot, id );
}

void Server::call( const QString &method, const QDateTime &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  args << QVariant( arg );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

void Server::call( const QString &method, const QStringList &arg,
                   QObject* msgObj, const char* messageSlot,
                   QObject* faultObj, const char* faultSlot,
                   const QVariant &id )
{
  QValueList<QVariant> args;
  QStringList::ConstIterator it = arg.begin();
  QStringList::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << QVariant( *it );
  call( method, args, msgObj, messageSlot, faultObj, faultSlot, id );
}

#include "xmlrpciface.moc"
