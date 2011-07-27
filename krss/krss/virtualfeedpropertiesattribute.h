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

#ifndef KRSS_VIRTUALFEEDPROPERTIESATTRIBUTE_H
#define KRSS_VIRTUALFEEDPROPERTIESATTRIBUTE_H

#include "krss_export.h"

#include <akonadi/attribute.h>
#include <QtCore/QString>
#include <QtCore/QHash>

namespace KRss {

class KRSS_EXPORT VirtualFeedPropertiesAttribute : public Akonadi::Attribute
{
public:
    VirtualFeedPropertiesAttribute() {}
    QByteArray type() const;
    VirtualFeedPropertiesAttribute* clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray &data );

    QString title() const;
    void setTitle( const QString &title );
    QString description() const;
    void setDescription( const QString &description );

private:
    QHash<QString,QString> m_properties;
};

} // namespace KRss

#endif // KRSS_VIRTUALFEEDPROPERTIESATTRIBUTE_H
