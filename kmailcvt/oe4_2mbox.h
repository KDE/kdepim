/***************************************************************************
                          oe4_2mbox.h  -  description
                             -------------------
    begin                : Thu Aug 24 2000
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

#include "filters.hxx"

class oe4_2mbox
{
  private:
    filter        *f;
    filterInfo    *info;
    const char    *folderIn,*folderTo;
  public:
    oe4_2mbox(const char *folderIn,const char *folderTo,filter *f,filterInfo *info);
   ~oe4_2mbox();
  public:
    int convert(void);
};

