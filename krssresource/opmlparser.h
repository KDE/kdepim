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

#ifndef KRSSRESOURCE_OPMLPARSER_H
#define KRSSRESOURCE_OPMLPARSER_H

#include "krssresource_export.h"

#include <Akonadi/Collection>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QXmlStreamReader>

class QXmlStreamWriter;
class QXmlStreamAttributes;

namespace KRssResource {

class KRSSRESOURCE_EXPORT ParsedFeed
{
public:
    ParsedFeed();
    ParsedFeed( const ParsedFeed& other );
    ~ParsedFeed();

    void swap( ParsedFeed& other );
    ParsedFeed& operator=( const ParsedFeed& other );
    bool operator==( const ParsedFeed& other ) const;
    bool operator!=( const ParsedFeed& other ) const;

    QString title() const;
    void setTitle( const QString& title );
    QString xmlUrl() const;
    void setXmlUrl( const QString& xmlUrl );
    QString htmlUrl() const;
    void setHtmlUrl( const QString& htmlUrl );
    QString description() const;
    void setDescription( const QString& description );
    QString type() const;
    void setType( const QString& type );
    QHash<QString, QString> attributes() const;
    void setAttribute( const QString& key, const QString& value );
    void setAttributes( const QHash<QString, QString>& attributes );

    Akonadi::Collection toAkonadiCollection() const;
    static ParsedFeed fromAkonadiCollection( const Akonadi::Collection& collection );

private:
    class Private;
    QSharedDataPointer<Private> d;
};

class KRSSRESOURCE_EXPORT OpmlReader
{
public:
    OpmlReader();
    ~OpmlReader();

    void readOpml( QXmlStreamReader& reader );

    QList<ParsedFeed> feeds() const;
    QList<QString> tags() const;
    QHash<int, QList<int> > tagsForFeeds() const;

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( OpmlReader )
};

class KRSSRESOURCE_EXPORT OpmlWriter
{
public:
    static void writeOpml( QXmlStreamWriter& writer, const QList<ParsedFeed>& feeds );
    static void writeOutline( QXmlStreamWriter& writer, const ParsedFeed& feed );
};

} //namespace KRssResource

#endif // KRSSRESOURCE_OPMLPARSER_H
