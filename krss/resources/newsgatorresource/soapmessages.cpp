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
    writeStartElement( "soap12:Envelope" );
    writeNamespace( "http://www.w3.org/2001/XMLSchema-instance", "xsi" );
    writeNamespace( "http://www.w3.org/2001/XMLSchema", "xsd" );
    writeNamespace( "http://www.w3.org/2003/05/soap-envelope", "soap12" );
    writeStartElement( "soap12:Header" );
    writeStartElement( "NGAPIToken" );
    writeDefaultNamespace( m_headerNamespace );
    writeTextElement( "Token", m_apiToken );
    writeTextElement( "OrgCode", "KDE" );
    writeTextElement( "EnableCompression", "false" );
    writeEndElement();   // NGAPIToken
    writeEndElement();   // soap12:Header
    writeStartElement( "soap12:Body" );
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
            if ( m_reader.name() == "Envelope" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "Envelope" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "Body" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "Body" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "Fault" ) {
                m_messageType = Fault;
                readSoapFault();
            }
            else if ( m_reader.name().toString().endsWith( "Response" ) ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "Fault" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "Code" ) {
                // skip Code, Value, SubCode etc
                readUnknownElement();
            }
            else if ( m_reader.name() == "Reason" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "Reason" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "Text" ) {
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

    if ( m_reader.name() != "GetLocationsResponse" ) {
        m_reader.raiseError( i18n( "The document doesn't contain a GetLocationsResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "GetLocationsResult" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "GetLocationsResult" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "Location" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "Location" );

    Location location;

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "id" ) {
                location.id = m_reader.readElementText().toInt();
            }
            else if ( m_reader.name() == "name" ) {
                location.name = m_reader.readElementText();
            }
            else if ( m_reader.name() == "isPublic" ) {
                location.isPublic = m_reader.readElementText().contains( "true", Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == "isApmlPublic" ) {
                location.isApmlPublic = m_reader.readElementText().contains( "true", Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == "contentOnline" ) {
                location.contentOnline = m_reader.readElementText().contains( "true", Qt::CaseInsensitive );
            }
            else if ( m_reader.name() == "autoAddSubs" ) {
                location.autoAddSubs = m_reader.readElementText().contains( "true", Qt::CaseInsensitive );
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

    if ( m_reader.name() != "GetSubscriptionListResponse" ) {
        m_reader.raiseError( i18n( "The document doesn't contain a GetSubscriptionListResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "GetSubscriptionListResult" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "GetSubscriptionListResult" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == "opml" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == "opml" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == "head" ) {
                readUnknownElement();
            }
            else if ( m_reader.name().toString().toLower() == "body" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == "body" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name().toString().toLower() == "outline" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name().toString().toLower() == "outline" );

    bool isFolder = false;
    const QString xmlUrl = attributeValue( m_reader.attributes(), "xmlurl" ).toString();

    if ( xmlUrl.isEmpty() ) {
        const QStringRef textAttribute = attributeValue( m_reader.attributes(), "text" );
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
            if ( attr.name().toString().toLower() == "title" )
                feed.title = attr.value().toString();
            else if ( attr.name().toString().toLower() == "text" && feed.title.isEmpty() )
                feed.title = attr.value().toString();
            else if ( attr.name().toString().toLower() == "htmlurl" )
                feed.htmlUrl = attr.value().toString();
            else if ( attr.name().toString().toLower() == "xmlurl" )
                feed.xmlUrl = attr.value().toString();
            else if ( attr.name().toString().toLower() == "description" )
                feed.description = attr.value().toString();
            else if ( attr.name().toString().toLower() == "type" )
                feed.type = attr.value().toString();
            else if ( attr.name().toString().toLower() == "category" ) {
                const QStringList categories = attr.value().toString().split( QRegExp( "[,/]" ), QString::SkipEmptyParts );
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
            feed.type = "rss";

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
            if ( m_reader.name().toString().toLower() == "outline" && isFolder )
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

    if ( m_reader.name() != "AddSubscriptionResponse" ) {
        m_reader.raiseError( i18n( "The document doesn't contain a AddSubscriptionResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "AddSubscriptionResult" ) {
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

    if ( m_reader.name() != "RenameSubscriptionResponse" ) {
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

    if ( m_reader.name() != "DeleteSubscriptionsResponse" ) {
        m_reader.raiseError( i18n( "The document doesn't contain a DeleteSubscriptionsResponse element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "DeleteSubscriptionsResult" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "DeleteSubscriptionsResult" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "ResultType" ) {
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
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "ResultType" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "ResourceId" ) {
                kDebug() << "ResourceId:" << m_reader.readElementText();
            }
            else if ( m_reader.name() == "StatusCode" ) {
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

    if ( m_reader.name() != "UpdatePostMetadatav3Response" ) {
        m_reader.raiseError( i18n( "The document doesn't contain a UpdatePostMetadatav3Response element" ) );
        return;
    }

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            if ( m_reader.name() == "UpdatePostMetadatav3Result" ) {
                readResult();
            }
        }
    }
}

void UpdatePostResponse::readResult()
{
    Q_ASSERT( m_reader.isStartElement() && m_reader.name() == "UpdatePostMetadatav3Result" );

    while ( !m_reader.atEnd() ) {
        m_reader.readNext();

        if ( m_reader.isEndElement() )
            break;

        if ( m_reader.isStartElement() ) {
            readUnknownElement();
        }
    }
}
