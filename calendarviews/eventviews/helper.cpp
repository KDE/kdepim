/*
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "helper.h"
#include "prefs.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KIconLoader>

#include <QPixmap>
#include <QPixmapCache>

QColor EventViews::getTextColor( const QColor &c )
{
  double luminance = ( c.red() * 0.299 ) + ( c.green() * 0.587 ) + ( c.blue() * 0.114 );
  return ( luminance > 128.0 ) ? QColor( 0, 0, 0 ) : QColor( 255, 255, 255 );
}

QColor EventViews::resourceColor( const Akonadi::Collection &coll, const PrefsPtr &preferences )
{
  if ( !coll.isValid() ) {
    return QColor();
  }
  const QString id = QString::number( coll.id() );
  return preferences->resourceColor( id );
}

QColor EventViews::resourceColor( const Akonadi::Item &item, const PrefsPtr &preferences )
{
  if ( !item.isValid() ) {
    return QColor();
  }
  const QString id = QString::number( item.storageCollectionId() );
  return preferences->resourceColor( id );
}

int EventViews::yearDiff( const QDate &start, const QDate &end )
{
  return end.year() - start.year();
}

QPixmap EventViews::cachedSmallIcon( const QString &name )
{
  QPixmap p;
  if ( !QPixmapCache::find( name, &p ) ) {
    p = SmallIcon( name );
  }

  return p;
}
