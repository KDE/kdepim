/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
*/

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qlabel.h>

#include "ksplashwidget.h"

KSplashWidget::KSplashWidget( const QString &msg, const QString &icon, QWidget *parent )
  : QHBox( parent, "KSplashWidget", Qt::WStyle_Customize | Qt::WStyle_NoBorder )
{
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth( 2 );

  setMargin( KDialog::marginHint() );
  setSpacing( KDialog::spacingHint() );

  int widthHint = 0;
  int heightHint = 0;
  if ( !icon.isEmpty() ) {
    QLabel *iconLabel = new QLabel( this );
    iconLabel->show();
    QPixmap pm = KGlobal::iconLoader()->loadIcon( icon, KIcon::Desktop );
    iconLabel->setPixmap( pm );

    widthHint = pm.width() + 10;
    heightHint = pm.height() + 10;
  }

  QLabel *label = new QLabel( msg, this );
  label->show();

  QFontMetrics fm = label->fontMetrics();
  widthHint += fm.width( msg ) + 20;
  heightHint = QMAX( heightHint, fm.height() + 10 );
  heightHint += KDialog::marginHint() * 2;

  resize( widthHint, heightHint );

  if ( parent )
    move( parent->width() / 2 - widthHint / 2, parent->height() / 2 - heightHint / 2 );

  show();

  repaint();

  qApp->processEvents( QEventLoop::ExcludeUserInput );
}

