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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KQCSTRINGSPLITTER_H
#define KQCSTRINGSPLITTER_H

#include <qcstring.h>

#include <kdemacros.h>

class KDE_EXPORT KQCStringSplitter {
  
  public:
    KQCStringSplitter();
    ~KQCStringSplitter();
        
    void reset()                  { start=0; end=0; sep=""; incSep=false;}
    
    void init(const QCString &str, const char *s);
    void init(const char *str, const char *s);
    void setIncludeSep(bool inc)  { incSep=inc; }
    
    bool first();
    bool last();
    
    bool next();
    bool prev();  
      
    QCString& string()              { return dst; }
    const QCString& source()        { return src; }
          
  private:
    QCString src, dst, sep;
    int start,end;
    bool incSep;
      
};

#endif
