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
#ifndef KODE_FILE_H
#define KODE_FILE_H

#include "license.h"
#include "code.h"
#include "class.h"
#include "membervariable.h"

#include <tqstring.h>
#include <tqstringlist.h>
#include <kdepimmacros.h>

namespace KODE {

class KDE_EXPORT File
{
  public:
    File();

    void setFilename( const TQString & );
    TQString filename() const;

    void setNameSpace( const TQString & );
    TQString nameSpace() const { return mNameSpace; }

    void setProject( const TQString &project );
    TQString project() const { return mProject; }

    void addCopyright( int year, const TQString &name, const TQString &email );
    TQStringList copyrightStrings() const { return mCopyrightStrings; }

    void setLicense( const License & );
    License license() const { return mLicense; }

    void addInclude( const TQString &include );
    TQStringList includes() const { return mIncludes; }

    void insertClass( const Class & );
    Class::List classes() const { return mClasses; }

    bool hasClass( const TQString &className );

    Class findClass( const TQString &name );

    void clearClasses();
    void clearFileFunctions();
    void clearFileVariables();

    void clearCode();

    void addFileVariable( const Variable & );
    Variable::List fileVariables() const { return mFileVariables; }

    void addFileFunction( const Function & );
    Function::List fileFunctions() const { return mFileFunctions; }

    void addExternCDeclaration( const TQString & );
    TQStringList externCDeclarations() const { return mExternCDeclarations; }

    void addFileCode( const Code & );
    Code fileCode() const { return mFileCode; }

  private:
    TQString mFilename;
    TQString mNameSpace;
    TQString mProject;
    TQStringList mCopyrightStrings;
    License mLicense;
    TQStringList mIncludes;
    Class::List mClasses;
    Variable::List mFileVariables;
    Function::List mFileFunctions;
    TQStringList mExternCDeclarations;
    Code mFileCode;
};

}

#endif
