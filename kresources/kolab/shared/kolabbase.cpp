/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kolabbase.h"

#include <kabc/addressee.h>
#include <libkcal/journal.h>
#include <libkdepim/kpimprefs.h>
#include <kdebug.h>
#include <qfile.h>

using namespace Kolab;


KolabBase::KolabBase( const QString& tz )
  : mCreationDate( QDateTime::currentDateTime() ),
    mLastModified( QDateTime::currentDateTime() ),
    mSensitivity( Public ), mTimeZoneId( tz ),
    mHasPilotSyncId( false ),  mHasPilotSyncStatus( false )
{
}

KolabBase::~KolabBase()
{
}

void KolabBase::setFields( const KCal::Incidence* incidence )
{
  // So far unhandled KCal::IncidenceBase fields:
  // mPilotID, mSyncStatus, mFloats

  setUid( incidence->uid() );
  setBody( incidence->description() );
  setCategories( incidence->categoriesStr() );
  setCreationDate( localToUTC( incidence->created() ) );
  setLastModified( localToUTC( incidence->lastModified() ) );
  setSensitivity( static_cast<Sensitivity>( incidence->secrecy() ) );
  // TODO: Attachments
}

void KolabBase::saveTo( KCal::Incidence* incidence ) const
{
  incidence->setUid( uid() );
  incidence->setDescription( body() );
  incidence->setCategories( categories() );
  incidence->setCreated( utcToLocal( creationDate() ) );
  incidence->setLastModified( utcToLocal( lastModified() ) );
  incidence->setSecrecy( sensitivity() );
  // TODO: Attachments
}

void KolabBase::setFields( const KABC::Addressee* addressee )
{
  // An addressee does not have a creation date, so somehow we should
  // make one, if this is a new entry

  setUid( addressee->uid() );
  setBody( addressee->note() );
  setCategories( addressee->categories().join( "," ) );

  // Set creation-time and last-modification-time
  const QString creationString = addressee->custom( "KOLAB", "CreationDate" );
  kdDebug(5006) << "Creation time string: " << creationString << endl;
  QDateTime creationDate;
  if ( creationString.isEmpty() ) {
    creationDate = QDateTime::currentDateTime();
    kdDebug(5006) << "Creation date set to current time\n";
  }
  else {
    creationDate = stringToDateTime( creationString );
    kdDebug(5006) << "Creation date loaded\n";
  }
  QDateTime modified = addressee->revision();
  if ( !modified.isValid() )
    modified = QDateTime::currentDateTime();
  setLastModified( modified );
  if ( modified < creationDate ) {
    // It's not possible that the modification date is earlier than creation
    creationDate = modified;
    kdDebug(5006) << "Creation date set to modification date\n";
  }
  setCreationDate( creationDate );
  const QString newCreationDate = dateTimeToString( creationDate );
  if ( creationString != newCreationDate ) {
    // We modified the creation date, so store it for future reference
    const_cast<KABC::Addressee*>( addressee )
      ->insertCustom( "KOLAB", "CreationDate", newCreationDate );
    kdDebug(5006) << "Creation date modified. New one: " << newCreationDate << endl;
  }

  switch( addressee->secrecy().type() ) {
  case KABC::Secrecy::Private:
    setSensitivity( Private );
    break;
  case KABC::Secrecy::Confidential:
    setSensitivity( Confidential );
    break;
  default:
    setSensitivity( Public );
  }

  // TODO: Attachments
}

void KolabBase::saveTo( KABC::Addressee* addressee ) const
{
  addressee->setUid( uid() );
  addressee->setNote( body() );
  addressee->setCategories( QStringList::split( ',', categories() ) );
  addressee->setRevision( lastModified() );
  addressee->insertCustom( "KOLAB", "CreationDate",
                           dateTimeToString( creationDate() ) );

  switch( sensitivity() ) {
  case Private:
    addressee->setSecrecy( KABC::Secrecy( KABC::Secrecy::Private ) );
    break;
  case Confidential:
    addressee->setSecrecy( KABC::Secrecy( KABC::Secrecy::Confidential ) );
    break;
  default:
    addressee->setSecrecy( KABC::Secrecy( KABC::Secrecy::Public ) );
    break;
  }

  // TODO: Attachments
}

