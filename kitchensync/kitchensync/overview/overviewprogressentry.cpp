/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
† † Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

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

#include <kiconloader.h>

#include <qimage.h>

#include "overviewprogressentry.h"

using namespace KSync;
using namespace KSync::OverView;

OverViewProgressEntry::OverViewProgressEntry( QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this, QBoxLayout::LeftToRight );
  layout->setSpacing( 5 );

  QWidget* spacer = new QWidget( this );
  spacer->setMinimumWidth( 5 );

  layout->addWidget( spacer, 0, AlignLeft );

  m_pixmapLabel = new QLabel( this );
  m_pixmapLabel->setFixedWidth( 16 );
  m_textLabel = new QLabel( this );
  m_progressField = new QLabel( this );

  spacer = new QWidget( this );
  spacer->setMinimumWidth( 10 );

  layout->addWidget( m_pixmapLabel, 0, AlignLeft );
  layout->addWidget( m_textLabel, 5, AlignLeft );
  layout->addStretch( 0 );
  layout->addWidget( m_progressField, 0, AlignLeft );
  layout->addWidget( spacer, 0, AlignRight );
}

OverViewProgressEntry::~OverViewProgressEntry()
{
}

void OverViewProgressEntry::setText( const QString &text )
{
  m_textLabel->setText( text );
  m_name = text;
}

void OverViewProgressEntry::setProgress( int status )
{
  // SyncStatus { SYNC_START=0, SYNC_PROGRESS=1,  SYNC_DONE=2,  SYNC_FAIL };

  if ( status == 0 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "player_play", KIcon::Desktop, 16 ) );
  } else if ( status == 1 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "reload", KIcon::Desktop, 16 ) );
  } else if ( status == 2 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "ok", KIcon::Desktop, 16 ) );
  } else {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "no", KIcon::Desktop, 16 ) );
  }
}

void OverViewProgressEntry::setPixmap( const QPixmap &pixmap )
{
  QImage test = pixmap.convertToImage();
  m_pixmapLabel->setPixmap( test.smoothScale( 16, 16, QImage::ScaleMin ) );
}

QString OverViewProgressEntry::name()
{
  return m_name;
}

#include "overviewprogressentry.moc"
