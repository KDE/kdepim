/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
#include <libkcal/journal.h>
#include <qfile.h>

using namespace Kolab;


KolabBase::KolabBase()
  : mCreationDate( QDateTime::currentDateTime() ),
    mLastModified( QDateTime::currentDateTime() ),
    mSensitivity( Public )
{
}

KolabBase::~KolabBase()
{
}

void KolabBase::setFields( const KCal::Incidence* incidence )
{
  setUid( incidence->uid() );
  setBody( incidence->description() );
  setCategories( incidence->categoriesStr() );
  setCreationDate( incidence->created() );
  setLastModified( incidence->lastModified() );
  setSensitivity( static_cast<Sensitivity>( incidence->secrecy() ) );
  // TODO: Attachments
}

void KolabBase::saveTo( KCal::Incidence* incidence ) const
{
  incidence->setUid( uid() );
  incidence->setDescription( body() );
  incidence->setCategories( categories() );
  incidence->setCreated( creationDate() );
  incidence->setLastModified( lastModified() );
  incidence->setSecrecy( sensitivity() );
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

bool KolabBase::loadAttribute( QDomElement& element )
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
  else
    return false;

  // Handled here
  return true;
}

bool KolabBase::saveAttributes( QDomElement& element ) const
{
  writeString( element, "uid", uid() );
  writeString( element, "body", body() );
  writeString( element, "categories", categories() );
  writeString( element, "creation-date", dateTimeToString( creationDate() ) );
  writeString( element, "last-modification-date",
               dateTimeToString( lastModified() ) );
  writeString( element, "sensitivity", sensitivityToString( sensitivity() ) );

  return true;
}

bool KolabBase::load( const QString& xml )
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
  return time.toString( Qt::ISODate );
}

QString KolabBase::dateToString( const QDate& date )
{
  return date.toString( Qt::ISODate );
}

QDateTime KolabBase::stringToDateTime( const QString& date )
{
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
  QDomElement e = element.ownerDocument().createElement( tag );
  QDomText t = element.ownerDocument().createTextNode( tagString );
  e.appendChild( t );
  element.appendChild( e );
}
