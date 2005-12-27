/*
    kqcstringsplitter.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KQCSTRINGSPLITTER_H
#define KQCSTRINGSPLITTER_H

#include <q3cstring.h>

#include <kdepimmacros.h>

class KDE_EXPORT KQCStringSplitter {
  
  public:
    KQCStringSplitter();
    ~KQCStringSplitter();
        
    void reset()                  { start=0; end=0; sep=""; incSep=false;}
    
    void init(const QByteArray &str, const char *s);
    void init(const char *str, const char *s);
    void setIncludeSep(bool inc)  { incSep=inc; }
    
    bool first();
    bool last();
    
    bool next();
    bool prev();  
      
    QByteArray& string()              { return dst; }
    const QByteArray& source()        { return src; }
          
  private:
    QByteArray src, dst, sep;
    int start,end;
    bool incSep;
      
};

#endif
