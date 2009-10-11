/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "soapmessages.h"

#include <QtXml/QXmlAttributes>
#include <KLocale>
#include <KDebug>

SoapRequest::SoapRequest( QByteArray *array ) :
    QXmlStreamWriter( array )
{
    setAutoFormatting( true );
}

void SoapRequest::setApiToken( const QString& apiToken )
{
    m_apiToken = apiToken;
}

void SoapRequest::setHeaderNamespace( const QString& headerNamespace )
{
    m_headerNamespace = headerNamespace;
}

void SoapRequest::writeRequestStart()
{
    writeStartDocument();
    writeStartElement( QLatin1String("soap12:Envelope") );
    writeNamespace( QLatin1String("http://www.w3.org/2001/XMLSchema-instance"), QLatin1String("xsi") );
    writeNamespace( QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("xsd") );
    writeNamespace( QLatin1String("http://www.w3.org/2003/05/soap-envelope"), QLatin1String("soap12") );
    writeStartElement( QLatin1String("soap12:Header") );
    writeStartElement( QLatin1String("NGAPIToken") );
    writeDefaultNamespace( m_headerNamespace );
    writeTextElement( QLatin1String("Token"), m_apiToken );
    writeTextElement( QLatin1String("OrgCode"), QLatin1String("KDE") );
    writeTextElement( QLatin1String("EnableCompression"), QLatin1String("false") );
    writeEndElement();   // NGAPIToken
    writeEndElement();   // soap12:Header
    writeStartElement( QLatin1String("soap12:Body") );
}

void SoapRequest::writeRequestEnd()
{
    writeEndElement();   // soap12:Body
    writeEndElement();   // soap12:Envelope
    writeEndDocument();
}



SoapMessage::SoapMessage()
    : m_messageType( UnknownMessage ), m_soapErrorString( i18n( "There were no SOAP errors" ) )
{
}

SoapMessage::~SoapMessage()
{
}

void SoapMessage::addData( const QByteArray& data )
{
    m_reader.addData( data );
}

bool SoapMessage::hasError() const
{
    return m_reader.hasError();
}

QString SoapMessage::errorString() const
{
    return m_reader.errorString();
}

SoapMessage::MessageType SoapMessage::messageType() const
{
    return m_messageType;
}

QString SoapMessage::soapErrorString() const
{
    return m_soapErrorString;
}

void SoapMessage::parse()
{
    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Envelope") ) {
                readSoapEnvelope();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain an Envelope token" ) );
                return;
            }
        }
    }
}

void SoapMessage::readUnknownElement()
{
    Q_ASSERT( m_reader.isStartElement() );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() )
            readUnknownElement();
    }
}

void SoapMessage::readSoapEnvelope()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("Envelope") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Body") ) {
                readSoapBody();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a Body element" ) );
                return;
            }
        }
    }
}

void SoapMessage::readSoapBody()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("Body") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Fault") ) {
                m_messageType = Fault;
                readSoapFault();
            }
            else if ( m_reader.name().toString().endsWith( QLatin1String("Response") ) ) {
                m_messageType = Response;
                readResponse();
            }
            else {
                m_reader.raiseError( i18n( "Unsupported SOAP message type: %1", m_reader.name().toString() ) );
                return;
            }
        }
    }
}

void SoapMessage::readSoapFault()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("Fault") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Code") ) {
                // skip Code, Value, SubCode etc
                readUnknownElement();
            }
            else if ( m_reader.name() == QLatin1String("Reason") ) {
                readSoapFaultReason();
            }
            else {
                // skip whatever else we can encounter here
                readUnknownElement();
            }
        }
    }
}

void SoapMessage::readSoapFaultReason()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("Reason") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Text") ) {
                m_soapErrorString = m_reader.readElementText();
            }
            else {
                // skip whatever else we can encounter here
                readUnknownElement();
            }
        }
    }
}

void SoapMessage::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() )
            readUnknownElement();
    }
}



LocationsResponse::LocationsResponse()
    : SoapMessage()
{
}

QList<Location> LocationsResponse::locations() const
{
    return m_locations;
}

void LocationsResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("GetLocationsResponse") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a GetLocationsResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("GetLocationsResult") ) {
                readResult();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a GetLocationsResult element" ) );
                return;
            }
        }
    }
}

void LocationsResponse::readResult()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("GetLocationsResult") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("Location") ) {
                readLocation();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a Location element" ) );
                return;
            }
        }
    }
}

void LocationsResponse::readLocation()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("Location") );

    Location location;

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("id") ) {
                location.id = m_reader.readElementText().toInt();
            }
            else if ( m_reader.name() == QLatin1String("name") ) {
                location.name = m_reader.readElementText();
            }
            else if ( m_reader.name() == QLatin1String("isPublic") ) {
                location.isPublic = m_reader.readElementText().contains( QLatin1String("true"), Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == QLatin1String("isApmlPublic") ) {
                location.isApmlPublic = m_reader.readElementText().contains( QLatin1String("true"), Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == QLatin1String("contentOnline") ) {
                location.contentOnline = m_reader.readElementText().contains( QLatin1String("true"), Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == QLatin1String("autoAddSubs") ) {
                location.autoAddSubs = m_reader.readElementText().contains( QLatin1String("true"), Qt::CaseInsensitive );
            }
            else {
                readUnknownElement();
            }
        }
    }

    m_locations.append( location );
}



SubscriptionsResponse::SubscriptionsResponse()
    : SoapMessage(), m_currentId( 0 )
{
}

QHash<int, ParsedFeed> SubscriptionsResponse::feeds() const
{
    return m_feeds;
}

QHash<QString, QList<int> > SubscriptionsResponse::tags() const
{
    return m_tags;
}

QStringRef SubscriptionsResponse::attributeValue( const QXmlStreamAttributes& attributes, const QString& name )
{
    Q_FOREACH( const QXmlStreamAttribute& attr, attributes ) {
        if ( attr.name().toString().toLower() == name ) {
            return attr.value();
        }
    }

    return QStringRef();
}

void SubscriptionsResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("GetSubscriptionListResponse") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a GetSubscriptionListResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("GetSubscriptionListResult") ) {
                readResult();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a GetSubscriptionListResult element" ) );
                return;
            }
        }
    }
}

void SubscriptionsResponse::readResult()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("GetSubscriptionListResult") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == QLatin1String("opml") ) {
                readOpml();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain an Opml element" ) );
                return;
            }
        }
    }
}

void SubscriptionsResponse::readOpml()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == QLatin1String("opml") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == QLatin1String("head") ) {
                readUnknownElement();
            }
            else if ( m_reader.name().toString().toLower() == QLatin1String("body") ) {
                readBody();
            }
            else {
                readUnknownElement();
            }
        }
    }
}

void SubscriptionsResponse::readBody()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == QLatin1String("body") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == QLatin1String("outline") ) {
                QStringList tags;
                readOutline( tags );
            }
            else {
                readUnknownElement();
            }
        }
    }
}

