/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef ksharedfiledevice_h
#define ksharedfiledevice_h
#include <qfile.h>

/**
 * This class behaves like a QFile
 * on open it'll try to lock the file
 * via the KDED Module
 *
 * @version 0.1
 * @author Zecke
 */
class KSharedFileDevice : public QFile 
{
 public:
 
  /**
   * Simple c'tor
   */
  KSharedFileDevice( );
  
  /**
   * Simple c'tor with a filename as parameter
   * @param name The filename
   */
  KSharedFileDevice(const QString &name );
  ~KSharedFileDevice();
  
  /**
   * Try to open the file it'll also
   * lock through the KDED Module
   * @param mode the mode to open the file
   * @return failure
   * @see QFile::open
   */
  virtual bool open( int mode );
  
  /**
   * This will close and unlock the file   
   */
  virtual void close();
};
#endif