void KolabBase::setUid( const QString& uid )
{
  mUid = uid;
}

QString KolabBase::uid() const
{
  return mUid;
}

void KolabBase::setBody( const QString& body )
{
  mBody = body;
}

QString KolabBase::body() const
{
  return mBody;
}

void KolabBase::setCategories( const QString& categories )
{
  mCategories = categories;
}

QString KolabBase::categories() const
{
  return mCategories;
}

void KolabBase::setCreationDate( const QDateTime& date )
{
  mCreationDate = date;
}

QDateTime KolabBase::creationDate() const
{
  return mCreationDate;
}

void KolabBase::setLastModified( const QDateTime& date )
{
  mLastModified = date;
}

QDateTime KolabBase::lastModified() const
{
  return mLastModified;
}

void KolabBase::setSensitivity( Sensitivity sensitivity )
{
  mSensitivity = sensitivity;
}

KolabBase::Sensitivity KolabBase::sensitivity() const
{
  return mSensitivity;
}

void KolabBase::setPilotSyncId( unsigned long id )
{
  mHasPilotSyncId = true;
  mPilotSyncId = id;
}

bool KolabBase::hasPilotSyncId() const
{
  return mHasPilotSyncId;
}

unsigned long KolabBase::pilotSyncId() const
{
  return mPilotSyncId;
}

void KolabBase::setPilotSyncStatus( int status )
{
  mHasPilotSyncStatus = true;
  mPilotSyncStatus = status;
}

bool KolabBase::hasPilotSyncStatus() const
{
  return mHasPilotSyncStatus;
}

int KolabBase::pilotSyncStatus() const
{
  return mPilotSyncStatus;
}

