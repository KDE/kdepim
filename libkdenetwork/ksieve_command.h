/*  -*- c++ -*-
    ksieve_command.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_COMMAND_H__
#define __KSIEVE_COMMAND_H__

#include "ksieve_commandbase.h"
#include "ksieve_test.h"

#include <qvaluevector.h>

namespace KSieve {

  class Command : public CommandBase {
  public:
    typedef QValueVector<Command> List;

    Test::List tests() const { return mTests; }
    void setTestList( const Test::List & tests ) { mTests = tests; }

    List block() const { return mBlock; }
    void setBlock( const List & block ) { mBlock = block; }

  protected:
    Test::List mTests;
    List mBlock;
  };

};

#endif // __KSIEVE_COMMAND_H__
