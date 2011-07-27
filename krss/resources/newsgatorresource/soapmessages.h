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

#ifndef KRSS_NEWSGATORBACKEND_SOAPMESSAGES_H
#define KRSS_NEWSGATORBACKEND_SOAPMESSAGES_H

#include "location.h"

#include <QtXml/QXmlStreamWriter>
#include <QtXml/QXmlStreamReader>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>

class QXmlStreamAttributes;

class SoapRequest : public QXmlStreamWriter
{
public:
    explicit SoapRequest( QByteArray *array );

    void setApiToken( const QString& apiToken );
    void setHeaderNamespace( const QString& headerNamespace );
    void writeRequestStart();
    void writeRequestEnd();

protected:
    QString m_apiToken;
    QString m_headerNamespace;
    Q_DISABLE_COPY( SoapRequest )
};

class SoapMessage
{
public:
    enum MessageType {
        UnknownMessage,
        Fault,
        Response
    };

    explicit SoapMessage();
    virtual ~SoapMessage();

    void addData( const QByteArray& data );

    bool hasError() const;
    QString errorString() const;
    MessageType messageType() const;
    QString soapErrorString() const;

    void parse();

protected:
    void readUnknownElement();
    void readSoapEnvelope();
    void readSoapBody();
    void readSoapFault();
    void readSoapFaultReason();
    virtual void readResponse();

protected:
    QXmlStreamReader m_reader;
    MessageType m_messageType;
    QString m_soapErrorString;
    Q_DISABLE_COPY( SoapMessage )
};

class LocationsResponse : public SoapMessage
{
public:
    explicit LocationsResponse();

    QList<Location> locations() const;

private:
    void readResponse();
    void readResult();
    void readLocation();

private:
    QList<Location> m_locations;
    Q_DISABLE_COPY( LocationsResponse )
};

struct ParsedFeed {
    QString title;
    QString xmlUrl;
    QString htmlUrl;
    QString description;
    QString type;
    QHash<QString, QString> attributes;
};

class SubscriptionsResponse : public SoapMessage
{
public:
    explicit SubscriptionsResponse();

    QHash<int, ParsedFeed> feeds() const;
    QHash<QString, QList<int> > tags() const;

private:
    QStringRef attributeValue( const QXmlStreamAttributes& attributes, const QString& name );
    void readResponse();
    void readResult();
    void readOpml();
    void readBody();
    void readOutline( QStringList &tags );

private:
    int m_currentId;
    QHash<int, ParsedFeed> m_feeds;
    QHash<QString, QList<int> > m_tags;
    Q_DISABLE_COPY( SubscriptionsResponse )
};

class AddSubscriptionResponse : public SoapMessage
{
public:
    explicit AddSubscriptionResponse();

    int subscriptionId() const;

private:
    void readResponse();

private:
    int m_subscriptionId;
    Q_DISABLE_COPY( AddSubscriptionResponse )
};

class RenameSubscriptionResponse : public SoapMessage
{
public:
    explicit RenameSubscriptionResponse();

private:
    void readResponse();
    Q_DISABLE_COPY( RenameSubscriptionResponse )
};

class DeleteSubscriptionResponse : public SoapMessage
{
public:
    explicit DeleteSubscriptionResponse();

    int statusCode() const;

private:
    void readResponse();
    void readResult();
    void readResultType();

private:
    int m_statusCode;
    Q_DISABLE_COPY( DeleteSubscriptionResponse )
};

class UpdatePostResponse : public SoapMessage
{
public:
    explicit UpdatePostResponse();

private:
    void readResponse();
    void readResult();

private:
    Q_DISABLE_COPY( UpdatePostResponse )
};


#endif // KRSS_NEWSGATORBACKEND_SOAPMESSAGES_H
