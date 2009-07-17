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

#ifndef KRSS_NEPOMUKTAG_P_H
#define KRSS_NEPOMUKTAG_P_H

#include "krss/tag_p.h"

using namespace KRss;

namespace KRss {

class NepomukTagPrivate : public TagPrivate
{
public:
    explicit NepomukTagPrivate( const TagId& id )
        : TagPrivate( id )
    {
        m_url = id;
    }

    NepomukTagPrivate( const NepomukTagPrivate& other )
        : TagPrivate( other ), m_url( other.m_url ), m_label( other.m_label ),
          m_description( other.m_description ), m_icon( other.m_icon )
    {
    }

    QString label() const
    {
        return m_label;
    }

    void setLabel( const QString& label )
    {
        m_label = label;
    }

    QString description() const
    {
        return m_description;
    }

    void setDescription( const QString& description )
    {
        m_description = description;
    }

    QString icon() const
    {
        return m_icon;
    }

    void setIcon( const QString& icon )
    {
        m_icon = icon;
    }

private:
    KUrl m_url;
    QString m_label;
    QString m_description;
    QString m_icon;
};

} // namespace KRss

#endif // KRSS_NEPOMUKTAG_P_H
