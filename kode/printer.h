/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KODE_PRINTER_H
#define KODE_PRINTER_H

#include "code.h"
#include "file.h"
#include "style.h"
#include "automakefile.h"

#include <kdepimmacros.h>
#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqstringlist.h>

namespace KODE {

class KDE_EXPORT Printer
{
  public:
    Printer();
    Printer( const Style & );

    void setCreationWarning( bool );
    void setGenerator( const TQString & );
    void setOutputDirectory( const TQString & );
    void setSourceFile( const TQString & );

    void printHeader( const File & );
    void printImplementation( const File &, bool createHeaderInclude = true );
    void printAutoMakefile( const AutoMakefile & );

    TQString functionSignature( const Function &f,
      const TQString &className = TQString::null,
      bool includeClassQualifier = false );

  protected:
    TQString creationWarning();
    TQString licenseHeader( const File & );
    TQString classHeader( const Class & );
    TQString classImplementation( const Class & );
    Code functionHeaders( const Function::List &functions,
                          const TQString &className,
                          int access );

  private:
    Style mStyle;

    bool mCreationWarning;
    TQString mGenerator;
    TQString mOutputDirectory;
    TQString mSourceFile;
};

}

#endif
