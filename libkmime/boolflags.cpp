/*
    boolflags.cpp

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

#include "boolflags.h"

void BoolFlags::set(unsigned int i, bool b)
{
  if(i>15) return;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  if(b)
    bits[n] = bits[n] | p;
  else
    bits[n] = bits[n] & (255-p);
}


bool BoolFlags::get(unsigned int i)
{
  if(i>15) return false;

  unsigned char p; //bitmask
  int n;

  if(i<8) { //first byte
    p=(1 << i);
    n=0;
  }
  else { //second byte
    p=(1 << i-8);
    n=1;
  }

  return ( (bits[n] & p)>0 );
}


