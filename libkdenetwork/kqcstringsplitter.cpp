/*
    kqcstringsplitter.cpp

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

#include "kqcstringsplitter.h"

KQCStringSplitter::KQCStringSplitter()
{
  reset();
}




KQCStringSplitter::~KQCStringSplitter()
{
}



void KQCStringSplitter::init(const QCString &str, const char *s)
{
  sep=s;
  src=str;
}



void KQCStringSplitter::init(const char *str, const char *s)
{
  sep=s;
  src=str;
}

bool KQCStringSplitter::first()
{
  /*int plus;
  if(incSep) plus=sep.length();
  else plus=0;  */
  
  start=0;
  
  end=src.find(sep, start);
  
  if(end!=-1) {
    dst=src.mid(start, end);
    return true;
  }
  else {
    start=src.length();
    end=start;
    return false;
  }
    
}



bool KQCStringSplitter::last()
{
  /*int startplus, endplus;
  
  if(incSep) {
    startplus=0;
    endplus=sep.length();
  }
  else {
    startplus=sep.length();
    endplus=0;
  }*/
    
  end=src.length();
    
  start=src.findRev(sep,end);
  
  if(start!=-1) {
    dst=src.mid(start, end-start);
    return true;
  }
  else return false;
  

}



bool KQCStringSplitter::next()
{
  /*int plus;
  if(incSep) plus=sep.length();
  else plus=0;*/
  
  start=end+1;
  
  if(start< (int) src.length()) {
    
    end=src.find(sep, start);
    
    if(end!=-1) {
      dst=src.mid(start, end-start);
    }
    else {
      dst=src.mid(start, src.length()-start);
      start=src.length();
      end=src.length();
    }
    
    return true;
  }
  else return false;
  
}



bool KQCStringSplitter::prev()
{
  /*int startplus, endplus;
  
  if(incSep) {
    startplus=0;
    endplus=sep.length();
  }
  else {
    startplus=sep.length();
    endplus=0;
  }*/
  
  end=start-1;
  
  if(end>0) {
    
    start=src.findRev(sep,end);
    
    if(start!=-1)
      dst=src.mid(start, end-start);
    
    else {
      dst=src.mid(0, end+1);
      end=0;
      start=0;
    }
  
    return true;
  }
  else return false;

}

