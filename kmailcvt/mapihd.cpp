/***************************************************************************
                          mapihd.cpp  -  description
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

#include "pablib.hxx"

bool operator < (mapitag_t & a,mapitag_t & b)  { return a._order<b._order; }
bool operator > (mapitag_t & a,mapitag_t & b)  { return a._order>b._order; }
bool operator == (mapitag_t & a,mapitag_t & b) { return a._order==b._order; }

word_t
map_givenname[]=
   { pr_givenname,
          SET_MS_GIVEN_NAME,
          0
   },
map_email[]=
   { pr_email,
          SET_MS_EMAIL,
          0
   },
map_firstname[]=
   { pr_firstname,
	  SET_MS_FIRSTNAME,
          0
   },
map_lastname[]=
   { pr_lastname,
          SET_MS_LASTNAME,
          0
   },
map_additionalname[]=
   { pr_additionalname,
          SET_MS_MIDDLENAME,
          0
   },
map_title[]=
   { pr_title,
          SET_MS_TITLE,
          0
   },
map_address[]=
   { pr_address,
          SET_MS_ADDRESS,
          0
   },
map_zip[]=
   { pr_zip,
          SET_MS_ZIP,
          0
   },
map_state[]=
   { pr_state,
          SET_MS_STATE,
          0
   },
map_town[]=
   { pr_town,
          SET_MS_TOWN,
          0
   },
map_country[]=
   { pr_country,
          SET_MS_COUNTRY,
          0
   },
map_tel[]=
   { pr_tel,
          SET_MS_TEL,
          0
   },
map_mobile[]=
   { pr_mobile,
          SET_MS_MOBILE,
          0
   },
map_fax[]=
   { pr_fax,
          SET_MS_FAX,
          0
   },
map_job[]=
   { pr_job,
          HP_OPENMAIL_JOB,
          0
   },
map_organization[]=
   { pr_organization,
          SET_MS_ORGANIZATION,
          HP_OPENMAIL_ORGANIZATION,
          0
   },
map_department[]=
   { pr_department,
          SET_MS_DEPARTMENT,
          HP_OPENMAIL_DEPARTMENT,
          0
   },
map_subdep[]=
   { pr_subdep,
          HP_OPENMAIL_SUBDEP,
          0
   },
map_notes[]=
   { pr_notes,
          SET_MS_COMMENT,
          0
   },
map_notused[]=
   { pr_notused,
          HP_OPENMAIL_LOCATION_OF_WORK,           // location of work
	  SET_NOT_USED,
          0
   };


word_t *mapi_map[]={ map_givenname, map_email,
                     map_firstname, map_lastname, map_additionalname,map_title,
                     map_address,   map_town, map_zip, map_state, map_country,
                     map_tel, map_mobile, map_fax,
                     map_organization, map_department, map_subdep, map_job,
                     map_notes,
                     map_notused,
                     NULL
                   };

pabrec_entry mapitag_t::matchTag(void)
{
int          i,j;
pabrec_entry e=pr_unknown;

  for(i=0;mapi_map[i]!=NULL && e==pr_unknown;i++) {
    for(j=1;mapi_map[i][j]!=0 && _tag!=mapi_map[i][j];j++);
    if (mapi_map[i][j]!=0) {
      e=(pabrec_entry) mapi_map[i][0];
    }
  }
return e;
}

pabfields_t::pabfields_t(pabrec & R,FilterInfo *info,QWidget * /*parent*/)
{
  // Skip the first two words, because they're always the
  // same 000c 0014 ==> 0014 gives us the types, so we
  // parse from 0014 till the next offset and order the tags.

  int         mb,me;
  uint        i,k;
  content_t   _tag,_order;

  mb=R[1];
  me=R[2];

  while (mb<me) {
    _tag=R.read(mb);mb+=sizeof(_tag);
    _order=R.read(mb);mb+=sizeof(_order);

    {mapitag_t mt(_tag,_order);
     tags[tags.size()]=mt;
     context_tags[context_tags.size()]=mt;
    }
  }
  tags.sort();

  // get the right entries now

  for(i=2,k=0;i<R.N() && k<tags.size();i++,k++) {
    if (!isUsed(k)) { i-=1; }
    else {pabrec_entry e;
          QString      E;

       e=isWhat(k);
       E=R.getEntry(i);
       { QString s=E;
           s=s.stripWhiteSpace();
           E=s;
       }

       {char m[1024];
          snprintf(m, sizeof(m), "%d %d %04x %08lx %d %s %d %d",i,k,literal(k),order(k),e,E.latin1(),E[0].latin1(),E.length());
          info->log(m);
       }

       if (E!="") {

         switch (e) {
           case pr_givenname: givenName=E;
           break;
           case pr_email:     email=E;
           break;
           case pr_firstname: firstName=E;
           break;
           case pr_additionalname: additionalName=E;
           break;
           case pr_lastname:  lastName=E;
           break;
           case pr_title:     title=E;
           break;
           case pr_address:   address=E;
           break;
           case pr_town:      town=E;
           break;
           case pr_state:     state=E;
           break;
           case pr_zip:       zip=E;
           break;
           case pr_country:  country=E;
           break;
           case pr_organization: organization=E;
           break;
           case pr_department: department=E;
           break;
           case pr_subdep:     subDep=E;
           break;
           case pr_job:        job=E;
           break;
           case pr_tel:        tel=E;
           break;
           case pr_fax:        fax=E;
           break;
           case pr_modem:      modem=E;
           break;
           case pr_mobile:     mobile=E;
           break;
           case pr_url:        homepage=E;
           break;
           case pr_talk:       talk=E;
           break;
           case pr_notes:      comment=E;
           break;
           case pr_birthday:   birthday=E;
           break;
           case pr_notused:
           break;
           default:            {/*char m[250];
                                  snprintf(m,sizeof(m),"unknown tag '%x'",literal(k));
                                  info->log(m);*/
                               }
           break;
         }
      }
    }
  }

  if (firstName!="" && lastName!="") {
    givenName=lastName+", "+firstName;
  }

  // Determine if the record is ok.

  OK=true;
}

