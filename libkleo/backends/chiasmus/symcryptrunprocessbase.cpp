/*
    symcryptrunbackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klar�vdalens Datakonsult AB

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

#include "symcryptrunprocessbase.h"

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kshell.h>

#include <QStringList>

#include <cstring>

Kleo::SymCryptRunProcessBase::SymCryptRunProcessBase( const QString & class_, const QString & program,
                                                      const QString & keyFile, const QString & options,
                                                      Operation mode,
                                                      QObject * parent )
  : KProcess( parent ),
    mOperation( mode ), mOptions( options )
{
  *this << QLatin1String("symcryptrun")
        << QLatin1String("--class") << class_
        << QLatin1String("--program" )<< program
        << QLatin1String("--keyfile") << keyFile
        << ( mode == Encrypt ? QLatin1String("--encrypt") : QLatin1String("--decrypt") );
}

Kleo::SymCryptRunProcessBase::~SymCryptRunProcessBase() {}

bool Kleo::SymCryptRunProcessBase::launch( const QByteArray & input, bool block ) {
  connect( this, SIGNAL(readyReadStandardOutput()),
           this, SLOT(slotReadyReadStandardOutput()) );
  connect( this, SIGNAL(readyReadStandardError()),
           this, SLOT(slotReadyReadStandardError()) );
  if ( block ) {
    KTemporaryFile tempfile;
    if ( tempfile.open() )
      tempfile.write( input );
    else
      return false;
    tempfile.flush();
    *this << QLatin1String("--input") << tempfile.fileName();
    addOptions();
    if(KProcess::execute() == -2)
        return false;
  } else {
    addOptions();
    KProcess::start();
    const bool ok = waitForStarted();
    if ( !ok )
      return ok;
    mInput = input;
    write( mInput );
    closeWriteChannel();
  }
  return true;
}

void Kleo::SymCryptRunProcessBase::addOptions() {
  if ( !mOptions.isEmpty() )
  {
      const QStringList args = KShell::splitArgs( mOptions );
      *this <<QLatin1String("--") << args;
  }
}

void Kleo::SymCryptRunProcessBase::slotReadyReadStandardOutput() {
  mOutput += readAllStandardOutput();
}

void Kleo::SymCryptRunProcessBase::slotReadyReadStandardError() {
  mStderr += QLatin1String(readAllStandardError());
}

