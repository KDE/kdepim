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

class OE52MBox
{
  public:
    OE52MBox(const char *folderIn,const char *folderTo,Filter *f,FilterInfo *info);
   ~OE52MBox();

    int convert(void);

  private:
    static void addMessage(const char *, int);
    static Filter     *F;
    static const char *FOLDER;
    static FilterInfo *INFO;
    static int         numOfMessages;
    static unsigned long added, mails;
    static QString cap;

    Filter        *f;
    FilterInfo    *info;
    const char    *folderIn,*folderTo;
    QString CAP;
};

#endif
