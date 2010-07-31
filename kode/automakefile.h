/*
    This file is part of KDE.

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
#ifndef KODE_AUTOMAKEFILE_H
#define KODE_AUTOMAKEFILE_H

#include <tqvaluelist.h>
#include <tqstring.h>
#include <tqmap.h>
#include <tqstringlist.h>

#include <kdepimmacros.h>

namespace KODE {

class KDE_EXPORT AutoMakefile
{
  public:
    class KDE_EXPORT Target
    {
      public:
        typedef TQValueList<Target> List;

        Target() {}
        Target( const TQString &type, const TQString &name );

        void setType( const TQString &type ) { mType = type; }
        TQString type() const { return mType; }

        void setName( const TQString &name ) { mName = name; }
        TQString name() const { return mName; }

        void setSources( const TQString &sources ) { mSources = sources; }
        TQString sources() const { return mSources; }

        void setLibAdd( const TQString &libAdd ) { mLibAdd = libAdd; }
        TQString libAdd() const { return mLibAdd; }

        void setLdAdd( const TQString &ldAdd ) { mLdAdd = ldAdd; }
        TQString ldAdd() const { return mLdAdd; }

        void setLdFlags( const TQString &ldFlags ) { mLdFlags = ldFlags; }
        TQString ldFlags() const { return mLdFlags; }

      private:
        TQString mType;
        TQString mName;

        TQString mSources;
        TQString mLibAdd;
        TQString mLdAdd;
        TQString mLdFlags;
    };
  
    AutoMakefile();
  
    void addTarget( const Target &t );
    Target::List targets() const { return mTargets; }

    void addEntry( const TQString &variable,
                   const TQString &value = TQString::null );

    void newLine();

    TQString text() const;

  private:
    Target::List mTargets;
    TQStringList mTargetTypes;

    TQStringList mEntries;
    TQMap<TQString,TQString> mValues;
};

}

#endif
