/* This file is part of the KDE libraries
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    Based on the davjob:
    Copyright (C) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    XML-RPC specific parts taken from the xmlrpciface:
    Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>
                                 Tobias Koenig <tokoe@kde.org>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "xmlrpcjob.h"

#include <qvariant.h>
#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <kio/http.h>
#include <kmdcodec.h>
#include <kio/davjob.h>


#define KIO_ARGS QByteArray packedArgs; \
                 QDataStream stream( packedArgs, IO_WriteOnly ); stream

using namespace KIO;

namespace KIO {
  class XMLRPCResult
  {
      friend class XmlrpcJob;
    public:
      XMLRPCResult() {}
      bool success() const { return m_success; }
      int errorCode() const { return m_errorCode; }
      QString errorString() const { return m_errorString; }
      QValueList<QVariant> data() const { return m_data; }
    private:
      bool m_success;
      int m_errorCode;
      QString m_errorString;
      QValueList<QVariant> m_data;
  };
}

class XmlrpcJob::XmlrpcJobPrivate
{
public:
//   QByteArray savedStaticData;
};


XmlrpcJob::XmlrpcJob( const KURL& url, const QString& method,
                      const QValueList<QVariant> &params, bool showProgressInfo)
  : TransferJob( url, KIO::CMD_SPECIAL, QByteArray(), QByteArray(),
                 showProgressInfo )
{
  d = new XmlrpcJobPrivate;
  // We couldn't set the args when calling the parent constructor,
  // so do it now.
  QDataStream stream( m_packedArgs, IO_WriteOnly );
   stream << (int)1 << url;
kdDebug()<<"XMLrpcJob::url="<<url.url()<<endl;
kdDebug()<<"XmlrpcJob::XmlrpcJob, method="<<method<<endl;
  // Same for static data
  if ( ! method.isEmpty() ) {
kdDebug()<<"XmlrpcJob::XmlrpcJob, method not empty."<<endl;

    QString call = markupCall( method, params );
    staticData = call.utf8();
    staticData.truncate( staticData.size() - 1 );
    kdDebug() << "Message: " << call << endl;
//     d->savedStaticData = staticData.copy();
  }
  addMetaData( "UserAgent", "KDE XML-RPC TransferJob" );
  addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  addMetaData( "ConnectTimeout", "50" );
}

XmlrpcJob::~XmlrpcJob()
{
  delete d;
  d = 0;
}

QString XmlrpcJob::markupCall( const QString &cmd,
                               const QValueList<QVariant> &args )
{
kdDebug()<<"XmlrpcJob::markupCall, cmd="<<cmd<<endl;
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





void XmlrpcJob::slotData( const QByteArray& data )
{
kdDebug()<<"XmlrpcJob::slotData()"<<endl;
  if ( m_redirectionURL.isEmpty() || !m_redirectionURL.isValid() || m_error )
    m_str_response.append( QString( data ) );
}

void XmlrpcJob::slotFinished()
{
kdDebug() << "XmlrpcJob::slotFinished()" << endl;
kdDebug() << m_str_response << endl;

  // TODO: Redirection with XML-RPC??
/*  if (! m_redirectionURL.isEmpty() && m_redirectionURL.isValid() ) {
    QDataStream istream( m_packedArgs, IO_ReadOnly );
    int s_cmd, s_method;
    KURL s_url;
    istream >> s_cmd;
    istream >> s_url;
    istream >> s_method;
    // PROPFIND
    if ( (s_cmd == 7) && (s_method == (int)KIO::HTTP_POST) ) {
      m_packedArgs.truncate(0);
      QDataStream stream( m_packedArgs, IO_WriteOnly );
      stream << (int)7 << m_redirectionURL << (int)KIO::HTTP_POST;
    }
  } else */

  kdDebug() << "\033[35;40mResult: " << m_str_response << "\033[0;0m" << endl;
  QDomDocument doc;
  QString errMsg;
  int errLine, errCol;
  if ( doc.setContent( m_str_response, false, &errMsg, &errLine, &errCol ) ) {
    if ( isMessageResponse( doc ) ) {
      m_response = parseMessageResponse( doc ).data();
      m_responseType = XMLRPCMessageResponse;
    } else if ( isFaultResponse( doc ) ) {
      // TODO: Set the error of the job
      m_response.clear();
      m_response << QVariant( parseFaultResponse( doc ).errorString() );
      m_responseType = XMLRPCFaultResponse;
    } else {
      // TODO: Set the error of the job
      m_response.clear();
      m_response << QVariant( i18n( "Unknown type of XML markup received. "
                    "Markup: \n %1" ).arg( m_str_response ) );
      m_responseType = XMLRPCUnknownResponse;
    }

  } else {
    // TODO: if we can't parse the XML response, set the correct error message!
//     emit fault( -1, i18n( "Received invalid XML markup: %1 at %2:%3" )
//                         .arg( errMsg ).arg( errLine ).arg( errCol ), m_id );
  }

  TransferJob::slotFinished();
// TODO: Redirect:   if( d ) staticData = d->savedStaticData.copy();
// Need to send XMLRPC request to this host too
}





bool XmlrpcJob::isMessageResponse( const QDomDocument &doc )
{
  return doc.documentElement().firstChild().toElement()
            .tagName().lower() == "params";
}

XMLRPCResult XmlrpcJob::parseMessageResponse( const QDomDocument &doc )
{
  XMLRPCResult response;
  response.m_success = true;

  QDomNode paramNode = doc.documentElement().firstChild().firstChild();
  while ( !paramNode.isNull() ) {
    response.m_data << demarshal( paramNode.firstChild().toElement() );
    paramNode = paramNode.nextSibling();
  }

  return response;
}





