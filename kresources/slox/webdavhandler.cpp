/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <config.h>

#include "webdavhandler.h"

#ifdef HAVE_VALUES_H
#include <values.h>
#else
#ifdef HAVE_SYS_LIMITS_H
#include <sys/limits.h>
#endif
#endif

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>
#include <kconfig.h>

#include <qfile.h>


SloxItem::SloxItem()
  : status( Invalid )
{
}

WebdavHandler::WebdavHandler()
  : mLogCount( 0 )
{
  KConfig cfg( "sloxrc" );

  cfg.setGroup( "General" );
  mLogFile = cfg.readEntry( "LogFile" );

  kdDebug() << "LOG FILE: " << mLogFile << endl;
}

void WebdavHandler::log( const QString &text )
{
  if ( mLogFile.isEmpty() ) return;

  QString filename = mLogFile + "-" + QString::number( mLogCount );
  QFile file( filename );
  if ( !file.open( IO_WriteOnly ) ) {
    kdWarning() << "Unable to open log file '" << filename << "'" << endl;
    return;
  }
  
  QCString textUtf8 = text.utf8();
  file.writeBlock( textUtf8.data(), textUtf8.size() - 1 );
  
  if ( ++mLogCount > 5 ) mLogCount = 0;
}

QValueList<SloxItem> WebdavHandler::getSloxItems( const QDomDocument &doc )
{
  kdDebug() << "getSloxItems" << endl;

  QValueList<SloxItem> items;

  QDomElement docElement = doc.documentElement();

  QDomNode responseNode;
  for( responseNode = docElement.firstChild(); !responseNode.isNull();
       responseNode = responseNode.nextSibling() ) {
    QDomElement responseElement = responseNode.toElement();
    if ( responseElement.tagName() == "response" ) {
      QDomNode propstat = responseElement.namedItem( "propstat" );
      if ( propstat.isNull() ) {
        kdError() << "Unable to find propstat tag." << endl;
        continue;
      }

      QDomNode prop = propstat.namedItem( "prop" );
      if ( prop.isNull() ) {
        kdError() << "Unable to find WebDAV property" << endl;
        continue;
      }

      QDomNode sloxIdNode = prop.namedItem( "sloxid" );
      if ( sloxIdNode.isNull() ) {
        kdError() << "Unable to find SLOX id." << endl;
        continue;
      }
      QDomElement sloxIdElement = sloxIdNode.toElement();
      QString sloxId = sloxIdElement.text();

      QDomNode sloxStatus = prop.namedItem( "sloxstatus" );
      if ( sloxStatus.isNull() ) {
        kdError() << "Unable to find SLOX status." << endl;
        continue;
      }

      SloxItem item;
      item.sloxId = sloxId;
      item.domNode = prop;

      QDomElement sloxStatusElement = sloxStatus.toElement();
      if ( sloxStatusElement.text() == "DELETE" ) {
        item.status = SloxItem::Delete;
      } else if ( sloxStatusElement.text() == "CREATE" ) {
        item.status = SloxItem::Create;
      }

      items.append( item );
    }
  }
  
  return items;
}

QString WebdavHandler::qDateTimeToSlox( const QDateTime &dt )
{
  uint ticks = -dt.secsTo( QDateTime( QDate( 1970, 1, 1 ), QTime( 0, 0 ) ) );

  return QString::number( ticks ) + "000";
}

QString WebdavHandler::qDateTimeToSlox( const QDateTime &dt,
                                        const QString &timeZoneId )
{
  QDateTime utc = KPimPrefs::localTimeToUtc( dt, timeZoneId );

  uint ticks = -utc.secsTo( QDateTime( QDate( 1970, 1, 1 ), QTime( 0, 0 ) ) );

  return QString::number( ticks ) + "000";
}

QDateTime WebdavHandler::sloxToQDateTime( const QString &str )
{
  QString s = str.mid( 0, str.length() - 3 );

  bool preEpoch = s.startsWith("-");
  if (preEpoch)
     s = s.mid(1);

  unsigned long ticks = s.toULong();

  QDateTime dt;

  if (preEpoch) {
    dt.setTime_t( 0, Qt::UTC );
    if (ticks > INT_MAX) {
      dt = dt.addSecs(-INT_MAX);
      ticks -= INT_MAX;
    }
    dt = dt.addSecs(-((long) ticks));
  }
  else
  {
    dt.setTime_t( ticks, Qt::UTC );
  }

  return dt;
}

QDateTime WebdavHandler::sloxToQDateTime( const QString &str,
                                          const QString &timeZoneId )
{
  return KPimPrefs::utcToLocalTime( sloxToQDateTime(str), timeZoneId );
}

QDomElement WebdavHandler::addElement( QDomDocument &doc, QDomNode &node,
                                       const QString &tag )
{
  QDomElement el = doc.createElement( tag );
  node.appendChild( el );
  return el;
}

QDomElement WebdavHandler::addDavElement( QDomDocument &doc, QDomNode &node,
                                          const QString &tag )
{
  QDomElement el = doc.createElementNS( "DAV", tag );
  node.appendChild( el );
  return el;
}

QDomElement WebdavHandler::addSloxElement( QDomDocument &doc, QDomNode &node,
                                           const QString &tag,
                                           const QString &text )
{
  QDomElement el = doc.createElementNS( "SLOX", tag );
  if ( !text.isEmpty() ) {
    QDomText textnode = doc.createTextNode( text );
    el.appendChild( textnode );
  }
  node.appendChild( el );
  return el;
}