bool KolabBase::loadEmailAttribute( QDomElement& element, Email& email )
{
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      const QString tagName = e.tagName();

      if ( tagName == "display-name" )
        email.displayName = e.text();
      else if ( tagName == "smtp-address" )
        email.smtpAddress = e.text();
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

void KolabBase::saveEmailAttribute( QDomElement& element, const Email& email,
                                    const QString& tagName ) const
{
  QDomElement e = element.ownerDocument().createElement( tagName );
  element.appendChild( e );
  writeString( e, "display-name", email.displayName );
  writeString( e, "smtp-address", email.smtpAddress );
}

bool KolabBase::loadAttribute( QDomElement& element )
{
  const QString tagName = element.tagName();
  switch ( tagName[0].latin1() ) {
  case 'u':
    if ( tagName == "uid" ) {
      setUid( element.text() );
      return true;
    }
    break;
  case 'b':
    if ( tagName == "body" ) {
      setBody( element.text() );
      return true;
    }
    break;
  case 'c':
    if ( tagName == "categories" ) {
      setCategories( element.text() );
      return true;
    }
    if ( tagName == "creation-date" ) {
      setCreationDate( stringToDateTime( element.text() ) );
      return true;
    }
    break;
  case 'l':
    if ( tagName == "last-modification-date" ) {
      setLastModified( stringToDateTime( element.text() ) );
      return true;
    }
    break;
  case 's':
    if ( tagName == "sensitivity" ) {
      setSensitivity( stringToSensitivity( element.text() ) );
      return true;
    }
    break;
  case 'p':
    if ( tagName == "product-id" )
      return true; // ignore this field
    if ( tagName == "pilot-sync-id" ) {
      setPilotSyncId( element.text().toULong() );
      return true;
    }
    if ( tagName == "pilot-sync-status" ) {
      setPilotSyncStatus( element.text().toInt() );
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

bool KolabBase::saveAttributes( QDomElement& element ) const
{
  writeString( element, "product-id", productID() );
  writeString( element, "uid", uid() );
  writeString( element, "body", body() );
  writeString( element, "categories", categories() );
  writeString( element, "creation-date", dateTimeToString( creationDate() ) );
  writeString( element, "last-modification-date",
               dateTimeToString( lastModified() ) );
  writeString( element, "sensitivity", sensitivityToString( sensitivity() ) );
  if ( hasPilotSyncId() )
    writeString( element, "pilot-sync-id", QString::number( pilotSyncId() ) );
  if ( hasPilotSyncStatus() )
    writeString( element, "pilot-sync-status", QString::number( pilotSyncStatus() ) );
  return true;
}

bool KolabBase::load( const QString& xml )
{
  // This QString is actually in utf-8, but that fails loading in the
  // XML engine, since it won't detect encoding in a QString. So we do this:
  const QString utf8 = QString::fromUtf8( xml.local8Bit() );

  QString errorMsg;
  int errorLine, errorColumn;
  QDomDocument document;
  bool ok = document.setContent( utf8, &errorMsg, &errorLine, &errorColumn );

  if ( !ok ) {
    qWarning( "Error loading document: %s, line %d, column %d",
              errorMsg.latin1(), errorLine, errorColumn );
    return false;
  }

  // XML file loaded into tree. Now parse it
  return loadXML( document );
}

bool KolabBase::load( QFile& xml )
{
  QString errorMsg;
  int errorLine, errorColumn;
  QDomDocument document;
  bool ok = document.setContent( &xml, &errorMsg, &errorLine, &errorColumn );

  if ( !ok ) {
    qWarning( "Error loading document: %s, line %d, column %d",
              errorMsg.latin1(), errorLine, errorColumn );
    return false;
  }

  // XML file loaded into tree. Now parse it
  return loadXML( document );
}

QDomDocument KolabBase::domTree()
{
  QDomDocument document;

  QString p = "version=\"1.0\" encoding=\"UTF-8\"";
  document.appendChild(document.createProcessingInstruction( "xml", p ) );

  return document;
}


QString KolabBase::dateTimeToString( const QDateTime& time )
{
  return time.toString( Qt::ISODate ) + 'Z';
}

QString KolabBase::dateToString( const QDate& date )
{
  return date.toString( Qt::ISODate );
}

QDateTime KolabBase::stringToDateTime( const QString& _date )
{
  QString date( _date );
  if ( date.endsWith( "Z" ) )
    date.truncate( date.length() - 1 );
  return QDateTime::fromString( date, Qt::ISODate );
}

QDate KolabBase::stringToDate( const QString& date )
{
  return QDate::fromString( date, Qt::ISODate );
}

QString KolabBase::sensitivityToString( Sensitivity s )
{
  switch( s ) {
  case Private: return "private";
  case Confidential: return "confidential";
  case Public: return "public";
  }

  return "What what what???";
}

KolabBase::Sensitivity KolabBase::stringToSensitivity( const QString& s )
{
  if ( s == "private" )
    return Private;
  if ( s == "confidential" )
    return Confidential;
  return Public;
}

QString KolabBase::colorToString( const QColor& color )
{
  // Color is in the format "#RRGGBB"
  return color.name();
}

QColor KolabBase::stringToColor( const QString& s )
{
  return QColor( s );
}

void KolabBase::writeString( QDomElement& element, const QString& tag,
                             const QString& tagString )
{
  if ( !tagString.isEmpty() ) {
    QDomElement e = element.ownerDocument().createElement( tag );
    QDomText t = element.ownerDocument().createTextNode( tagString );
    e.appendChild( t );
    element.appendChild( e );
  }
}

QDateTime KolabBase::localToUTC( const QDateTime& time ) const
{
  return KPimPrefs::localTimeToUtc( time, mTimeZoneId );
}

QDateTime KolabBase::utcToLocal( const QDateTime& time ) const
{
  return KPimPrefs::utcToLocalTime( time, mTimeZoneId );
}
