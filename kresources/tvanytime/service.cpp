/***************************************************************************
 *   Copyright (C) 2005 by Will Stephenson   *
 *   wstephenson@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "service.h"

Service::Service() : mActive( true )
{

}

Service::Service( bool active, const QString & name, const QString & owner, const KURL & serviceUrl, const KURL & logo, const QStringList & genres )
: mActive( active ), mName( name ), mOwner( owner ), mServiceUrl( serviceUrl ), mLogo( logo ), mGenres( genres )
{
}

void Service::setActive( bool active )
{
  mActive = active;
}

void Service::setName( const QString & name )
{
  mName = name;
}

void Service::setProgramInformation( const ProgramInformationMap & map )
{
  mProgInfo = map;
}

bool Service::active() const
{
  return mActive;
}

QString Service::name() const
{
  return mName;
}

bool Service::loadXML( const QDomElement & top )
{
  if ( top.tagName() != "ServiceInformation" ) {
    qWarning( "XML error: Top tag was %s instead of the expected service information",
              top.tagName().ascii() );
    return false;
  }

  setId( top.attribute( "serviceId" ) );

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      loadAttribute( e );
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool Service::loadAttribute( const QDomElement& element )
{
  QString tagName = element.tagName();
  if ( tagName == "Name" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mName = t.data();
  }
  else if ( tagName == "Owner" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mOwner =  t.data();
  }
  else if ( tagName == "ServiceURL" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mServiceUrl = t.data();
  }
  // TODO: parse logo data
  // TODO: parse genre data
  return true;
}

QRegExp ScheduleEvent::sRegExp( "PT(\\d{2})H(\\d{2})M(\\d{2})S" );

bool ScheduleEvent::loadXML( const QDomElement & top )
{
  if ( top.tagName() != "ScheduleEvent" ) {
    qWarning( "XML error: Top tag was %s instead of the expected event",
              top.tagName().ascii() );
    return false;
  }

  mCrid = top.attribute( "serviceId" );

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      loadAttribute( e );
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool ScheduleEvent::loadAttribute( const QDomElement& element )
{
  QString tagName = element.tagName();
  if ( tagName == "ProgramURL" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mUrl = t.data();
  }
  else if ( tagName == "Program" ) {
    mCrid = element.attribute( "crid" );
  }
  else if ( tagName == "PublishedStartTime" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mStartTime = QDateTime::fromString( t.data(), Qt::ISODate );
  }
  else if ( tagName == "PublishedDuration" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    QString duration = t.data();
    if ( sRegExp.search( duration ) != -1 )
    {
      mDuration = 0;
      mDuration += 60 * 60 * sRegExp.cap( 1 ).toUInt();
      mDuration += 60 * sRegExp.cap( 2 ).toUInt();
      mDuration += sRegExp.cap( 3 ).toUInt();
    }
  }
  return true;
}

ProgramInformationMap Service::programmeInformation() const
{
  return mProgInfo;
}

ProgramInformation::ProgramInformation( const QString & title, const QString &synopsis )
: mTitle( title ), mSynopsis( synopsis )
{

}

bool ProgramInformation::loadXML( const QDomElement & top )
{
  if ( top.tagName() != "ProgramInformation" ) {
    qWarning( "XML error: Top tag was %s instead of the expected program information",
              top.tagName().ascii() );
    return false;
  }

  setId( top.attribute( "programId" ) );

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( e.tagName() == "BasicDescription" )
      {
        for ( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          if ( n.isComment() )
            continue;
          if ( n.isElement() ) {
            QDomElement e = n.toElement();    
            loadAttribute( e );
          }
        }
      }
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool ProgramInformation::loadAttribute( const QDomElement& element )
{
  QString tagName = element.tagName();
  if ( tagName == "Title" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mTitle = t.data();
  }
  else if ( tagName == "Synopsis" ) {
    QDomNode cn = element.firstChild();
    QDomText t = cn.toText();
    mSynopsis =  t.data();
  }
  else if ( tagName == "Genre" ) {
    QDomNode name = element.firstChild();
    QDomElement nameElem = name.toElement();
    if ( nameElem.tagName() == "Name" ) {
      QDomNode cn = nameElem.firstChild();
      QDomText t = cn.toText();
      mGenres.append( t.data() );
    }
  }
  return true;
}
