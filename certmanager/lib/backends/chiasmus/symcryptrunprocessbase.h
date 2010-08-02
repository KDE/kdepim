/*
    symcryptrunbackend.h

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

#ifndef __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__
#define __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__

#include <kprocess.h>

#include <tqcstring.h>

class TQString;

namespace Kleo {

class SymCryptRunProcessBase : public KProcess {
  Q_OBJECT
public:
  enum Operation {
    Encrypt, Decrypt
  };
  SymCryptRunProcessBase( const TQString & class_, const TQString & program,
                          const TQString & keyFile, const TQString& options,
                          Operation op,
                          TQObject * parent=0, const char * name=0 );
  ~SymCryptRunProcessBase();

  bool launch( const TQByteArray & input, RunMode rm=NotifyOnExit );

  const TQByteArray & output() const { return mOutput; }
  const TQString & stdErr() const { return mStderr; }

public slots:
  /*! upgraded to slot */
  void closeStdin() { KProcess::closeStdin(); }

private slots:
  void slotReceivedStdout( KProcess *, char *, int );
  void slotReceivedStderr( KProcess *, char *, int );

private:
  void addOptions();

  TQByteArray mInput;
  TQByteArray mOutput;
  TQString mStderr;
  const Operation mOperation;
  TQString mOptions;
};

}

#endif // __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__
