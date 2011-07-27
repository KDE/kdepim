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

#ifndef KRSSRESOURCE_ITEMIMPORTREADER_H
#define KRSSRESOURCE_ITEMIMPORTREADER_H

#include "krssresource_export.h"

#include <QString>
#include <QXmlStreamReader>

class QIODevice;

namespace Akonadi {
    class Item;
}

namespace KRssResource
{

class KRSSRESOURCE_TESTS_EXPORT ItemImportReader {
public:
    explicit ItemImportReader( QIODevice* dev );
    ~ItemImportReader();

    bool hasNext() const;

    Akonadi::Item nextItem();

private:
    bool findFirstItem();

private:
    QXmlStreamReader* m_reader;
    QString m_sourceFeed;
    bool m_firstItemFound;
};

} // namespace KRssResource

#endif // KRSSRESOURCE_ITEMIMPORTREADER_H