bool XmlrpcJob::isFaultResponse( const QDomDocument &doc )
{
  return doc.documentElement().firstChild().toElement()
            .tagName().lower() == "fault";
}

XMLRPCResult XmlrpcJob::parseFaultResponse( const QDomDocument &doc )
{
  XMLRPCResult response;
  response.m_success = false;

  QDomNode errorNode = doc.documentElement().firstChild().firstChild();
  const QVariant errorVariant = demarshal( errorNode.toElement() );
  response.m_errorCode = errorVariant.toMap() [ "faultCode" ].toInt();
  response.m_errorString = errorVariant.toMap() [ "faultString" ].toString();

  return response;
}





QString XmlrpcJob::marshal( const QVariant &arg )
{
  switch ( arg.type() )
  {
    case QVariant::String:
    case QVariant::CString:
      return "<value><string>" + arg.toString() + "</string></value>\r\n";
    case QVariant::Int:
      return "<value><int>" + QString::number( arg.toInt() ) +
             "</int></value>\r\n";
    case QVariant::Double:
      return "<value><double>" + QString::number( arg.toDouble() ) +
             "</double></value>\r\n";
    case QVariant::Bool:
      {
        QString markup = "<value><boolean>";
        markup += arg.toBool() ? "1" : "0";
        markup += "</boolean></value>\r\n";
        return markup;
      }
    case QVariant::ByteArray:
      return "<value><base64>" + KCodecs::base64Encode( arg.toByteArray() ) +
             "</base64></value>\r\n";
    case QVariant::DateTime:
      return "<value><datetime.iso8601>" +
             arg.toDateTime().toString( Qt::ISODate ) +
             "</datetime.iso8601></value>\r\n";
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
      kdWarning() << "Failed to marshal unknown variant type: "
                  << arg.type() << endl;
  };
  return QString::null;
}

QVariant XmlrpcJob::demarshal( const QDomElement &elem )
{
  Q_ASSERT( elem.tagName().lower() == "value" );

  if ( !elem.hasChildNodes() ) {
    // it doesn't have child nodes, so no explicit type name was given,
    // i.e. <value>here comes the value</value> instead of
    //      <value><string>here comes the value</string></value>
    // Assume <string> in that case:
    // Actually, the element will still have a child node, so this will not help here.
    // The dirty hack is at the end of this method.
kdDebug()<<"XmlrpcJob::demarshal: No child nodes, assume type=string. Text: "<<elem.text()<<endl;
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
      // ISO 8601 Format 2004-11-20T..., else QDateTime::fromString won't work:
      text = text.insert( 6, '-' );
      text = text.insert( 4, '-' );
    }
    return QVariant( QDateTime::fromString( text, Qt::ISODate ) );

  } else if ( typeName == "array" ) {

    QValueList<QVariant> values;
    QDomNode valueNode = typeElement.firstChild().firstChild();
    while ( !valueNode.isNull() ) {
      values << demarshal( valueNode.toElement() );
      valueNode = valueNode.nextSibling();
    }
    return QVariant( values );

  } else if ( typeName == "struct" ) {

    QMap<QString, QVariant> map;
    QDomNode memberNode = typeElement.firstChild();
    while ( !memberNode.isNull() ) {
      const QString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
      const QVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
      map[ key ] = data;
      memberNode = memberNode.nextSibling();
    }
    return QVariant( map );

  } else {

    kdWarning() << "Cannot demarshal unknown type " << typeName << ", text= " << typeElement.text() << endl;
    // FIXME: This is just a workaround, for the issue mentioned at the beginning of this method.
    return QVariant( elem.text() );
  }

  return QVariant();
}





/* Convenience methods */

XmlrpcJob* KIO::xmlrpcCall( const KURL& url, const QString &method, const QValueList<QVariant> &params, bool showProgressInfo )
{
  if ( url.isEmpty() ) {
    kdWarning() << "Cannot execute call to " << method << ": empty server URL" << endl;
    return 0;
  }
  XmlrpcJob *job = new XmlrpcJob( url, method, params, showProgressInfo );
//   job->addMetaData( "xmlrpcDepth", depth );

  return job;
}

XmlrpcJob* KIO::xmlrpcCall( const KURL& url, const QString &method,
		                        const QVariant &arg, bool showProgressInfo )
{
  QValueList<QVariant> args;
  args << arg;
  return KIO::xmlrpcCall( url, method, args, showProgressInfo );
}

XmlrpcJob* KIO::xmlrpcCall( const KURL& url, const QString &method,
		                        const QStringList &arg, bool showProgressInfo )
{
  QValueList<QVariant> args;
  QStringList::ConstIterator it = arg.begin();
  QStringList::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << QVariant( *it );
  return KIO::xmlrpcCall( url, method, args, showProgressInfo );
}

template <typename T>
XmlrpcJob* KIO::xmlrpcCall( const KURL& url, const QString &method,
		                        const QValueList<T>&arg, bool showProgressInfo )
{
  QValueList<QVariant> args;

  typename QValueList<T>::ConstIterator it = arg.begin();
  typename QValueList<T>::ConstIterator end = arg.end();
  for ( ; it != end; ++it )
    args << QVariant( *it );
  return KIO::xmlrpcCall( url, method, args, showProgressInfo );
}

#include "xmlrpcjob.moc"
