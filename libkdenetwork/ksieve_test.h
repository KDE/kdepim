/*  -*- c++ -*-
    ksieve_test.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_TEST_H__
#define __KSIEVE_TEST_H__

#include "ksieve_commandbase.h"

#include <qvaluevector.h>

namespace KSieve {

  class Test : public CommandBase {
  public:
    typedef QValueVector<Test> List;

    List tests() const { return mTests; }
    void setTestList( const List & tests ) { mTests = tests; }

  protected:
    List mTests;
  };

};


#endif // __KSIEVE_TEST_H__
