/* This file has been copied from the KDE libraries and slightly modified.
   This can be removed as soon as kdelibs provides the same functionality.

   Copyright (C) 2000 Ronny Standtke <Ronny.Standtke@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "krsqueezedtextlabel.h"
#include "kstringhandler.h"
#include <tqtooltip.h>

KRSqueezedTextLabel::KRSqueezedTextLabel( const TQString &text , TQWidget *parent, const char *name )
 : TQLabel ( parent, name ) {
  setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
  fullText = text;
  squeezeTextToLabel();
}

KRSqueezedTextLabel::KRSqueezedTextLabel( TQWidget *parent, const char *name )
 : TQLabel ( parent, name ) {
  setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
}

void KRSqueezedTextLabel::resizeEvent( TQResizeEvent * ) {
  squeezeTextToLabel();
}

TQSize KRSqueezedTextLabel::minimumSizeHint() const
{
  TQSize sh = TQLabel::minimumSizeHint();
  sh.setWidth(-1);
  return sh;
}

TQSize KRSqueezedTextLabel::sizeHint() const
{
  return TQSize(contentsRect().width(), TQLabel::sizeHint().height());
}

void KRSqueezedTextLabel::setText( const TQString &text ) {
  fullText = text;
  squeezeTextToLabel();
}

void KRSqueezedTextLabel::squeezeTextToLabel() {
  TQFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(fullText);
  if (textWidth > labelWidth) {
    TQString squeezedText = KStringHandler::rPixelSqueeze(fullText, fm, labelWidth);
    TQLabel::setText(squeezedText);

    TQToolTip::remove( this );
    TQToolTip::add( this, fullText );

  } else {
    TQLabel::setText(fullText);

    TQToolTip::remove( this );
    TQToolTip::hide();

  }
}

void KRSqueezedTextLabel::setAlignment( int alignment )
{
  // save fullText and restore it
  TQString tmpFull(fullText);
  TQLabel::setAlignment(alignment);
  fullText = tmpFull;
}

#include "krsqueezedtextlabel.moc"