bool pabfields_t::isUsed(int k)
{
return tags[k].isUsed();
}

pabrec_entry pabfields_t::isWhat(int k)
{
return tags[k].matchTag();
}

word_t pabfields_t::literal(int k)
{
return tags[k].literal();
}

content_t pabfields_t::order(int k)
{
return tags[k].order();
}

#define C(a)  _##a=(a=="") ? NULL : ((char *) a.latin1())

void pabfields_t::get(char * & _givenName,char * &_email,
              char * & _title,char * & _firstName,char * & _additionalName,char * & _lastName,
              char * & _address,char * & _town,char * & _state,char * & _zip,char * & _country,
              char * & _organization,char * & _department,char * & _subDep,char * & _job,
              char * & _tel,char * & _fax,char * & _mobile,char * & _modem,
              char * & _homepage,char * & _talk,
              char * & _comment,char * & _birthday
             )
{
  C(givenName);
  C(email);
  C(title);
  C(firstName);
  C(additionalName);
  C(lastName);
  C(address);
  C(town);
  C(zip);
  C(state);
  C(country);
  C(organization);
  C(department);
  C(subDep);
  C(job);
  C(tel);
  C(fax);
  C(mobile);
  C(modem);
  C(homepage);
  C(talk);
  C(comment);
  C(birthday);
}




/* class pabrec {
   private:
     char   entry[1024];
     byte   *_mem;
     word_t *_N;
     word_t *_w;
   public:
     pabrec(pab *);       // expects record the begin at reading point (ftell).
    ~pabrec();
   public:
     word_t N(void)           { return _N[0]; }
     word_t operator[](int i) { return _w[i]; }
     const char *getEntry(int i);
   public:
     content_t read(word_t offset);
 };
*/

pabrec::pabrec(pab & P)
{
adr_t     A=P.tell();
content_t hdr;
word_t    offset,size,dummy;
int       i;

  hdr=P.go(A);
  offset=P.lower(hdr);

  size=offset;
  _mem=new byte[size];
  P.read(_mem,size);

  P.go(A+offset);
  P.read(m_N);
  m_W=new word_t[m_N+1];

  P.read(dummy);
  for(i=0;i<=m_N;i++) {
    P.read(m_W[i]);
  }
}

pabrec::~pabrec()
{
  delete _mem;
  delete m_W;
}


content_t pabrec::read(word_t offset)
{
content_t R;
  R=_mem[offset+3];
  R<<=8;R+=_mem[offset+2];
  R<<=8;R+=_mem[offset+1];
  R<<=8;R+=_mem[offset];
return R;
}

const char *pabrec::getEntry(int i)
{
int mb,me;
int k;
  mb=m_W[i];me=m_W[i+1];
  for(k=0;mb!=me;mb++) {
    if (_mem[mb]>=' ' || _mem[mb]=='\n' || _mem[mb]==13 || _mem[mb]=='\t') {
      if (_mem[mb]==13) { entry[k]='\n'; }
      else { entry[k]=_mem[mb]; }
      k+=1;
    }
  }
  entry[k]='\0';
return (const char *) entry;
}
