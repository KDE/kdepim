/*  -*- c++ -*-
    ksieve_commandbase.h

    KSieve, the KDE internet mail/usenet news message filtering library.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef __KSIEVE_COMMANDBASE_H__
#define __KSIEVE_COMMANDBASE_H__

#include "ksieve_argument.h"

#include <qstring.h>
#include <qvaluevector.h>

namespace KSieve {
  
  class CommandBase {
  public:
    QString identifier() const { return mIdentifier; }
    void setIdentifier( const QString & identifier ) { mIdentifier = identifier; }
    
    Argument::List arguments() const { return mArguments; }
    void setArgumentList( const Argument::List & args ) { mArguments = args; }

#if 0 // doesn't work: circualr dependency of CommandBase <-> Test
    Test::List tests() const { return mTests; }
    void setTestList( const Test::List & tests ) { mTests = tests; }
#endif
    
  protected:
    QString mIdentifier;
    Argument::List mArguments;
#if 0 // see above; must be replicated in both Command and Test :-(
    Test::List mTests;
#endif
  };

};

#endif // __KSIEVE_COMMANDBASE_H__
