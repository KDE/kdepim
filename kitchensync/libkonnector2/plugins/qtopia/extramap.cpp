/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdom.h>
#include <qstylesheet.h>
#include <qstringlist.h>

#include "extramap.h"

using namespace OpieHelper;

/**
 * d'tor
 */
CustomExtraItem::~CustomExtraItem() {}


ExtraMap::~ExtraMap()
{
  clear();
}

void ExtraMap::clear()
{
  ExtraMapBase::clear();

  for ( QMap<CUID, CustomExtraItem*>::Iterator it = m_custom.begin();
        it != m_custom.end(); ++it )
    delete it.data();

  m_custom.clear();
}


QString ExtraMap::toString( const CUID& cuid)
{
    if (!contains( cuid ) ) return QString::null;

    KeyValue val = (*this)[cuid];
    KeyValue::Iterator it;
    QString str;
    for (it = val.begin(); it != val.end(); ++it )
        str += " "+it.key()+"=\""+escape( it.data() )+"\"";


    return str;
}
QString ExtraMap::toString( const QString& app, const QString& uid )
{
    return toString(app+uid);
}

void ExtraMap::add( const QString& app, const QString& uid, const QDomNamedNodeMap& map, const QStringList& lst )
{
    KeyValue val;
    uint count =  map.count();
    for ( uint i = 0; i < count; i++ ) {
        QDomAttr attr = map.item( i ).toAttr();
        if (!attr.isNull() ) {
            if (!lst.contains(attr.name() ) ) {
                val.insert( attr.name(), attr.value() );
            }
        }
    }
    insert(app+uid, val );
}

QString ExtraMap::escape( const QString& str )
{
    return QStyleSheet::escape( str );
}


void ExtraMap::add( const QString& app, const QString& type,
                    const QString& uid, CustomExtraItem* item )
{
  m_custom.insert(app+type+uid, item );
}

CustomExtraItem* ExtraMap::item( const QString& app,
                                 const QString& type,
                                 const QString& uid )
{
  return m_custom[app+type+uid];
}
