/*
    This file is part of the scalix resource - based on the kolab resource.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#include "scalixbase.h"

#include <kabc/addressee.h>
#include <libkcal/journal.h>
#include <libkdepim/kpimprefs.h>
#include <kdebug.h>
#include <qfile.h>

using namespace Scalix;


ScalixBase::ScalixBase( const QString& tz )
  : mCreationDate( QDateTime::currentDateTime() ),
    mLastModified( QDateTime::currentDateTime() ),
    mSensitivity( Public ), mTimeZoneId( tz ),
    mHasPilotSyncId( false ),  mHasPilotSyncStatus( false )
{
}

ScalixBase::~ScalixBase()
{
}

void ScalixBase::setFields( const KCal::Incidence* incidence )
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

void ScalixBase::saveTo( KCal::Incidence* incidence ) const
{
  incidence->setUid( uid() );
  incidence->setDescription( body() );
  incidence->setCategories( categories() );
  incidence->setCreated( utcToLocal( creationDate() ) );
  incidence->setLastModified( utcToLocal( lastModified() ) );
  incidence->setSecrecy( sensitivity() );
  // TODO: Attachments
}

void ScalixBase::setFields( const KABC::Addressee* addressee )
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

void ScalixBase::saveTo( KABC::Addressee* addressee ) const
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

void ScalixBase::setUid( const QString& uid )
{
  mUid = uid;
}

QString ScalixBase::uid() const
{
  return mUid;
}

void ScalixBase::setBody( const QString& body )
{
  mBody = body;
}

QString ScalixBase::body() const
{
  return mBody;
}

void ScalixBase::setCategories( const QString& categories )
{
  mCategories = categories;
}

QString ScalixBase::categories() const
{
  return mCategories;
}

void ScalixBase::setCreationDate( const QDateTime& date )
{
  mCreationDate = date;
}

QDateTime ScalixBase::creationDate() const
{
  return mCreationDate;
}

void ScalixBase::setLastModified( const QDateTime& date )
{
  mLastModified = date;
}

QDateTime ScalixBase::lastModified() const
{
  return mLastModified;
}

void ScalixBase::setSensitivity( Sensitivity sensitivity )
{
  mSensitivity = sensitivity;
}

ScalixBase::Sensitivity ScalixBase::sensitivity() const
{
  return mSensitivity;
}

void ScalixBase::setPilotSyncId( unsigned long id )
{
  mHasPilotSyncId = true;
  mPilotSyncId = id;
}

bool ScalixBase::hasPilotSyncId() const
{
  return mHasPilotSyncId;
}

unsigned long ScalixBase::pilotSyncId() const
{
  return mPilotSyncId;
}

void ScalixBase::setPilotSyncStatus( int status )
{
  mHasPilotSyncStatus = true;
  mPilotSyncStatus = status;
}

bool ScalixBase::hasPilotSyncStatus() const
{
  return mHasPilotSyncStatus;
}

int ScalixBase::pilotSyncStatus() const
{
  return mPilotSyncStatus;
}

bool ScalixBase::loadEmailAttribute( QDomElement& element, Email& email )
{
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

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

void ScalixBase::saveEmailAttribute( QDomElement& element, const Email& email,
                                    const QString& tagName ) const
{
  QDomElement e = element.ownerDocument().createElement( tagName );
  element.appendChild( e );
  writeString( e, "display-name", email.displayName );
  writeString( e, "smtp-address", email.smtpAddress );
}

bool ScalixBase::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "uid" )
    setUid( element.text() );
  else if ( tagName == "body" )
    setBody( element.text() );
  else if ( tagName == "categories" )
    setCategories( element.text() );
  else if ( tagName == "creation-date" )
    setCreationDate( stringToDateTime( element.text() ) );
  else if ( tagName == "last-modification-date" )
    setLastModified( stringToDateTime( element.text() ) );
  else if ( tagName == "sensitivity" )
    setSensitivity( stringToSensitivity( element.text() ) );
  else if ( tagName == "product-id" )
    return true; // ignore this field
  else if ( tagName == "pilot-sync-id" )
    setPilotSyncId( element.text().toULong() );
  else if ( tagName == "pilot-sync-status" )
    setPilotSyncStatus( element.text().toInt() );
  else
    return false;

  // Handled here
  return true;
}

bool ScalixBase::saveAttributes( QDomElement& element ) const
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

bool ScalixBase::load( const QString& xml )
{
  QString errorMsg;
  int errorLine, errorColumn;
  QDomDocument document;
  bool ok = document.setContent( xml, &errorMsg, &errorLine, &errorColumn );

  if ( !ok ) {
    qWarning( "Error loading document: %s, line %d, column %d",
              errorMsg.latin1(), errorLine, errorColumn );
    return false;
  }

  // XML file loaded into tree. Now parse it
  return loadXML( document );
}

bool ScalixBase::load( QFile& xml )
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

QDomDocument ScalixBase::domTree()
{
  QDomDocument document;

  QString p = "version=\"1.0\" encoding=\"UTF-8\"";
  document.appendChild(document.createProcessingInstruction( "xml", p ) );

  return document;
}


QString ScalixBase::dateTimeToString( const QDateTime& time )
{
  return time.toString( Qt::ISODate ) + 'Z';
}

QString ScalixBase::dateToString( const QDate& date )
{
  return date.toString( Qt::ISODate );
}

QDateTime ScalixBase::stringToDateTime( const QString& _date )
{
  QString date( _date );
  if ( date.endsWith( "Z" ) )
    date.truncate( date.length() - 1 );
  return QDateTime::fromString( date, Qt::ISODate );
}

QDate ScalixBase::stringToDate( const QString& date )
{
  return QDate::fromString( date, Qt::ISODate );
}

QString ScalixBase::sensitivityToString( Sensitivity s )
{
  switch( s ) {
  case Private: return "private";
  case Confidential: return "confidential";
  case Public: return "public";
  }

  return "What what what???";
}

ScalixBase::Sensitivity ScalixBase::stringToSensitivity( const QString& s )
{
  if ( s == "private" )
    return Private;
  if ( s == "confidential" )
    return Confidential;
  return Public;
}

QString ScalixBase::colorToString( const QColor& color )
{
  // Color is in the format "#RRGGBB"
  return color.name();
}

QColor ScalixBase::stringToColor( const QString& s )
{
  return QColor( s );
}

void ScalixBase::writeString( QDomElement& element, const QString& tag,
                             const QString& tagString )
{
  if ( !tagString.isEmpty() ) {
    QDomElement e = element.ownerDocument().createElement( tag );
    QDomText t = element.ownerDocument().createTextNode( tagString );
    e.appendChild( t );
    element.appendChild( e );
  }
}

QDateTime ScalixBase::localToUTC( const QDateTime& time ) const
{
  return KPimPrefs::localTimeToUtc( time, mTimeZoneId );
}

QDateTime ScalixBase::utcToLocal( const QDateTime& time ) const
{
  return KPimPrefs::utcToLocalTime( time, mTimeZoneId );
}
