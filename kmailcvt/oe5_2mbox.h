/***************************************************************************
                          oe5_2mbox.h  -  description
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

#ifndef _OE5_2MBOX_H_
#define _OE5_2MBOX_H_

#include "filters.hxx"

class oe5_2mbox
{
  private:
    static void addMessage(const char *, int);
    static filter     *F;
    static const char *FOLDER;
    static filterInfo *INFO;
    static int         numOfMessages;
    static unsigned long added, mails;
  private:
    filter        *f;
    filterInfo    *info;
    const char    *folderIn,*folderTo;
  public:
    oe5_2mbox(const char *folderIn,const char *folderTo,filter *f,filterInfo *info);
   ~oe5_2mbox();
  public:
    int convert(void);
};

#endif