void SubscriptionsResponse::readOutline( QStringList &tags )
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == QLatin1String("outline") );

    bool isFolder = false;
    const QString xmlUrl = attributeValue( m_reader.attributes(), QLatin1String("xmlurl") ).toString();

    if ( xmlUrl.isEmpty() ) {
        const QStringRef textAttribute = attributeValue( m_reader.attributes(), QLatin1String("text") );
        if ( !textAttribute.isEmpty() ) {
            // this attribute seem to represent a folder
            isFolder = true;
            if ( !tags.contains( textAttribute.toString() ) )
                tags.append( textAttribute.toString() );

            kDebug() << "Tags:" << tags;
        }
        else {
            kDebug() << "Encountered an empty outline";
            const QXmlStreamAttributes attrs;
            Q_FOREACH( const QXmlStreamAttribute& attr, attrs ) {
                kDebug() << "Attribute name:" << attr.name() << ", value:" << attr.value();
            }
        }
    }
    else {
        // this is a feed
        kDebug() << "Feed:" << xmlUrl;
        isFolder = false;
        ParsedFeed feed;
        const QXmlStreamAttributes attrs = m_reader.attributes();
        Q_FOREACH( const QXmlStreamAttribute& attr, attrs ) {
            if ( attr.name().toString().toLower() == QLatin1String("title") )
                feed.title = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("text") && feed.title.isEmpty() )
                feed.title = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("htmlurl") )
                feed.htmlUrl = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("xmlurl") )
                feed.xmlUrl = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("description") )
                feed.description = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("type") )
                feed.type = attr.value().toString();
            else if ( attr.name().toString().toLower() == QLatin1String("category") ) {
                const QStringList categories = attr.value().toString().split( QRegExp( QLatin1String("[,/]") ), QString::SkipEmptyParts );
                Q_FOREACH( const QString& category, categories ) {
                    if ( !tags.contains( category ) )
                        tags.append( category );
                }
            }
            else {
                feed.attributes[ attr.name().toString() ] = attr.value().toString();
            }
        }

        if ( feed.title.isEmpty() )
            feed.title = xmlUrl;

        if ( feed.type.isEmpty() )
            feed.type = QLatin1String("rss");

        // everything is parsed
        m_feeds[ m_currentId ] = feed;
        Q_FOREACH( const QString& tag, tags ) {
            m_tags[ tag ].append( m_currentId );
        }
        ++m_currentId;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == QLatin1String("outline") && isFolder )
                readOutline( tags );
            else
                readUnknownElement();
        }
    }

    // once we are back from recursion remove the added tag
    // from the top of the list
    if ( isFolder )
        tags.removeLast();
}



AddSubscriptionResponse::AddSubscriptionResponse()
    : SoapMessage(), m_subscriptionId( 0 )
{
}

int AddSubscriptionResponse::subscriptionId() const
{
    return m_subscriptionId;
}

void AddSubscriptionResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("AddSubscriptionResponse") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a AddSubscriptionResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("AddSubscriptionResult") ) {
                m_subscriptionId = m_reader.readElementText().toInt();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a AddSubscriptionResult element" ) );
                return;
            }
        }
    }
}



RenameSubscriptionResponse::RenameSubscriptionResponse()
    : SoapMessage()
{
}

void RenameSubscriptionResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("RenameSubscriptionResponse") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a RenameSubscriptionResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            readUnknownElement();
        }
    }
}



DeleteSubscriptionResponse::DeleteSubscriptionResponse()
    : SoapMessage(), m_statusCode( 0 )
{
}

int DeleteSubscriptionResponse::statusCode() const
{
    return m_statusCode;
}

void DeleteSubscriptionResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("DeleteSubscriptionsResponse") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a DeleteSubscriptionsResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("DeleteSubscriptionsResult") ) {
                readResult();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a DeleteSubscriptionsResult element" ) );
                return;
            }
        }
    }
}

void DeleteSubscriptionResponse::readResult()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("DeleteSubscriptionsResult") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("ResultType") ) {
                readResultType();
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a ResultType element" ) );
                return;
            }
        }
    }
}

void DeleteSubscriptionResponse::readResultType()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("ResultType") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("ResourceId") ) {
                kDebug() << "ResourceId:" << m_reader.readElementText();
            }
            else if ( m_reader.name() == QLatin1String("StatusCode") ) {
                m_statusCode = m_reader.readElementText().toInt();
                kDebug() << "StatusCode:" << m_statusCode;
            }
            else {
                m_reader.raiseError( i18n( "The document doesn't contain a ResourceId or StatusCode element" ) );
                return;
            }
        }
    }
}



UpdatePostResponse::UpdatePostResponse()
    : SoapMessage()
{
}

void UpdatePostResponse::readResponse()
{
    Q_ASSERT( m_reader.isStartElement() );

    if ( m_reader.name() != QLatin1String("UpdatePostMetadatav3Response") ) {
        m_reader.raiseError( i18n( "The document doesn't contain a UpdatePostMetadatav3Response element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == QLatin1String("UpdatePostMetadatav3Result") ) {
                readResult();
            }
        }
    }
}

void UpdatePostResponse::readResult()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == QLatin1String("UpdatePostMetadatav3Result") );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            readUnknownElement();
        }
    }
}
