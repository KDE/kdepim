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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include "config.h"
#include "progressbar.h"

#include <qtimer.h>
#include <kdebug.h>

static const int busyTimerTickInterval = 100;
static const int busyTimerTickIncrement = 5;

Kleo::ProgressBar::ProgressBar( QWidget * parent, const char * name, WFlags f )
  : QProgressBar( 0, parent, name, f ),
    mRealProgress( -1 )
{
  mBusyTimer = new QTimer( this );
  connect( mBusyTimer, SIGNAL(timeout()), SLOT(slotBusyTimerTick()) );
  fixup( true );
}

void Kleo::ProgressBar::slotProgress( const QString &, int cur, int tot ) {
  setProgress( cur, tot );
}

void Kleo::ProgressBar::slotProgress( const QString &, int, int cur, int tot ) {
  setProgress( cur, tot );
}

void Kleo::ProgressBar::setTotalSteps( int total ) {
  kdDebug() << "Kleo::ProgressBar::setTotalSteps( " << total << " )" << endl;
  if ( total == totalSteps() )
    return;
  QProgressBar::setTotalSteps( 0 );
  fixup( false );
}

void Kleo::ProgressBar::setProgress( int p ) {
  kdDebug() << "Kleo::ProgressBar::setProgress( " << p << " )" << endl;
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
    QProgressBar::setProgress( QProgressBar::progress() + busyTimerTickIncrement );
}

void Kleo::ProgressBar::fixup( bool newValue ) {
  const int cur = QProgressBar::progress();
  const int tot = QProgressBar::totalSteps();

  kdDebug() << "Kleo::ProgressBar::startStopBusyTimer() cur = " << cur << "; tot = " << tot << "; real = " << mRealProgress << endl;

  if ( ( newValue && mRealProgress < 0 ) || ( !newValue && cur < 0 ) ) {
    kdDebug() << "(new value) switch to reset" << endl;
    mBusyTimer->stop();
    if ( newValue )
      QProgressBar::reset();
    mRealProgress = -1;
  } else if ( tot == 0 ) {
    kdDebug() << "(new value) switch or stay in busy" << endl;
    if ( !mBusyTimer->isActive() ) {
      mBusyTimer->start( busyTimerTickInterval );
      if ( newValue )
	QProgressBar::setProgress( mRealProgress );
    }
  } else {
    kdDebug() << "(new value) normal progress" << endl;
    mBusyTimer->stop();
    if ( QProgressBar::progress() != mRealProgress )
      QProgressBar::setProgress( mRealProgress );
  }
}

#include "progressbar.moc"
