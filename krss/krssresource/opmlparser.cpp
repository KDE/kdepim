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

#include "opmlparser.h"

#include <krss/feedcollection.h>

#include <KLocale>
#include <KDebug>
#include <QtXml/QXmlStreamWriter>
#include <QtXml/QXmlStreamReader>
#include <QtXml/QXmlAttributes>
#include <memory>

using namespace KRssResource;

class ParsedFeed::Private : public QSharedData
{
public:
    Private() {}

    Private( const Private& other );

    bool operator!=( const Private& other ) const
    {
        return !( *this == other );
    }

    bool operator==( const Private& other ) const
    {
        return title == other.title
            && xmlUrl == other.xmlUrl
            && htmlUrl == other.htmlUrl
            && description == other.description
            && type == other.type
            && attributes == other.attributes;
    }

    QString title;
    QString xmlUrl;
    QString htmlUrl;
    QString description;
    QString type;
    QHash<QString, QString> attributes;
};

ParsedFeed::Private::Private( const Private& other )
    : QSharedData( other ),
    title( other.title ),
    xmlUrl( other.xmlUrl ),
    htmlUrl( other.htmlUrl ),
    description( other.description ),
    type( other.type ),
    attributes( other.attributes )
{
}

ParsedFeed::ParsedFeed() : d( new Private )
{
}

ParsedFeed::~ParsedFeed()
{
}

void ParsedFeed::swap( ParsedFeed& other )
{
    std::swap( d, other.d );
}

ParsedFeed& ParsedFeed::operator=( const ParsedFeed& other )
{
    ParsedFeed copy( other );
    swap( copy );
    return *this;
}

bool ParsedFeed::operator==( const ParsedFeed& other ) const
{
    return *d == *(other.d);
}

bool ParsedFeed::operator!=( const ParsedFeed& other ) const
{
    return *d != *(other.d);
}

ParsedFeed::ParsedFeed( const ParsedFeed& other ) : d( other.d )
{
}

QString ParsedFeed::title() const
{
    return d->title;
}

void ParsedFeed::setTitle( const QString& title )
{
    d->title = title;
}

QString ParsedFeed::xmlUrl() const
{
    return d->xmlUrl;
}

void ParsedFeed::setXmlUrl( const QString& xmlUrl )
{
    d->xmlUrl = xmlUrl;
}

QString ParsedFeed::htmlUrl() const
{
    return d->htmlUrl;
}

void ParsedFeed::setHtmlUrl( const QString& htmlUrl )
{
    d->htmlUrl = htmlUrl;
}

QString ParsedFeed::description() const
{
    return d->description;
}

void ParsedFeed::setDescription( const QString& description )
{
    d->description = description;
}

QString ParsedFeed::type() const
{
    return d->type;
}

void ParsedFeed::setType( const QString& type )
{
    d->type = type;
}

QHash<QString, QString> ParsedFeed::attributes() const
{
    return d->attributes;
}

void ParsedFeed::setAttribute( const QString& key, const QString& value )
{
    d->attributes.insert( key, value );
}

void ParsedFeed::setAttributes( const QHash<QString, QString>& attributes )
{
    d->attributes = attributes;
}

Akonadi::Collection ParsedFeed::toAkonadiCollection() const
{
    KRss::FeedCollection feed;
    feed.setRemoteId( d->attributes.value( "remoteid" ) );
    feed.setTitle( d->title );
    feed.setXmlUrl( d->xmlUrl );
    feed.setHtmlUrl( d->htmlUrl );
    feed.setDescription( d->description );
    feed.setFeedType( d->type );
    feed.setName( d->title );
    feed.setContentMimeTypes( QStringList( "application/rss+xml" ) );
    return feed;
}

ParsedFeed ParsedFeed::fromAkonadiCollection( const Akonadi::Collection& collection )
{
    const KRss::FeedCollection feedCollection = collection;
    ParsedFeed parsedFeed;
    parsedFeed.d->title = feedCollection.title();
    parsedFeed.d->xmlUrl = feedCollection.xmlUrl();
    parsedFeed.d->htmlUrl = feedCollection.htmlUrl();
    parsedFeed.d->description = feedCollection.description();
    parsedFeed.d->type = feedCollection.feedType();
    parsedFeed.d->attributes.insert( "remoteid", feedCollection.remoteId() );
    return parsedFeed;
}

class OpmlReader::Private
{
public:
    explicit Private() {}

    QStringRef attributeValue( const QXmlStreamAttributes& attributes, const QString& name );
    void readBody( QXmlStreamReader& reader );
    void readOutline( QXmlStreamReader& reader, QStringList& currentTags );
    void readUnknownElement( QXmlStreamReader& reader );

    QList<ParsedFeed> m_feeds;
    QList<QString > m_tags;
    QHash<int, QList<int> > m_tagsForFeeds;
};

QStringRef OpmlReader::Private::attributeValue( const QXmlStreamAttributes& attributes, const QString& name )
{
    Q_FOREACH( const QXmlStreamAttribute& attr, attributes ) {
        if ( attr.name().toString().toLower() == name ) {
            return attr.value();
        }
    }

    return QStringRef();
}

