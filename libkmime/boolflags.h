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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMIME_BOOLFLAGS_H__
#define __KMIME_BOOLFLAGS_H__

#include <kdepimmacros.h>

/** This class stores boolean values in single bytes.
    It provides a similar functionality as QBitArray
    but requires much less memory.
    We use it to store the flags of an article
    @internal
*/
class KDE_EXPORT BoolFlags {

public:
  BoolFlags()       { clear(); }
  ~BoolFlags()      {}

  /** Sets bit number @p i to the value @p b.
      @param i bit number. Valid values are 0 through 15.
             Higher values will be silently ignored.
      @param b value to set bit @p i to.
  */
  void set(unsigned int i, bool b=true);
  /** Get bit number @p i.
      @param i bit number. Valid values are 0 through 15.
	     Higher values all return @c false.
      @return Value of the single bit @p i. Invalid bit
	      numbers return @c false.
  */
  bool get(unsigned int i);
  /** Set all bits to false. */
  void clear()            { bits[0]=0; bits[1]=0; }
  /** Returns a pointer to the data structure used to store the bits. */
  unsigned char *data()   { return bits; }

protected:
  /** Two bytes (at least) of storage for the bits. */
  unsigned char bits[2];  //space for 16 flags
};

#endif // __KMIME_BOOLFLAGS_H__
