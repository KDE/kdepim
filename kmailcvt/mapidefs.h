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

/* Object type */

typedef unsigned long ULONG;

#define MAPI_STORE      ((ULONG) 0x00000001)    /* Message Store */
#define MAPI_ADDRBOOK   ((ULONG) 0x00000002)    /* Address Book */
#define MAPI_FOLDER     ((ULONG) 0x00000003)    /* Folder */
#define MAPI_ABCONT     ((ULONG) 0x00000004)    /* Address Book Container */
#define MAPI_MESSAGE    ((ULONG) 0x00000005)    /* Message */
#define MAPI_MAILUSER   ((ULONG) 0x00000006)    /* Individual Recipient */
#define MAPI_ATTACH     ((ULONG) 0x00000007)    /* Attachment */
#define MAPI_DISTLIST   ((ULONG) 0x00000008)    /* Distribution List Recipient */
#define MAPI_PROFSECT   ((ULONG) 0x00000009)    /* Profile Section */
#define MAPI_STATUS     ((ULONG) 0x0000000A)    /* Status Object */
#define MAPI_SESSION    ((ULONG) 0x0000000B)    /* Session */
#define MAPI_FORMINFO   ((ULONG) 0x0000000C)    /* Form Information */

/* Property Types */

//#define MV_FLAG         0x1000          /* Multi-value flag */
#define MV_FLAG         0x1100          /* Multi-value flag */

#define PT_UNSPECIFIED  ((ULONG)  0)    /* (Reserved for interface use) type doesn't matter to caller */
#define PT_NULL         ((ULONG)  1)    /* NULL property value */
#define PT_I2           ((ULONG)  2)    /* Signed 16-bit value */
#define PT_LONG         ((ULONG)  3)    /* Signed 32-bit value */
#define PT_R4           ((ULONG)  4)    /* 4-byte floating point */
#define PT_DOUBLE       ((ULONG)  5)    /* Floating point double */
#define PT_CURRENCY     ((ULONG)  6)    /* Signed 64-bit int (decimal w/    4 digits right of decimal pt) */
#define PT_APPTIME      ((ULONG)  7)    /* Application time */
#define PT_ERROR        ((ULONG) 10)    /* 32-bit error value */
#define PT_BOOLEAN      ((ULONG) 11)    /* 16-bit boolean (non-zero true) */
#define PT_OBJECT       ((ULONG) 13)    /* Embedded object in a property */
#define PT_I8           ((ULONG) 20)    /* 8-byte signed integer */
#define PT_STRING8      ((ULONG) 30)    /* Null terminated 8-bit character string */
#define PT_UNICODE      ((ULONG) 31)    /* Null terminated Unicode string */
#define PT_SYSTIME      ((ULONG) 64)    /* FILETIME 64-bit int w/ number of 100ns periods since Jan 1,1601 */
#define PT_CLSID        ((ULONG) 72)    /* OLE GUID */
#define PT_BINARY       ((ULONG) 258)   /* Uninterpreted (counted byte array) */
/* Changes are likely to these numbers, and to their structures. */

/* Alternate property type names for ease of use */
#define PT_SHORT    PT_I2
#define PT_I4       PT_LONG
#define PT_FLOAT    PT_R4
#define PT_R8       PT_DOUBLE
#define PT_LONGLONG PT_I8



 #define MAPI_TAG(tag)         (((unsigned long) tag)&0xFFFF)
 #define PROP_TAG(type,tag)    MAPI_TAG(tag)

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
     bool      isUsed(void)                     { return (_type==PT_STRING8 || (_type&MV_FLAG)!=0) && _order!=0; }
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
     string givenName,email,
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

