/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _HEX_CLIPBOARD_H_
#define _HEX_CLIPBOARD_H_

#include <qstring.h> 

class CHexClipboard
{
  public:
    CHexClipboard( void );
    ~CHexClipboard( void );

    bool encode( QByteArray &dst, QByteArray &src );
    bool decode( QByteArray &dst, QString &src );

  private:
    bool plainDecode( QByteArray &dst, QString &src );
};


#endif
