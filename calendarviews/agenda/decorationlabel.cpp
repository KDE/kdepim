/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "decorationlabel.h"

#include <KToolInvocation>

#include <QMouseEvent>
#include <QResizeEvent>

using namespace EventViews;

DecorationLabel::DecorationLabel( CalendarDecoration::Element *e,
                                  QWidget *parent )
  : QLabel( parent ), mAutomaticSqueeze( true ), mDecorationElement( e ),
    mShortText( e->shortText() ), mLongText( e->longText() ),
    mExtensiveText( e->extensiveText() )
{
  mPixmap = e->newPixmap( size() );
  mUrl = e->url();
  setUrl( e->url() );

  connect( e, SIGNAL(gotNewExtensiveText(QString)),
           this, SLOT(setExtensiveText(QString)) );
  connect( e, SIGNAL(gotNewLongText(QString)),
           this, SLOT(setLongText(QString)) );
  connect( e, SIGNAL(gotNewPixmap(QPixmap)),
           this, SLOT(setPixmap(QPixmap)) );
  connect( e, SIGNAL(gotNewShortText(QString)),
           this, SLOT(setShortText(QString)) );
  connect( e, SIGNAL(gotNewUrl(KUrl)),
           this, SLOT(setUrl(KUrl)) );
  squeezeContentsToLabel();
}

DecorationLabel::DecorationLabel( const QString &shortText,
                                  const QString &longText,
                                  const QString &extensiveText,
                                  const QPixmap &pixmap,
                                  const KUrl &url,
                                  QWidget *parent )
  : QLabel( parent ), mAutomaticSqueeze( true ), mShortText( shortText ),
    mLongText( longText ), mExtensiveText( extensiveText ),
    mPixmap( pixmap )
{
  setUrl( url );

  squeezeContentsToLabel();
}

DecorationLabel::~DecorationLabel()
{
}

void DecorationLabel::mouseReleaseEvent( QMouseEvent *event )
{
  QLabel::mouseReleaseEvent( event );

  switch ( event->button() ) {
    case Qt::LeftButton:
      if ( ! mUrl.isEmpty() ) {
        KToolInvocation::invokeBrowser( mUrl.url() );
        setForegroundRole( QPalette::LinkVisited );
      }
      break;
    case Qt::MidButton:
    case Qt::RightButton:
    default:
      break;
  }
}

void DecorationLabel::resizeEvent( QResizeEvent *event )
{
  mPixmap = mDecorationElement->newPixmap( event->size() );
  QLabel::resizeEvent( event );
  squeezeContentsToLabel();
}

void DecorationLabel::setExtensiveText( const QString &text )
{
  mExtensiveText = text;
  squeezeContentsToLabel();
}

void DecorationLabel::setLongText( const QString &text )
{
  mLongText = text;
  squeezeContentsToLabel();
}

void DecorationLabel::setPixmap( const QPixmap &pixmap )
{
  mPixmap = pixmap.scaled( size(), Qt::KeepAspectRatio );
  squeezeContentsToLabel();
}

void DecorationLabel::setShortText( const QString &text )
{
  mShortText = text;
  squeezeContentsToLabel();
}

void DecorationLabel::setText( const QString &text )
{
  setLongText( text );
}

void DecorationLabel::setUrl( const KUrl &url )
{
  mUrl = url;
  QFont f = font();
  if ( url.isEmpty() ) {
    setForegroundRole( QPalette::WindowText );
    f.setUnderline( false );
#ifndef QT_NO_CURSOR
    setCursor( QCursor( Qt::ArrowCursor ) );
#endif
  } else {
    setForegroundRole( QPalette::Link );
    f.setUnderline( true );
#ifndef QT_NO_CURSOR
    setCursor( QCursor( Qt::PointingHandCursor ) );
#endif
  }
  setFont( f );
}

void DecorationLabel::squeezeContentsToLabel()
{
  if ( !mAutomaticSqueeze ) { // The content type to use has been set manually
    return;
  }

  QFontMetrics fm( fontMetrics() );

  int labelWidth = size().width();
  int longTextWidth = fm.width(mLongText);
  int extensiveTextWidth = fm.width(mExtensiveText);

  if ( ! mPixmap.isNull() ) {
    usePixmap( true );
  } else if ( ( !mExtensiveText.isEmpty() ) && ( extensiveTextWidth <= labelWidth ) ) {
    useExtensiveText( true );
  } else if ( ( !mLongText.isEmpty() ) && ( longTextWidth <= labelWidth ) ) {
    useLongText( true );
  } else {
    useShortText( true );
  }

  setAlignment( Qt::AlignCenter );
  setWordWrap( true );
  QSize msh = QLabel::minimumSizeHint();
  msh.setHeight( fontMetrics().lineSpacing() );
  msh.setWidth( 0 );
  setMinimumSize( msh );
  setSizePolicy( sizePolicy().horizontalPolicy(),
                 QSizePolicy::MinimumExpanding );
}

void DecorationLabel::useDefaultText()
{
  mAutomaticSqueeze = false;
  squeezeContentsToLabel();
}

void DecorationLabel::useExtensiveText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mExtensiveText );
  setToolTip( QString() );
}

void DecorationLabel::useLongText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mLongText );
  setToolTip( mExtensiveText.isEmpty() ? QString() : mExtensiveText );
}

void DecorationLabel::usePixmap( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setPixmap( mPixmap );
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}

void DecorationLabel::useShortText( bool allowAutomaticSqueeze )
{
  mAutomaticSqueeze = allowAutomaticSqueeze;
  QLabel::setText( mShortText );
  setToolTip( mExtensiveText.isEmpty() ? mLongText : mExtensiveText );
}

