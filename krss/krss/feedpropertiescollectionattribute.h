/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H
#define KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H

#include <akonadi/attribute.h>

#include <QtCore/QString>
#include <QtCore/QHash>

namespace KRss {

class FeedPropertiesCollectionAttribute : public Akonadi::Attribute
{
public:

        FeedPropertiesCollectionAttribute();
        QByteArray type() const;
        FeedPropertiesCollectionAttribute* clone() const;
        QByteArray serialized() const;
        void deserialize( const QByteArray &data );

        QString name() const;
        void setName( const QString &name );
        QString xmlUrl() const;
        void setXmlUrl( const QString &xmlUrl );
        QString htmlUrl() const;
        void setHtmlUrl( const QString &htmlUrl );
        QString feedType() const;
        void setFeedType( const QString &feedType );
        QString description() const;
        void setDescription( const QString &description );
        bool preferItemLinkForDisplay() const;
        void setPreferItemLinkForDisplay( bool b );

private:
        QHash<QString,QString> m_properties;
};

} // namespace KRss

#endif // KRSS_FEEDPROPERTIESCOLLECTIONATTRIBUTE_H
