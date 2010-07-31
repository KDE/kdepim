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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "service.h"

Service::Service() : mActive( true )
{

}

Service::Service( bool active, const TQString & name, const TQString & owner, const KURL & serviceUrl, const KURL & logo, const TQStringList & genres )
: mActive( active ), mName( name ), mOwner( owner ), mServiceUrl( serviceUrl ), mLogo( logo ), mGenres( genres )
{
}

void Service::setActive( bool active )
{
  mActive = active;
}

void Service::setName( const TQString & name )
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

TQString Service::name() const
{
  return mName;
}

bool Service::loadXML( const TQDomElement & top )
{
  if ( top.tagName() != "ServiceInformation" ) {
    qWarning( "XML error: Top tag was %s instead of the expected service information",
              top.tagName().ascii() );
    return false;
  }

  setId( top.attribute( "serviceId" ) );

  for ( TQDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      TQDomElement e = n.toElement();
      loadAttribute( e );
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool Service::loadAttribute( const TQDomElement& element )
{
  TQString tagName = element.tagName();
  if ( tagName == "Name" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mName = t.data();
  }
  else if ( tagName == "Owner" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mOwner =  t.data();
  }
  else if ( tagName == "ServiceURL" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mServiceUrl = t.data();
  }
  // TODO: parse logo data
  // TODO: parse genre data
  return true;
}

TQRegExp ScheduleEvent::sRegExp( "PT(\\d{2})H(\\d{2})M(\\d{2})S" );

bool ScheduleEvent::loadXML( const TQDomElement & top )
{
  if ( top.tagName() != "ScheduleEvent" ) {
    qWarning( "XML error: Top tag was %s instead of the expected event",
              top.tagName().ascii() );
    return false;
  }

  mCrid = top.attribute( "serviceId" );

  for ( TQDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      TQDomElement e = n.toElement();
      loadAttribute( e );
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool ScheduleEvent::loadAttribute( const TQDomElement& element )
{
  TQString tagName = element.tagName();
  if ( tagName == "ProgramURL" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mUrl = t.data();
  }
  else if ( tagName == "Program" ) {
    mCrid = element.attribute( "crid" );
  }
  else if ( tagName == "PublishedStartTime" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mStartTime = TQDateTime::fromString( t.data(), Qt::ISODate );
  }
  else if ( tagName == "PublishedDuration" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    TQString duration = t.data();
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

ProgramInformation::ProgramInformation( const TQString & title, const TQString &synopsis )
: mTitle( title ), mSynopsis( synopsis )
{

}

bool ProgramInformation::loadXML( const TQDomElement & top )
{
  if ( top.tagName() != "ProgramInformation" ) {
    qWarning( "XML error: Top tag was %s instead of the expected program information",
              top.tagName().ascii() );
    return false;
  }

  setId( top.attribute( "programId" ) );

  for ( TQDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      TQDomElement e = n.toElement();
      if ( e.tagName() == "BasicDescription" )
      {
        for ( TQDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          if ( n.isComment() )
            continue;
          if ( n.isElement() ) {
            TQDomElement e = n.toElement();    
            loadAttribute( e );
          }
        }
      }
    } else
      qWarning( "Node is not a comment or an element???" );
  }
  return true;
}

bool ProgramInformation::loadAttribute( const TQDomElement& element )
{
  TQString tagName = element.tagName();
  if ( tagName == "Title" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mTitle = t.data();
  }
  else if ( tagName == "Synopsis" ) {
    TQDomNode cn = element.firstChild();
    TQDomText t = cn.toText();
    mSynopsis =  t.data();
  }
  else if ( tagName == "Genre" ) {
    TQDomNode name = element.firstChild();
    TQDomElement nameElem = name.toElement();
    if ( nameElem.tagName() == "Name" ) {
      TQDomNode cn = nameElem.firstChild();
      TQDomText t = cn.toText();
      mGenres.append( t.data() );
    }
  }
  return true;
}
