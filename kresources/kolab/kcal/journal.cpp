/*
    This file is part of libkolabformat - the library implementing the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004  Bo Thorsen <bo@sonofthor.dk>

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

#include "journal.h"

#include <libkcal/journal.h>

using namespace KolabFormat;


Journal::Journal()
{
}

Journal::~Journal()
{
}

void Journal::setFields( KCal::Journal* journal )
{
  // Set baseclass fields
  Base::setFields( journal );

  // Set our own fields
  setSummary( journal->summary() );
  setStartDate( journal->dtStart() );
}

void Journal::setSummary( const QString& summary )
{
  mSummary = summary;
}

QString Journal::summary() const
{
  return mSummary;
}

void Journal::setStartDate( const QDateTime& startDate )
{
  mStartDate = startDate;
}

QDateTime Journal::startDate() const
{
  return mStartDate;
}

void Journal::setEndDate( const QDateTime& endDate )
{
  mEndDate = endDate;
}

QDateTime Journal::endDate() const
{
  return mEndDate;
}

bool Journal::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "summary" ) {
    setSummary( element.text() );
    return true;
  }

  // Not handled here
  return Base::loadAttribute( element );
}

bool Journal::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  Base::saveAttributes( element );

  // Save the elements
#if 0
  QDomComment c = element.ownerDocument().createComment( "Journal specific attributes" );
  element.appendChild( c );
#endif

  writeString( element, "summary", summary() );
  return true;
}


bool Journal::load( const QDomDocument& document )
{
  QDomElement top = document.documentElement();

  if ( top.tagName() != "journal" ) {
    qWarning( "XML error: Top tag was %s instead of the expected Journal",
              top.tagName().ascii() );
    return false;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // Unhandled tag - save for later storage
        ;//qDebug( "Unhandled tag: %s", e.toCString().data() );
    } else
      qDebug( "Node is not a comment or an element???" );
  }

  return true;
}

QString Journal::save() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "journal" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}