void OpmlReader::Private::readBody( QXmlStreamReader& reader )
{
    Q_ASSERT( reader.isStartElement() && reader.name().toString().toLower() == "body" );

    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isEndElement() )
            break;

        if ( reader.isStartElement() ) {
            if ( reader.name().toString().toLower() == "outline" ) {
                QStringList currentTags;
                readOutline( reader, currentTags );
            }
            else {
                readUnknownElement( reader );
            }
        }
    }
}

void OpmlReader::Private::readOutline( QXmlStreamReader& reader, QStringList& currentTags )
{
    Q_ASSERT( reader.isStartElement() && reader.name().toString().toLower() == "outline" );

    bool isFolder = false;
    const QString xmlUrl = attributeValue( reader.attributes(), "xmlurl" ).toString();

    if ( xmlUrl.isEmpty() ) {
        const QStringRef textAttribute = attributeValue( reader.attributes(), "text" );
        if ( !textAttribute.isEmpty() ) {
            // this attribute seem to represent a folder
            isFolder = true;
            if ( !currentTags.contains( textAttribute.toString() ) )
                currentTags.append( textAttribute.toString() );

            kDebug() << "Current tags:" << currentTags;
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
        const QXmlStreamAttributes attrs = reader.attributes();
        Q_FOREACH( const QXmlStreamAttribute& attr, attrs ) {
            if ( attr.name().toString().toLower() == "title" )
                feed.setTitle( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "text" && feed.title().isEmpty() )
                feed.setTitle( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "htmlurl" )
                feed.setHtmlUrl( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "xmlurl" )
                feed.setXmlUrl( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "description" )
                feed.setDescription( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "type" )
                feed.setType( attr.value().toString() );
            else if ( attr.name().toString().toLower() == "category" ) {
                const QStringList categories = attr.value().toString().split( QRegExp( "[,/]" ), QString::SkipEmptyParts );
                Q_FOREACH( const QString& category, categories ) {
                    if ( !currentTags.contains( category ) )
                        currentTags.append( category );
                }
            }
            else {
                feed.setAttribute( attr.name().toString(), attr.value().toString() );
            }
        }

        if ( feed.title().isEmpty() )
            feed.setTitle( xmlUrl );

        if ( feed.type().isEmpty() )
            feed.setType( "rss" );

        // everything is parsed
        QList<int> currentTagIds;
        Q_FOREACH( const QString& tag, currentTags ) {
            int index = m_tags.indexOf( tag );
            if ( index == -1 ) {
                m_tags.append( tag );
                index = m_tags.count() - 1;
            }
            currentTagIds.append( index );
        }

        m_feeds.append( feed );
        m_tagsForFeeds.insert( m_feeds.count() - 1, currentTagIds );
    }

    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isEndElement() )
            break;

        if ( reader.isStartElement() ) {
            if ( reader.name().toString().toLower() == "outline" && isFolder )
                readOutline( reader, currentTags );
            else
                readUnknownElement( reader );
        }
    }

    // once we are back from recursion remove the added tag
    // from the top of the list
    if ( isFolder )
        currentTags.removeLast();
}

void OpmlReader::Private::readUnknownElement( QXmlStreamReader& reader )
{
    Q_ASSERT( reader.isStartElement() );

    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isEndElement() )
            break;

        if ( reader.isStartElement() )
            readUnknownElement( reader );
    }
}

OpmlReader::OpmlReader()
    : d( new Private )
{
}

OpmlReader::~OpmlReader()
{
    delete d;
}

QList<ParsedFeed> OpmlReader::feeds() const
{
    return d->m_feeds;
}

QList<QString> OpmlReader::tags() const
{
    return d->m_tags;
}

QHash<int, QList<int> > OpmlReader::tagsForFeeds() const
{
    return d->m_tagsForFeeds;
}

void OpmlReader::readOpml( QXmlStreamReader& reader )
{
    Q_ASSERT( reader.isStartElement() && reader.name().toString().toLower() == "opml" );

    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isEndElement() )
            break;

        if ( reader.isStartElement() ) {
            if ( reader.name().toString().toLower() == "head" ) {
                d->readUnknownElement( reader );
            }
            else if ( reader.name().toString().toLower() == "body" ) {
                d->readBody( reader );
            }
            else {
                d->readUnknownElement( reader );
            }
        }
    }
}

void OpmlWriter::writeOpml( QXmlStreamWriter& writer, const QList<ParsedFeed>& feeds )
{
    writer.writeStartElement( "opml" );
    writer.writeAttribute( "version", "2.0" );
    writer.writeEmptyElement( "head" );
    writer.writeStartElement( "body" );
    Q_FOREACH( const ParsedFeed& feed, feeds ) {
        writeOutline( writer, feed );
    }
    writer.writeEndElement(); // body
    writer.writeEndElement(); // opml
}

void OpmlWriter::writeOutline( QXmlStreamWriter& writer, const ParsedFeed& feed )
{
    writer.writeStartElement( "outline" );
    writer.writeAttribute( "text", feed.title() );
    writer.writeAttribute( "title", feed.title() );
    writer.writeAttribute( "description", feed.description() );
    writer.writeAttribute( "htmlUrl", feed.htmlUrl() );
    writer.writeAttribute( "xmlUrl", feed.xmlUrl() );
    QHashIterator<QString, QString> it( feed.attributes() );
    while ( it.hasNext() ) {
        it.next();
        writer.writeAttribute( it.key(), it.value() );
    }
    writer.writeEndElement();   // outline
}
