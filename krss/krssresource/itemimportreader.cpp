/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#include "itemimportreader.h"

#include <krss/category.h>
#include <krss/enclosure.h>
#include <krss/rssitem.h>
#include <krss/item.h>
#include <krss/person.h>

#include <syndication/atom/constants.h>
#include <syndication/constants.h>

#include <Akonadi/Item>
#include <KDateTime>

#include <QXmlStreamReader>
#include <QVariant>

using namespace KRssResource;

namespace {

enum TextMode {
    PlainText,
    Html
};

static QString akregatorNamespace() {
    return QString::fromLatin1("http://akregator.kde.org/StorageExporter#");
}

class Element
{
public:
    Element( const QString& ns_, const QString& name_, const QVariant& defaultValue_ = QVariant() ) : ns( ns_ ), name( name_ ), qualifiedName( ns + ':' + name ), defaultValue( defaultValue_ )
    {
    }

    const QString ns;
    const QString name;
    const QString qualifiedName;
    const QVariant defaultValue;

    bool isNextIn( const QXmlStreamReader& reader ) const
    {
        return reader.isStartElement() && reader.name() == name && reader.namespaceUri() == ns;
    }
};

struct Elements
{
    Elements() : atomNS( Syndication::Atom::atom1Namespace() ),
                 akregatorNS( akregatorNamespace() ),
                 commentNS( Syndication::commentApiNamespace() ),
                 title( atomNS, "title", QString() ),
                 summary( atomNS, "summary", QString() ),
                 content( atomNS, "content", QString() ),
                 link( atomNS, "link", QString() ),
                 language( atomNS, "language", QString() ),
                 guid( atomNS, "id", QString() ),
                 published( atomNS, "published", KDateTime().toString( KDateTime::ISODate ) ),
                 updated( atomNS, "updated", KDateTime().toString( KDateTime::ISODate ) ),
                 commentsCount( Syndication::slashNamespace(), "comments", -1 ),
                 commentsFeed( commentNS, "commentRss", QString() ),
                 commentPostUri( commentNS, "comment", QString() ),
                 commentsLink( akregatorNS, "commentsLink", QString() ),
                 hash( akregatorNS, "hash", 0 ),
                 guidIsHash( akregatorNS, "idIsHash", false ),
                 sourceFeedId( akregatorNS, "sourceFeedId", -1 ),
                 name( atomNS, "name", QString() ),
                 uri( atomNS, "uri", QString() ),
                 email( atomNS, "email", QString() ),
                 author( atomNS, "author", QString() ),
                 category( atomNS, "category", QString() ),
                 entry( atomNS, "entry", QString() ),
                 readStatus( akregatorNS, "readStatus" ),
                 deleted( akregatorNS, "deleted" ),
                 important( akregatorNS, "important" )
{}
    const QString atomNS;
    const QString akregatorNS;
    const QString commentNS;
    const Element title;
    const Element summary;
    const Element content;
    const Element link;
    const Element language;
    const Element guid;
    const Element published;
    const Element updated;
    const Element commentsCount;
    const Element commentsFeed;
    const Element commentPostUri;
    const Element commentsLink;
    const Element hash;
    const Element guidIsHash;
    const Element sourceFeedId;
    const Element name;
    const Element uri;
    const Element email;
    const Element author;
    const Element category;
    const Element entry;
    const Element readStatus;
    const Element deleted;
    const Element important;
    static const Elements instance;
};

const Elements Elements::instance;

static void readLink( KRss::RssItem& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    const QString rel = attrs.value( QString(), "rel" ).toString();
    if (  rel == "alternate" )
    {
        item.setLink( attrs.value( QString(), "href" ).toString() );
    }
    else if ( rel == "enclosure" )
    {
        KRss::Enclosure enc;
        enc.setUrl( attrs.value( QString(), "href" ).toString() );
        enc.setType( attrs.value( QString(), "type" ).toString() );
        enc.setTitle( attrs.value( QString(), "title" ).toString() );
        bool ok;
        const uint length = attrs.value( QString(), "length" ).toString().toUInt( &ok );
        if ( ok )
            enc.setLength( length );
        const uint duration = attrs.value( Syndication::itunesNamespace(), "duration" ).toString().toUInt( &ok );
        if ( ok )
            enc.setDuration( duration );
        QList<KRss::Enclosure> encs = item.enclosures();
        encs.append( enc );
        item.setEnclosures( encs );
    }
}

static void readAuthor( KRss::RssItem& item, QXmlStreamReader& reader )
{
    KRss::Person author;
    int depth = 1;
    while ( !reader.atEnd() && depth > 0 )
    {
        reader.readNext();
        if ( reader.isEndElement() )
            --depth;
        else if ( reader.isStartElement() )
        {
            if ( Elements::instance.name.isNextIn( reader ) )
                author.setName( reader.readElementText() );
            else if ( Elements::instance.uri.isNextIn( reader ) )
                author.setUri( reader.readElementText() );
            else if ( Elements::instance.email.isNextIn( reader ) )
                author.setEmail( reader.readElementText() );
        }

    }
    QList<KRss::Person> authors = item.authors();
    authors.append( author );
    item.setAuthors( authors );
}

static void readCategory( KRss::RssItem& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    KRss::Category cat;
    cat.setTerm( attrs.value( QString(), "term" ).toString() );
    cat.setScheme( attrs.value( QString(), "scheme" ).toString() );
    cat.setLabel( attrs.value( QString(), "label" ).toString() );
    QList<KRss::Category> cats = item.categories();
    cats.append( cat );
    item.setCategories( cats );
}

static bool readItem( Akonadi::Item& akonadiItem, QXmlStreamReader& reader ) {
    const Elements& el = Elements::instance;
    if ( !reader.atEnd() )
        reader.readNext();

    Akonadi::Item::Flags flags;
    KRss::RssItem item;
    item.setHeadersLoaded( true );
    item.setContentLoaded( true );

    while ( !reader.atEnd() && !el.entry.isNextIn( reader ) )
    {
        if ( reader.isStartElement() )
        {
            if ( el.title.isNextIn( reader ) )
                item.setTitle( reader.readElementText() );
            else if ( el.summary.isNextIn( reader ) )
                item.setDescription( reader.readElementText() );
            else if ( el.content.isNextIn( reader ) )
                item.setContent( reader.readElementText() );
            else if ( el.language.isNextIn( reader ) )
                item.setLanguage( reader.readElementText() );
            else if ( el.guid.isNextIn( reader ) )
                item.setGuid( reader.readElementText() );
            else if ( el.hash.isNextIn( reader ) )
                item.setHash( reader.readElementText().toInt() );
            else if ( el.guidIsHash.isNextIn( reader ) )
                item.setGuidIsHash( QVariant( reader.readElementText() ).toBool() );
            else if ( el.sourceFeedId.isNextIn( reader ) )
                item.setSourceFeedId( reader.readElementText().toInt() );
            else if ( el.commentsLink.isNextIn( reader ) )
                item.setCommentsLink( reader.readElementText() );
            else if ( el.commentPostUri.isNextIn( reader ) )
                item.setCommentPostUri( reader.readElementText() );
            else if ( el.commentsCount.isNextIn( reader ) )
                item.setCommentsCount( reader.readElementText().toInt() );
            else if ( el.commentsFeed.isNextIn( reader ) )
                item.setCommentsFeed( reader.readElementText() );
            else if ( el.link.isNextIn( reader ) )
                ::readLink( item, reader );
            else if ( el.author.isNextIn( reader ) )
                ::readAuthor( item, reader );
            else if ( el.category.isNextIn( reader ) )
                ::readCategory( item, reader );
            else if ( el.published.isNextIn( reader ) )
                item.setDatePublished( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
            else if ( el.updated.isNextIn( reader ) )
                item.setDateUpdated( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
            else if ( el.readStatus.isNextIn( reader ) ) {
                const QString statusStr = reader.readElementText();
                if ( statusStr == "new" )
                    flags.insert( KRss::RssItem::flagNew() );
                else if ( statusStr != "unread" )
                    flags.insert( KRss::RssItem::flagRead() );
            } else if ( el.important.isNextIn( reader ) ) {
                if ( reader.readElementText() == "true" )
                    flags.insert( KRss::RssItem::flagImportant() );
            } else if ( el.deleted.isNextIn( reader ) ) {
                if ( reader.readElementText() == "true" )
                    flags.insert( KRss::RssItem::flagDeleted() );
            }

        }

        reader.readNext();
    }

    akonadiItem.setPayload<KRss::RssItem>( item );
    akonadiItem.setMimeType( KRss::Item::mimeType() );
    akonadiItem.setRemoteId( item.guid() );
    akonadiItem.setFlags( flags );
    return !reader.hasError();
}

} // namespace

ItemImportReader::ItemImportReader( QIODevice* dev ) : m_reader( new QXmlStreamReader( dev ) ) {
    m_reader->setNamespaceProcessing( true );
    m_firstItemFound = findFirstItem();
}

ItemImportReader::~ItemImportReader() {
    delete m_reader;
}

bool ItemImportReader::hasNext() const {
    return m_firstItemFound && !m_reader->atEnd();
}

Akonadi::Item ItemImportReader::nextItem() {
    const Elements el;
    if ( !m_firstItemFound )
        return Akonadi::Item();

    while ( !m_reader->atEnd() )
        if ( m_reader->isStartElement() &&
                el.entry.isNextIn( *m_reader ) ) {
                Akonadi::Item item;
                if ( readItem( item, *m_reader ) )
                    return item;
                else
                    return Akonadi::Item();
            }
    return Akonadi::Item();
}

bool ItemImportReader::findFirstItem() {
    const Elements el;
    bool itemFound = false;
    while ( !itemFound && !m_reader->atEnd() ) {
        m_reader->readNext();
        if ( m_reader->isStartElement() && el.entry.isNextIn( *m_reader ) )
            itemFound = true;
    }
    return itemFound;
}
