/***************************************************************************
                          pablib.cxx  -  description
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

#include "pab_pablib.h"

#define REC_OK PAB_REC_OK


pab::pab(const char *_pabfile)
{
  pabfile=_pabfile;
  in.setName(pabfile);
  in.open(IO_ReadOnly);
  cap=i18n("Import MS Exchange Personal Address Book (.PAB)");
}


pab::~pab()
{
  if (in.isOpen()) { in.close(); }
}

//////////////////////////////////////////////////////////////////////
//
// Main conversion
//
//////////////////////////////////////////////////////////////////////

bool pab::convert(void)
{
adr_t A;
bool ret;

   if (!in.isOpen()) {QString msg;
     msg=i18n("Cannot open %1 for reading").arg(pabfile);
     // info->alert(msg);
     return false;
   }
   if (!knownPAB()) {
     return false;
   }

/*
   if (!f->openAddressBook(info)) {
     return false;
   }
*/

   A=go(INDEX_OF_INDEX);
   ret=convert(A,0,0);

   // f->closeAddressBook();

return ret;
}

bool pab::convert(adr_t A,content_t ,content_t )
{
adr_t table;
content_t start,stop,T;
int n, N = 0;

   go(A);
   T=read();

   // Now we have to decide if this is a distribution list
   // or an addressbook container. If it is the last just
   // jump directly to dotable().

   //if (upper(T)==PAB_REC_OK) {
   //  dotable(A,strt,stp);
   //  return true;
   //}

   // OK, it's not an addressbook container,
   // handle it like a distribution list

   start=T;
   while(start!=0) {
     N+=1;
     stop=read();
     table=read();
     start=read();
   }
   if (N==0) { N=1; }
/*   {char m[100];
     sprintf(m,"%d",N);
     info->alert("",m);
   }*/

   //A=go(INDEX_OF_INDEX);
   //printf("IoI=%08lx\n",A);
   go(A);
   start=read();
   n=0;
   while(start!=0) {adr_t cp;
     stop=read();
     table=read();
     cp=tell();
     dotable(table,start,stop);
     //convert(table,start,stop);
     go(cp);
     start=read();
     n+=1;
     // info->setOverall( 100 * n / N );
   }

return true;
}


void pab::dotable(adr_t T,content_t start,content_t stop)
{
adr_t REC,pREC,cp;
content_t cmp,skip;
int   N,n;

  go(T);
  cp=tell();

  REC=0xffffffff;
  pREC=0;
  cmp=read();
  if (cmp!=start) {
    // first try processing as if this was a record. I.e. at the stop thing
    processRec(stop);
    // Then exit
    // info->setCurrent();
    // info->setCurrent(100);
    return;
  }   // This is not a table.

  // info->setCurrent();
  N=0;
  while (cmp!=stop && REC!=pREC) {
    pREC=REC;
    REC=read();
    if (REC!=pREC) {
      skip=read();
      cmp=read();
    }
    N+=1;
  }

  go(cp);
  REC=0xffffffff;
  pREC=0;

  cmp=read();

  n=0;
  while(cmp!=stop && REC!=pREC) {adr_t cp;
    pREC=REC;
    REC=read();
    if (REC!=pREC) {
      skip=read();
      cp=tell();
      processRec(REC);
      go(cp);
      cmp=read();
    }
    n+=1;
    // info->setCurrent(100 * n / N);
  }

  // info->setCurrent();
  // info->setCurrent(100);
}


void pab::processRec(adr_t REC)
{
content_t hdr;

   hdr=go(REC);
   if (upper(hdr)==REC_OK) {    // Now read a record and instantiate!
     pabrec       rec(*this);
     pabfields_t  fields(rec, NULL);

     if (fields.isOK() && fields.isUsable()) {
       // f->addContact( fields.get() );
      }
   }
}

void pab::prt(unsigned char *,pabrec &,pabrec_entry )
{
}

#define PABREC_N (sizeof(pabrec)/sizeof(word_t))

void pab::rdPabRec(pabrec & )
{
}

//////////////////////////////////////////////////////////////////////
//
// Here's where we recognize the record types
//
//////////////////////////////////////////////////////////////////////

bool  pab::recUnknown(pabrec &)
{
return false;
}

bool  pab::recNoFunction(pabrec & )
{
return false;
}

const char *pab::get(unsigned char *,pabrec_entry ,pabrec & )
{
return "";
}

void pab::getrange(pabrec & ,pabrec_entry ,word_t & ,word_t & )
{
}

//////////////////////////////////////////////////////////////////////
//
// Here's where we recognize the PAB files
//
//////////////////////////////////////////////////////////////////////

bool pab::knownPAB(void)
{
content_t id;
   id=go(0);
   if (id!=PAB_FILE_ID) {QString msg;
     msg=i18n("%1 has no PAB id that I know of, cannot convert this").arg(pabfile);
     // info->alert(msg);
     return false;
   }
return true;
}


//////////////////////////////////////////////////////////////////////
//
// Functions to do file reading/positioning
//
//////////////////////////////////////////////////////////////////////

content_t pab::go(adr_t a)
{
content_t A;
  in.at(a);
  A=read();
  in.at(a);
return A;
}

content_t pab::read(void)
{
unsigned char mem[4];
content_t A;
  in.readBlock((char *) &mem, sizeof(A));	// WinTel unsigned long opslag
  A=mem[3];
  A<<=8;A|=mem[2];
  A<<=8;A|=mem[1];
  A<<=8;A|=mem[0];
return A;
}

void pab::read(word_t & w)
{
unsigned char mem[2];
  in.readBlock((char *) &mem, sizeof(w));
  w=mem[1];
  w<<=8;w|=mem[0];
}

content_t pab::relative(int words)
{
adr_t     a;
  a=in.at();
return go(a+(words*sizeof(content_t)));
}

content_t pab::add(adr_t & A,int words)
{
  A+=(words*sizeof(content_t));
return go(A);
}

pabsize_t pab::size(content_t A)
{
return A&0xFFFF;
}

word_t pab::lower(content_t A)
{
return A&0xFFFF;
}

word_t pab::upper(content_t A)
{
return A>>16;
}

void pab::size(content_t A,pabsize_t & s1,pabsize_t & s2)
{
  s1=A&0xFFFF;
  s2>>=16;
}

byte_t pab::readbyte(void)
{
byte_t c;
    c=in.getch();
return c;
}

void pab::read(unsigned char *mem,content_t size)
{
  in.readBlock((char *) mem, size);
}
