/*  -*- mode: C++; c-file-style: "gnu" -*-
    test_keygen.h

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

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

#ifndef __KLEO_TEST_KEYGEN_H__
#define __KLEO_TEST_KEYGEN_H__

#include <kdialogbase.h>

#include <qcstring.h>

namespace GpgME {
  class Error;
  class KeyGenerationResult;
}

class QLineEdit;

class KeyGenerator : public KDialogBase {
  Q_OBJECT
public:
  KeyGenerator( QWidget * parent=0, const char * name=0, WFlags f=0 );
  ~KeyGenerator();

public slots:
  void slotStartKeyGeneration();
  void slotResult( const GpgME::KeyGenerationResult & res, const QByteArray & keyData );
private:
  void showError( const GpgME::Error & err );

private:
  QLineEdit * mLineEdits[20];
};

#endif // __KLEO_TEST_KEYGEN_H__
