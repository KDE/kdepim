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

#include "note.h"

#include <libkcal/journal.h>
#include <knotes/version.h>
#include <kdebug.h>

using namespace Kolab;


KCal::Journal* Note::xmlToJournal( const TQString& xml )
{
  Note note;
  note.load( xml );
  KCal::Journal* journal = new KCal::Journal();
  note.saveTo( journal );
  return journal;
}

TQString Note::journalToXML( KCal::Journal* journal )
{
  Note note( journal );
  return note.saveXML();
}

Note::Note( KCal::Journal* journal ) : mRichText( false )
{
  if ( journal )
    setFields( journal );
}

Note::~Note()
{
}

void Note::setSummary( const TQString& summary )
{
  mSummary = summary;
}

TQString Note::summary() const
{
  return mSummary;
}

void Note::setBackgroundColor( const TQColor& bgColor )
{
  mBackgroundColor = bgColor;
}

TQColor Note::backgroundColor() const
{
  return mBackgroundColor;
}

void Note::setForegroundColor( const TQColor& fgColor )
{
  mForegroundColor = fgColor;
}

TQColor Note::foregroundColor() const
{
  return mForegroundColor;
}

void Note::setRichText( bool richText )
{
  mRichText = richText;
}

bool Note::richText() const
{
  return mRichText;
}

bool Note::loadAttribute( TQDomElement& element )
{
  TQString tagName = element.tagName();
  if ( tagName == "summary" )
    setSummary( element.text() );
  else if ( tagName == "foreground-color" )
    setForegroundColor( stringToColor( element.text() ) );
  else if ( tagName == "background-color" )
    setBackgroundColor( stringToColor( element.text() ) );
  else if ( tagName == "knotes-richtext" )
    mRichText = ( element.text() == "true" );
  else
    return KolabBase::loadAttribute( element );

  // We handled this
  return true;
}

bool Note::saveAttributes( TQDomElement& element ) const
{
  // Save the base class elements
  KolabBase::saveAttributes( element );

  // Save the elements
#if 0
  TQDomComment c = element.ownerDocument().createComment( "Note specific attributes" );
  element.appendChild( c );
#endif

  writeString( element, "summary", summary() );
  if ( foregroundColor().isValid() )
    writeString( element, "foreground-color", colorToString( foregroundColor() ) );
  if ( backgroundColor().isValid() )
    writeString( element, "background-color", colorToString( backgroundColor() ) );
  writeString( element, "knotes-richtext", mRichText ? "true" : "false" );

  return true;
}


bool Note::loadXML( const TQDomDocument& document )
{
  TQDomElement top = document.documentElement();

  if ( top.tagName() != "note" ) {
    qWarning( "XML error: Top tag was %s instead of the expected note",
              top.tagName().ascii() );
    return false;
  }

  for ( TQDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      TQDomElement e = n.toElement();
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

TQString Note::saveXML() const
{
  TQDomDocument document = domTree();
  TQDomElement element = document.createElement( "note" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}

void Note::setFields( const KCal::Journal* journal )
{
  KolabBase::setFields( journal );

  setSummary( journal->summary() );

  TQString property = journal->customProperty( "KNotes", "BgColor" );
  if ( !property.isNull() ) {
    setBackgroundColor( property );
  } else {
    setBackgroundColor( "yellow" );
  }
  property = journal->customProperty( "KNotes", "FgColor" );
  if ( !property.isNull() ) {
    setForegroundColor( property );
  } else {
    setForegroundColor( "black" );
  }

  property = journal->customProperty( "KNotes", "RichText" );
  if ( !property.isNull() ) {
    setRichText( property == "true" ? true : false );
  } else {
    setRichText( "false" );
  }
}

void Note::saveTo( KCal::Journal* journal )
{
  KolabBase::saveTo( journal );

  // TODO: background and foreground
  journal->setSummary( summary() );
  if ( foregroundColor().isValid() )
    journal->setCustomProperty( "KNotes", "FgColor",
                                colorToString( foregroundColor() ) );
  if ( backgroundColor().isValid() )
    journal->setCustomProperty( "KNotes", "BgColor",
                                colorToString( backgroundColor() ) );
  journal->setCustomProperty( "KNotes", "RichText",
                              richText() ? "true" : "false" );
}

TQString Note::productID() const
{
  return TQString( "KNotes %1, Kolab resource" ).arg( KNOTES_VERSION );
}
