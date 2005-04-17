/*
    symcryptrunbackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

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

#include "symcryptrunprocessbase.h"

#include <qtimer.h>

#include <cstring>

Kleo::SymCryptRunProcessBase::SymCryptRunProcessBase( const QString & class_, const QString & program,
                                                      const QString & keyFile, Operation mode,
                                                      QObject * parent, const char * name )
  : KProcess( parent, name ),
    mOperation( mode )
{
  *this << "symcryptrun"
        << "--class" << class_
        << "--program" << program
        << "--keyfile" << keyFile;
  if ( mode == Encrypt )
    *this << "--encrypt";
  else
    *this << "--decrypt";
}

Kleo::SymCryptRunProcessBase::~SymCryptRunProcessBase() {}

bool Kleo::SymCryptRunProcessBase::launch( const QByteArray & input, RunMode rm ) {
  mInput = input.copy();
  connect( this, SIGNAL(receivedStdout(KProcess*,char*,int)),
           this, SLOT(slotReceivedStdout(KProcess*,char*,int)) );
  connect( this, SIGNAL(receivedStderr(KProcess*,char*,int)),
           this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );
  connect( this, SIGNAL(wroteStdin(KProcess*)), this, SLOT(closeStdin()) );
  return KProcess::start( rm, All );
}

int Kleo::SymCryptRunProcessBase::commSetupDoneP() {
  // since in blocked mode, we don't go into event loop at all, we
  // need to hook ourselves into a virtual function that is called
  // from the parent after the fork(). This is it.
  if ( const int rc = KProcess::commSetupDoneP() )
    return rc;
  writeStdin( mInput.data(), mInput.size() );
  return 0;
}

void Kleo::SymCryptRunProcessBase::slotReceivedStdout( KProcess * proc, char * buf, int len ) {
  Q_ASSERT( proc == this );
  const int oldsize = mOutput.size();
  mOutput.resize( oldsize + len );
  memcpy( mOutput.data() + oldsize, buf, len );
}

void Kleo::SymCryptRunProcessBase::slotReceivedStderr( KProcess * proc, char * buf, int len ) {
  Q_ASSERT( proc == this );
  if ( len > 0 )
    mStderr += QString::fromLocal8Bit( buf, len );
}

#include "symcryptrunprocessbase.moc"
