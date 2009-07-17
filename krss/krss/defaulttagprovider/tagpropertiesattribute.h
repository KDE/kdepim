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

#ifndef KRSS_TAGPROPERTIESATTRIBUTE_H
#define KRSS_TAGPROPERTIESATTRIBUTE_H

#include <akonadi/attribute.h>

#include <QtCore/QString>

namespace KRss {

class TagPropertiesAttribute : public Akonadi::Attribute
{
public:

    TagPropertiesAttribute();
    QByteArray type() const;
    TagPropertiesAttribute* clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray& data );

    QString label() const;
    void setLabel( const QString& label );
    QString description() const;
    void setDescription( const QString& description );
    QString icon() const;
    void setIcon( const QString& icon );

private:

    QString m_label;
    QString m_description;
    QString m_icon;
};

} // namespace KRss

#endif // KRSS_TAGPROPERTIESATTRIBUTE_H
