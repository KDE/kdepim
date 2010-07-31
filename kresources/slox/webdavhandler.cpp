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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "webdavhandler.h"
#include "sloxbase.h"

#ifdef HAVE_VALUES_H
#include <values.h>
#else
#ifdef HAVE_SYS_LIMITS_H
#include <sys/limits.h>
#endif
#endif

#include <config.h>
#include <stdlib.h>

#include <libkcal/incidence.h>

#include <libkdepim/kpimprefs.h>

#include <kdebug.h>
#include <kconfig.h>

#include <tqfile.h>

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

void WebdavHandler::setUserId( const TQString &id )
{
  mUserId = id;
}

TQString WebdavHandler::userId() const
{
  return mUserId;
}


void WebdavHandler::log( const TQString &text )
{
  if ( mLogFile.isEmpty() ) return;

  TQString filename = mLogFile + "-" + TQString::number( mLogCount );
  TQFile file( filename );
  if ( !file.open( IO_WriteOnly ) ) {
    kdWarning() << "Unable to open log file '" << filename << "'" << endl;
    return;
  }

  TQCString textUtf8 = text.utf8();
  file.writeBlock( textUtf8.data(), textUtf8.size() - 1 );

  if ( ++mLogCount > 5 ) mLogCount = 0;
}

TQValueList<SloxItem> WebdavHandler::getSloxItems( SloxBase *res, const TQDomDocument &doc )
{
  kdDebug() << "getSloxItems" << endl;

  TQValueList<SloxItem> items;

  TQDomElement docElement = doc.documentElement();

  TQDomNode responseNode;
  for( responseNode = docElement.firstChild(); !responseNode.isNull();
       responseNode = responseNode.nextSibling() ) {
    TQDomElement responseElement = responseNode.toElement();
    if ( responseElement.tagName() == "response" ) {
      SloxItem item;

      TQDomNode propstat = responseElement.namedItem( "propstat" );
      if ( propstat.isNull() ) {
        kdError() << "Unable to find propstat tag." << endl;
        continue;
      }

      TQDomNode prop = propstat.namedItem( "prop" );
      if ( prop.isNull() ) {
        kdError() << "Unable to find WebDAV property" << endl;
        continue;
      }
      item.domNode = prop;

      TQDomNode sloxIdNode = prop.namedItem( res->fieldName( SloxBase::ObjectId ) );
      if ( sloxIdNode.isNull() ) {
        kdError() << "Unable to find SLOX id." << endl;
        continue;
      }
      TQDomElement sloxIdElement = sloxIdNode.toElement();
      item.sloxId = sloxIdElement.text();

      TQDomNode clientIdNode = prop.namedItem( res->fieldName( SloxBase::ClientId ) );
      if ( !clientIdNode.isNull() ) {
        TQDomElement clientIdElement = clientIdNode.toElement();
        item.clientId = clientIdElement.text();
        if ( item.clientId != item.sloxId )
          item.status = SloxItem::New;
      }

      TQDomNode sloxStatus = prop.namedItem( res->fieldName( SloxBase::ObjectStatus ) );
      if ( !sloxStatus.isNull() ) {
        TQDomElement sloxStatusElement = sloxStatus.toElement();
        if ( sloxStatusElement.text() == "DELETE" ) {
          item.status = SloxItem::Delete;
        } else if ( sloxStatusElement.text() == "CREATE" ) {
          item.status = SloxItem::Create;
        }
      }

      TQDomNode status = propstat.namedItem( "status" );
      if ( status.isNull() ) {
        kdError() << "Unable to find WebDAV status" << endl;
        continue;
      }
      item.response = status.toElement().text();

      TQDomNode desc = propstat.namedItem( "responsedescription" );
      if ( desc.isNull() ) {
        kdError() << "Unable to find WebDAV responsedescription" << endl;
        continue;
      }
      item.responseDescription = desc.toElement().text();

      items.append( item );
    }
  }

  return items;
}

