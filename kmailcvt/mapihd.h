/***************************************************************************
                          mapihd.h  -  description
                             -------------------
    begin                : Tue Jul 25 2000
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

 #ifndef __MAPIHD__
 #define __MAPIHD__

 #include "filters.hxx"
 #include "harray.hxx"
 #include <string>
 #include <stdio.h>

#include "pabtypes.h"

#define T_MS_ARRAY         0x1100               // Some sort of array
#define T_MS_STRING	 ((unsigned long) 0x1e) // definitely a string 

 typedef unsigned long adr_t;
 typedef unsigned long content_t;
 typedef unsigned short pabsize_t;
 typedef unsigned char byte_t;
 typedef unsigned short word_t;
 typedef byte_t byte;

 class pab;

 class pabrec {
   private:
     char   entry[1024];
     byte   *_mem;
     word_t _N;
     word_t *_W;
   public:
     pabrec(pab &);       // expects record the begin at reading point (ftell).
    ~pabrec();
   public:
     word_t N(void)                 { return _N; }
     word_t operator[](int i)       { return _W[i]; }
     const char *getEntry(int i);
   public:
     content_t read(word_t offset);
 };

 typedef enum {
   pr_unknown,pr_notused,
   pr_givenname,pr_email,
   pr_firstname,pr_additionalname,pr_lastname,pr_title,
   pr_address,pr_town,pr_state,pr_zip,pr_country,
   pr_organization,pr_department,pr_subdep,pr_job,
   pr_tel,pr_fax,pr_modem,pr_mobile,pr_url,pr_talk,
   pr_notes,pr_birthday
 }
 pabrec_entry;

 class mapitag_t
 {
   friend bool operator < (mapitag_t &,mapitag_t &);
   friend bool operator > (mapitag_t &,mapitag_t &);
   friend bool operator == (mapitag_t &,mapitag_t &);
   private:
     word_t        _tag;
     word_t        _type;
     content_t     _order;
   public:
     mapitag_t(content_t tag,content_t order)   { _tag=(word_t) tag;_type=(word_t) (tag>>16);_order=order; }
     mapitag_t()                                { _tag=0;_type=0;_order=0; }
   public:
     mapitag_t & operator = (mapitag_t & t)     { _tag=t._tag;_type=t._type;_order=t._order;return *this; }
   public:
     bool      isUsed(void)                     { return (_type==T_MS_STRING || (_type&T_MS_ARRAY)!=0) && _order!=0; }
     word_t    literal(void)                    { return _tag; }
     content_t order(void)                      { return _order; }
     pabrec_entry matchTag(void);
 };

 bool operator < (mapitag_t & a,mapitag_t & b);
 bool operator > (mapitag_t & a,mapitag_t & b);
 bool operator == (mapitag_t & a,mapitag_t & b);

 class pabfields_t
 {
   private:
     harray<mapitag_t> tags,context_tags;
     pabrec            *_R;
     QString givenName,email,
             title,firstName,additionalName,lastName,
             address,town,state,zip,country,
             organization,department,subDep,job,
             tel,fax,modem,mobile,homepage,talk,
             comment,birthday;
     bool               OK;
   private:
     bool          isUsed(int k);
     pabrec_entry  isWhat(int k);
     word_t        literal(int k);
     content_t     order(int k);
   public:
     pabfields_t(pabrec & R,filterInfo *info,QWidget *parent);
   public:
     void get(char * &givenName,char * &email,
              char * &title,char * &firstName,char * &additionalName,char * &lastName,
              char * &address,char * &town,char * &state,char * &zip,char * &country,
              char * &organization,char * &department,char * &subDep,char * &job,
              char * &tel,char * &fax,char * & mobile,char * &modem,
              char * &homepage,char * &talk,
              char * &comment,char * &birthday
             );
     bool isOK(void)     { return OK; }
     bool isUsable(void) { return givenName!=""; }
 };


#endif

