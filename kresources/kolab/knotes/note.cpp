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

#include "note.h"

using namespace KolabFormat;


Note::Note()
{
}

Note::~Note()
{
}

void Note::setSummary( const QString& summary )
{
  mSummary = summary;
}

QString Note::summary() const
{
  return mSummary;
}

void Note::setBackgroundColor( const QColor& bgColor )
{
  mBackgroundColor = bgColor;
}

QColor Note::backgroundColor() const
{
  return mBackgroundColor;
}

void Note::setForegroundColor( const QColor& fgColor )
{
  mForegroundColor = fgColor;
}

QColor Note::foregroundColor() const
{
  return mForegroundColor;
}

bool Note::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName().lower();

  if ( tagName == "summary" )
    setSummary( element.text() );
  else if ( tagName == "foregroundcolor" )
    setForegroundColor( stringToColor( element.text() ) );
  else if ( tagName == "backgroundcolor" )
    setBackgroundColor( stringToColor( element.text() ) );
  else
    return Base::loadAttribute( element );

  // We handled this
  return true;
}

bool Note::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  Base::saveAttributes( element );

  // Save the elements
#if 0
  QDomComment c = element.ownerDocument().createComment( "Note specific attributes" );
  element.appendChild( c );
#endif

  writeString( element, "Summary", summary() );
  writeString( element, "ForegroundColor", colorToString( foregroundColor() ) );
  writeString( element, "BackgroundColor", colorToString( backgroundColor() ) );

  return true;
}


bool Note::load( const QDomDocument& document )
{
  QDomElement top = document.documentElement();

  if ( top.tagName().lower() != "note" ) {
    qWarning( "XML error: Top tag was %s instead of the expected Note",
              top.tagName().ascii() );
    return false;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        qDebug( "Warning: Unhandled tag %s", e.tagName().ascii() );
    } else
      qDebug( "Node is not a comment or an element???" );
  }

  return true;
}

QString Note::save() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "Note" );
  element.setAttribute( "Version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}
