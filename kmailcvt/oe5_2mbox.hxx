/***************************************************************************
                          oe5_2mbox.hxx  -  description
                             -------------------
    begin                : Mon Jul 3 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
    oe5_2mbox.hxx
    
    Copyright (C) 6-2000  Hans Dijkema 
    See README and COPYING for more information.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __OE52MBOX__
#define __OE52MBOX__

///////////////////////////////////////////////////////////////////////////
// This is a little class that converts
//
//   Outlook Express 5.0   .DBX mail boxes
// 
// to
//
//   mbox (kmail) format.
// 
///////////////////////////////////////////////////////////////////////////

#include "filters.hxx"

class oe5_2mbox 
{
  private:
    unsigned long *msgstarts;
    FILE          *in;
    filter        *f;
    filterInfo    *info;
    QWidget       *parent;
    const char    *folderIn,*folderTo;
  public:
    oe5_2mbox(const char *folderIn,const char *folderTo,filter *f,filterInfo *info);
   ~oe5_2mbox();
  public:
    int convert(void);
  public:
};

///////////////////////////////////////////////////////////////////////////
// Note! This class assumes conversion from the
// Wintel platform (So, no Outlook Express for Alpha).
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Some extra defines
///////////////////////////////////////////////////////////////////////////

#define OE5_ERR_WRONGLINK 1


#endif
