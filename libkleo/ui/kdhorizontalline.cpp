/* -*- Mode: C++ -*-
   KD Tools - a set of useful widgets for Qt
*/

/****************************************************************************
** Copyright (C) 2005 Klar�vdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/?page=products for
**   information about KD Tools Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
** In addition, as a special exception, the copyright holders give
** permission to link the code of this program with any edition of the
** Qt library by Trolltech AS, Norway (or with modified versions of Qt
** that use the same license as Qt), and distribute linked
** combinations including the two.  You must obey the GNU General
** Public License in all respects for all of the code used other than
** Qt.  If you modify this file, you may extend this exception to your
** version of the file, but you are not obligated to do so.  If you do
** not wish to do so, delete this exception statement from your
** version.
**
**********************************************************************/

#include "kdhorizontalline.h"

#include <QStyle>
#include <QPainter>
#ifdef QT_ACCESSIBILITY_SUPPORT
#include <QAccessible>
#endif
#include <QFontMetrics>
#include <QApplication>
#include <QPaintEvent>

KDHorizontalLine::KDHorizontalLine( QWidget * parent, const char * name, Qt::WindowFlags f )
  : QFrame( parent, f ),
    mAlign( Qt::AlignLeft ),
    mLenVisible( 0 )
{
  setObjectName( QLatin1String(name) );
  QFrame::setFrameStyle( HLine | Sunken );
}

KDHorizontalLine::KDHorizontalLine( const QString & title, QWidget * parent, const char * name, Qt::WindowFlags f )
  : QFrame( parent, f ),
    mAlign( Qt::AlignLeft ),
    mLenVisible( 0 )
{
  setObjectName( QLatin1String(name) );
  QFrame::setFrameStyle( HLine | Sunken );
  setTitle( title );
}

KDHorizontalLine::~KDHorizontalLine() {}

void KDHorizontalLine::setFrameStyle( int style ) {
  QFrame::setFrameStyle( ( style & ~Shape_Mask ) | HLine ); // force HLine
}

void KDHorizontalLine::setTitle( const QString & title ) {
  if ( mTitle == title )
    return;
  mTitle = title;
  calculateFrame();
  update();
  updateGeometry();
#ifdef QT_ACCESSIBILITY_SUPPORT
  QAccessible::updateAccessibility( this, 0, QAccessible::NameChanged );
#endif
}

void KDHorizontalLine::calculateFrame() {
  mLenVisible = mTitle.length();
#if 0
  if ( mLenVisible ) {
    const QFontMetrics fm = fontMetrics();
    while ( mLenVisible ) {
      const int tw = fm.width( mTitle, mLenVisible ) + 4*fm.width(QChar(' '));
      if ( tw < width() )
        break;
      mLenVisible--;
    }
    qDebug( "mLenVisible = %d (of %d)", mLenVisible, mTitle.length() );
    if ( mLenVisible ) { // but do we also have a visible label?
      QRect r = rect();
      const int va = style().styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, this );
      if( va & Qt::AlignVCenter )
        r.setTop( fm.height() / 2 );                // frame rect should be
      else if( va & Qt::AlignTop )
        r.setTop( fm.ascent() );
      setFrameRect( r );                        //   smaller than client rect
      return;
    }
  }
  // no visible label
  setFrameRect( QRect(0,0,0,0) );                //  then use client rect
#endif
}

QSizePolicy KDHorizontalLine::sizePolicy() const {
  return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

QSize KDHorizontalLine::sizeHint() const {
  return minimumSizeHint();
}

QSize KDHorizontalLine::minimumSizeHint() const {
  const int w = fontMetrics().width( mTitle, mLenVisible ) +
                fontMetrics().width( QLatin1Char( ' ' ) );
  const int h = fontMetrics().height();
  return QSize( qMax( w, indentHint() ), h ).expandedTo( qApp->globalStrut() );
}

void KDHorizontalLine::paintEvent( QPaintEvent * e ) {
  QPainter paint( this );

  if ( mLenVisible ) {        // draw title
    const QFontMetrics & fm = paint.fontMetrics();
    const int h = fm.height();
    const int tw = fm.width( mTitle, mLenVisible ) + fm.width(QLatin1Char(' '));
    int x;
    if ( mAlign & Qt::AlignHCenter )                // center alignment
      x = frameRect().width()/2 - tw/2;
    else if ( mAlign & Qt::AlignRight )        // right alignment
      x = frameRect().width() - tw;
    else if ( mAlign & Qt::AlignLeft )       // left alignment
      x = 0;
    else { // auto align
      if( QApplication::isRightToLeft() )
        x = frameRect().width() - tw;
      else
        x = 0;
    }
    QRect r( x, 0, tw, h );
    int va = style()->styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, 0, this );
    if ( va & Qt::AlignTop )
      r.translate( 0, fm.descent() );
    const QColor pen( (QRgb) style()->styleHint( QStyle::SH_GroupBox_TextLabelColor, 0, this ) );
    if ( !style()->styleHint( QStyle::SH_UnderlineShortcut, 0, this ) )
      va |= Qt::TextHideMnemonic;
    style()->drawItemText( &paint, r, Qt::TextShowMnemonic | Qt::AlignHCenter | va, palette(),
                           isEnabled(), mTitle );
    paint.setClipRegion( e->region().subtract( r ) ); // clip everything but title
  }
  drawFrame( &paint );
}

// static
int KDHorizontalLine::indentHint() {
  return 30;
}

#include "kdhorizontalline.moc"