TQString WebdavHandler::qDateTimeToSlox( const TQDateTime &dt )
{
  uint ticks = -dt.secsTo( TQDateTime( TQDate( 1970, 1, 1 ), TQTime( 0, 0 ) ) );

  return TQString::number( ticks ) + "000";
}

TQString WebdavHandler::qDateTimeToSlox( const TQDateTime &dt,
                                        const TQString &timeZoneId )
{
  TQDateTime utc = KPimPrefs::localTimeToUtc( dt, timeZoneId );

  // secsTo and toTime_t etc also perform a timezone conversion using the system timezone,
  // but we want to use the calendar timezone, so we have to convert ourself and spoof the tz to UTC before
  // converting to ticks to prevent this
  TQCString origTz = getenv("TZ");
  setenv( "TZ", "UTC", 1 );
  uint ticks = utc.toTime_t();
  if ( origTz.isNull() )
    unsetenv( "TZ" );
  else
    setenv( "TZ", origTz, 1 );

  return TQString::number( ticks ) + "000";
}

TQDateTime WebdavHandler::sloxToQDateTime( const TQString &str )
{
  TQString s = str.mid( 0, str.length() - 3 );

  bool preEpoch = s.startsWith("-");
  if (preEpoch)
     s = s.mid(1);

  unsigned long ticks = s.toULong();

  TQDateTime dt;

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

TQDateTime WebdavHandler::sloxToQDateTime( const TQString &str,
                                          const TQString &timeZoneId )
{
  return KPimPrefs::utcToLocalTime( sloxToQDateTime(str), timeZoneId );
}

TQDomElement WebdavHandler::addElement( TQDomDocument &doc, TQDomNode &node,
                                       const TQString &tag )
{
  TQDomElement el = doc.createElement( tag );
  node.appendChild( el );
  return el;
}

TQDomElement WebdavHandler::addDavElement( TQDomDocument &doc, TQDomNode &node,
                                          const TQString &tag )
{
  TQDomElement el = doc.createElementNS( "DAV:", "D:" + tag );
  node.appendChild( el );
  return el;
}

TQDomElement WebdavHandler::addSloxElement( SloxBase *res,
                                           TQDomDocument &doc, TQDomNode &node,
                                           const TQString &tag,
                                           const TQString &text )
{
  TQDomElement el;
  if ( res->resType() == "ox" )
    el = doc.createElementNS( "http://www.open-xchange.org", "ox:" + tag );
  else
    el = doc.createElementNS( "SLOX", "S:" + tag );
  if ( !text.isEmpty() ) {
    TQDomText textnode = doc.createTextNode( text );
    el.appendChild( textnode );
  }
  node.appendChild( el );
  return el;
}

void WebdavHandler::parseSloxAttribute( const TQDomElement &e )
{
//  kdDebug() << "parseSloxAttribute" << endl;

  TQString tag = e.tagName();
  TQString text = TQString::fromUtf8( e.text().latin1() );
  if ( text.isEmpty() ) return;

  if ( tag == "owner" ) {
    if ( text == mUserId ) mWritable = true;
  } else if ( tag == "writerights" ) {
    TQDomNode n;
    for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
      TQDomElement e2 = n.toElement();
      if ( e2.tagName() == "member" ) {
        if ( e2.text() == mUserId ) mWritable = true;
      }
      // TODO: Process group write rights
    }
  }
}

void WebdavHandler::clearSloxAttributeStatus()
{
  if ( mRes->resType() == "ox" )
    mWritable = true; // parseSloxAttribute() won't work for OX
  else
    mWritable = false;
}

void WebdavHandler::setSloxAttributes( KCal::Incidence *i )
{
  i->setReadOnly( !mWritable );
}

void WebdavHandler::setSloxAttributes( KABC::Addressee & )
{
  // FIXME: libkabc doesn't allow to set an individual addressee to read-only
}
