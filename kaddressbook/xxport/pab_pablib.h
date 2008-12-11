/***************************************************************************
                          pablib.hxx  -  description
                             -------------------
    begin                : Tue Jul 4 2000
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


#ifndef KADDRESSBOOK_PAB_PABLIB_H
#define KADDRESSBOOK_PAB_PABLIB_H

#include <QtCore/QFile>

#include <klocale.h>

#include "pab_mapihd.h"

#define INDEX_OF_INDEX	0x000000c4
#define PAB_REC_OK	0xbcec
#define PAB_FILE_ID 0x4e444221

class pab
{
  friend class pabrec;

  private:
    QFile in;
    const char *pabfile;
    QString     cap;
    QWidget    *parent;
  public:
    pab(const char *pabFile);
   ~pab();
  private:
    content_t  skip(int longwords) { return relative(longwords); }
    content_t  go(adr_t);
    content_t  relative(int longwords);
    content_t  relative(pabsize_t);
    content_t  add(adr_t &,int words);
    content_t  read(void);
    void       read(unsigned char *mem,content_t size);
    void       read(word_t &);
    pabsize_t  size(content_t);
    void       size(content_t,pabsize_t &, pabsize_t &);
    word_t     lower(content_t);
    word_t     upper(content_t);
    byte_t     readbyte(void);
    adr_t      curpos(void) { return in.pos(); }
    adr_t      tell(void)   { return curpos(); }
  private:
    bool  recUnknown(pabrec & R);
    bool  recNoFunction(pabrec & R);
    const char *get(unsigned char *mem,pabrec_entry e,pabrec & R);
    bool  knownPAB(void);
  private:
    void rdPabRec(pabrec & R);
    void prt(unsigned char *mem,pabrec & R,pabrec_entry E);
    void getrange(pabrec & R,pabrec_entry e,word_t & b,word_t & _e);
  private:
    bool convert(adr_t A,content_t start,content_t stop);
    void dotable(adr_t table,content_t start,content_t stop);
    void processRec(adr_t REC);
  public:
    bool convert(void);
};

#endif // KADDRESSBOOK_PAB_PABLIB_H
