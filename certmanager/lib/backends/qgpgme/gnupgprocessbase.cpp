/*  -*- mode: C++; c-file-style: "gnu" -*-
    gnupgprocessbase.cpp

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

#include "gnupgprocessbase.h"

#include <kdebug.h>

#include <qsocketnotifier.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

struct Kleo::GnuPGProcessBase::Private {
  Private() : useStatusFD( false ), statnot( 0 ) {
    statusFD[0] = statusFD[1] = -1;
  }

  bool useStatusFD;
  int statusFD[2];
  QSocketNotifier * statnot;
};


Kleo::GnuPGProcessBase::GnuPGProcessBase( QObject * parent, const char * name )
  : KProcess( parent, name )
{
  d = new Private();
}

Kleo::GnuPGProcessBase::~GnuPGProcessBase() {
  delete d; d = 0;
}

void Kleo::GnuPGProcessBase::setUseStatusFD( bool use ) {
  assert( d );
  d->useStatusFD = use;
}

bool Kleo::GnuPGProcessBase::start( RunMode runmode, Communication comm ) {
  if ( d->useStatusFD ) {
    // set up the status-fd. This should be in setupCommunication(),
    // but then it's too late: we need the fd of the pipe to pass it
    // as argument to the --status-fd option:
    // PENDING(marc) find out why KProcess uses both pipe() and socketpair()...
    if ( ::pipe( d->statusFD ) < 0 ) {
      kdDebug( 5150 ) << "Kleo::GnuPGProcessBase::start: pipe(2) failed: " << perror << endl;
      return false;
    }
    ::fcntl( d->statusFD[0], F_SETFD, FD_CLOEXEC );
    ::fcntl( d->statusFD[1], F_SETFD, FD_CLOEXEC );
    if ( !arguments.empty() ) {
      QValueList<QCString>::iterator it = arguments.begin();
      ++it;
      arguments.insert( it, "--status-fd" );
      char buf[25];
      sprintf( buf, "%d", d->statusFD[1] );
      arguments.insert( it, buf );
      arguments.insert( it, "--no-tty" );
      arguments.insert( it, "--charset" );
      arguments.insert( it, "utf8" );
      //arguments.insert( it, "--enable-progress-filter" ); // gpgsm doesn't know this
    }
  }
  return KProcess::start( runmode, comm );
}

int Kleo::GnuPGProcessBase::setupCommunication( Communication comm ) {
  if ( int ok = KProcess::setupCommunication( comm ) )
    return ok;
  if ( d->useStatusFD ) {
    // base class impl returned error, so close our fd's, too
    ::close( d->statusFD[0] );
    ::close( d->statusFD[1] );
    d->statusFD[0] = d->statusFD[1] = -1;
  }
  return 0; // Error
}

int Kleo::GnuPGProcessBase::commSetupDoneP() {
  if ( d->useStatusFD ) {
    ::close( d->statusFD[1] ); // close the input end of the pipe, we're the reader
    d->statnot = new QSocketNotifier( d->statusFD[0], QSocketNotifier::Read, this );
    connect( d->statnot, SIGNAL(activated(int)), SLOT(slotChildStatus(int)) );
  }
  return KProcess::commSetupDoneP();
}

int Kleo::GnuPGProcessBase::commSetupDoneC() {
  if ( d->useStatusFD )
    ::close( d->statusFD[0] ); // close the output end of the pipe, we're the writer
  return KProcess::commSetupDoneC();
}

void Kleo::GnuPGProcessBase::slotChildStatus( int fd ) {
  if ( !childStatus(fd) )
    closeStatus();
}

bool Kleo::GnuPGProcessBase::closeStatus() {
  if ( !d->useStatusFD )
    return false;
  d->useStatusFD = false;
  delete d->statnot; d->statnot = 0;
  ::close( d->statusFD[0] ); d->statusFD[0] = -1;
  return true;
}

int Kleo::GnuPGProcessBase::childStatus( int fd ) {
  char buf[1024];
  const int len = ::read( fd, buf, sizeof(buf)-1 );
  if ( len > 0 ) {
    buf[len] = 0;
    // fixme: parse it
  }
  return len;
}

void Kleo::GnuPGProcessBase::virtual_hook( int id, void * data ) {
  KProcess::virtual_hook( id, data );
}

#include "gnupgprocessbase.moc"
