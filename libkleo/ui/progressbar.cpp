/*
    progressbar.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "progressbar.h"

#include <QTimer>
#include "kleo_ui_debug.h"

static const int busyTimerTickInterval = 100;
static const int busyTimerTickIncrement = 5;

Kleo::ProgressBar::ProgressBar( QWidget * parent, Qt::WindowFlags f )
  : QProgressBar( parent/*, f*/ ),
    mRealProgress( -1 )
{
  Q_UNUSED(f)
  mBusyTimer = new QTimer( this );
  connect(mBusyTimer, &QTimer::timeout, this, &ProgressBar::slotBusyTimerTick);
  fixup( true );
}

void Kleo::ProgressBar::slotProgress( const QString &, int cur, int tot ) {
  setRange( cur, tot );
}

void Kleo::ProgressBar::slotProgress( const QString &, int, int cur, int tot ) {
  setRange( cur, tot );
}

void Kleo::ProgressBar::setMaximum( int total ) {
  qCDebug(KLEO_UI_LOG) <<"Kleo::ProgressBar::setMaximum(" << total <<" )";
  if ( total == maximum() )
    return;
  QProgressBar::setMaximum( 0 );
  fixup( false );
}

void Kleo::ProgressBar::setValue( int p ) {
  qCDebug(KLEO_UI_LOG) <<"Kleo::ProgressBar::setValue(" << p <<" )";
  mRealProgress = p;
  fixup( true );
}

void Kleo::ProgressBar::reset() {
  mRealProgress = -1;
  fixup( true );
}

void Kleo::ProgressBar::slotBusyTimerTick() {
  fixup( false );
  if ( mBusyTimer->isActive() )
    QProgressBar::setValue( QProgressBar::value() + busyTimerTickIncrement );
}

void Kleo::ProgressBar::fixup( bool newValue ) {
  const int cur = QProgressBar::value();
  const int tot = QProgressBar::maximum();

  qCDebug(KLEO_UI_LOG) <<"Kleo::ProgressBar::startStopBusyTimer() cur =" << cur <<"; tot =" << tot <<"; real =" << mRealProgress;

  if ( ( newValue && mRealProgress < 0 ) || ( !newValue && cur < 0 ) ) {
    qCDebug(KLEO_UI_LOG) <<"(new value) switch to reset";
    mBusyTimer->stop();
    if ( newValue )
      QProgressBar::reset();
    mRealProgress = -1;
  } else if ( tot == 0 ) {
    qCDebug(KLEO_UI_LOG) <<"(new value) switch or stay in busy";
    if ( !mBusyTimer->isActive() ) {
      mBusyTimer->start( busyTimerTickInterval );
      if ( newValue )
        QProgressBar::setValue( mRealProgress );
    }
  } else {
    qCDebug(KLEO_UI_LOG) <<"(new value) normal progress";
    mBusyTimer->stop();
    if ( QProgressBar::value() != mRealProgress )
      QProgressBar::setValue( mRealProgress );
  }
}

