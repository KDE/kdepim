/*
    boolflags.h

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

#ifndef __KMIME_BOOLFLAGS_H__
#define __KMIME_BOOLFLAGS_H__

/** This class stores boolean values in single bytes.
    It provides a similar functionality as QBitArray
    but requires much less memory.
    We use it to store the flags of an article
    @internal
*/
class BoolFlags {

public:
  BoolFlags()       { clear(); }
  ~BoolFlags()      {}

  void set(unsigned int i, bool b=true);
  bool get(unsigned int i);
  void clear()            { bits[0]=0; bits[1]=0; }
  unsigned char *data()   { return bits; }

protected:
  unsigned char bits[2];  //space for 16 flags
};

#endif // __KMIME_BOOLFLAGS_H__
