/***************************************************************************
                          filters.cxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
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
#include "kmailcvt.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <klocale.h>

#define A(a,b)  (a=="" || b=="" || a==b)

bool operator == (AddressBook::Entry::Address & a, AddressBook::Entry::Address & b)
{
  return A(a.headline,b.headline);
/*
  return A(a.headline,b.headline) &&
         A(a.position,b.position) &&
         A(a.org,b.org) &&
         A(a.orgUnit,b.orgUnit) &&
         A(a.orgSubUnit,b.orgSubUnit) &&
         A(a.deliveryLabel,b.deliveryLabel) &&
         A(a.address,b.address) &&
         A(a.zip,b.zip) &&
         A(a.town,b.town) &&
         A(a.country,b.country) &&
         A(a.state,b.state);
*/
}

#undef A

//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

filterInfo::filterInfo(QWidget *parent,char */*name*/)
{
int w,h;
   _parent=parent;

   w=parent->width();
   h=parent->height();
   w-=20;

   h-=100;
   _log=new QListBox(parent);
   _log->setGeometry(10,h,w,90);

   h-=30;
   _done_overall=new QProgressBar(100,parent);
   _done_overall->setGeometry(10,h,w,20);
   h-=30;
   _done_current=new QProgressBar(100,parent);
   _done_current->setGeometry(10,h,w,20);

   _current=new QLabel(parent);
   _current->setText(i18n("current:"));
   h-=(_current->height()+2);
   _current->setGeometry(10,h,w,_current->height());
   _to=new QLabel(parent);
   _to->setText(i18n("to:"));
   h-=(_to->height()+2);
   _to->setGeometry(10,h,w,_to->height());
   _from=new QLabel(parent);
   _from->setText(i18n("from:"));
   h-=(_from->height()+1);
   _from->setGeometry(10,h,w,_from->height());
}

filterInfo::~filterInfo()
{
  delete _log;
  delete _from;
  delete _to;
  delete _current;
  delete _done_overall;
  delete _done_current;
}

void  filterInfo::from(const char *from)
{
  _from->setText(from);
}

void filterInfo::from(QString from)
{
  _from->setText(from);
}

void  filterInfo::to(const char *to)
{
  _to->setText(to);
}

void filterInfo::to(QString to)
{
  _to->setText(to);
}

void  filterInfo::current(const char *current)
{
  _current->setText(current);
}

void filterInfo::current(QString current)
{
  _current->setText(current);
}


void  filterInfo::current(float percent)
{
int p=(int) (percent+0.5);
  if (percent<0) { _done_current->reset(); }
  _done_current->setProgress(p);
  procEvents();
}

void  filterInfo::overall(float percent)
{
int p=(int) (percent+0.5);
  if (percent<0) { _done_overall->reset(); }
  _done_overall->setProgress(p);
}

void filterInfo::log(const char *toLog)
{
  _log->insertItem(toLog);
  _log->setCurrentItem(_log->count()-1);
  _log->centerCurrentItem();
  procEvents();
}

void filterInfo::log(QString toLog)
{
  _log->insertItem(toLog);
  _log->setCurrentItem(_log->count()-1);
  _log->centerCurrentItem();
  procEvents();
}

void filterInfo::clear(void)
{
  _log->clear();
  current();
  overall();
  current("");
  from("");
  to("");
}

void filterInfo::alert(QString conversion, QString message)
{
  QMessageBox::information(_parent,conversion,message);
}

void filterInfo::adjWidth(QWidget *q)
{
int w=_parent->width()-20;
  q->resize(w,q->height());
}

void filterInfo::adjustSize(void)
{
  adjWidth(_log);
  adjWidth(_from);
  adjWidth(_to);
  adjWidth(_current);
  adjWidth(_done_overall);
  adjWidth(_done_current);
}

//////////////////////////////////////////////////////////////////////////////////
//
// The generic filter class
//
//////////////////////////////////////////////////////////////////////////////////

filter::filter(const char *_name, const char *_author)
{
  myName=_name;
  myAuthor=_author;
}

filter::filter(QString _name, const char *_author)
{
  myName=_name;
  myAuthor=_author;
}

filter::filter(QString _name, QString _author)
{
  myName=_name;
  myAuthor=_author;
}

filter::filter(const char *_name, QString _author)
{
  myName=_name;
  myAuthor=_author;
}

filter::~filter()
{
}

QString filter::name(void)
{
return myName;
}

QString filter::author(void)
{
return myAuthor;
}

void filter::import(filterInfo *info)
{
QString c,m;
  c=i18n("class filter");
  m=i18n("no import function implemented");
  info->alert(c,m);
}

//////////////////////////////////////////////////////////////////////////////////
//
// The filters class, container for the filter class and
// keeps the filterInfo widget and the reference to the parent
//
// Makes a selectable box for choosing the right filter.
//
//////////////////////////////////////////////////////////////////////////////////

filters::filters(filterInfo *i,QWidget *_parent,char *name) : QComboBox(_parent,name)
{
  info=i;
  parent=_parent;
}

filters::~filters()
{
int i;
  for(i=0;i<F.len();i++) {
    delete F[i];
  }
}

void filters::add(filter *f)
{
  F[F.len()]=f;
  insertItem(f->name());
}

void filters::import(void)
{
  info->clear();
  F[currentItem()]->import(info);
}

QString filters::getFilters(void)
{
int i;
QString f="";
  for(i=0;i<F.len();i++) {
    f+="  - ";
    f+=F[i]->name();
    f+=" (";
    f+=F[i]->author();
    f+=") ";
    f+="\n";
  }
return f;
}

//////////////////////////////////////////////////////////////////////////////////
//
// This is the kmail class, it provides the interface to kmail.
// I'm waiting for a kmail API.
//
//////////////////////////////////////////////////////////////////////////////////

kmail::kmail()
{
  cap=i18n("KmailCvt - KMail API");
}

kmail::~kmail()
{}

bool  kmail::kmailMessage(filterInfo *info,char *folder,char *_msg,unsigned long & added)
{
#ifdef KMAILCVT_DCOP

QString folderName(folder);
QString msg(_msg);
int     result;
  result=dcopAddMessage(folderName,msg);
  if (result==-1) { QString msg;
    msg.sprintf(" '%s' ",folder);
    msg=i18n("Cannot make folder")+msg+i18n("in kmail");
    info->alert(cap,msg);
    return false;
  }
  else if (result==-2) { QString msg;
    msg.sprintf(" '%s' ",folder);
    msg=i18n("Cannot add message to folder")+msg+i18n("in kmail");
    info->alert(cap,msg);
    return false;
  }
  else if (result==-3) { QString msg;
    msg=i18n("FATAL: Couldn't start kmail for dcop communication\n"
             "       make sure 'kmail' is in your path.");
    info->alert(cap,msg);
    return false;
  }
  else if (result==0) { QString msg;
    msg.sprintf(" '%s' ",folder);
    msg=i18n("Error while adding message to folder")+msg+i18n("in kmail");
    info->alert(cap,msg);
    return false;
  }
  else if (result==1) { added+=1; }
return true;

#else

FILE *f,*msg;
char  FOLDER[1024];
int   fh,bytes;
char  buf[4096];
//QWidget *parent=info->parent();

  sprintf(FOLDER,"%s/Mail",getenv("HOME"));
  mkdir(FOLDER,S_IRUSR|S_IWUSR|S_IXUSR);    // This makes $HOME/Mail if itdoesn't exist
  sprintf(FOLDER,"%s/Mail/%s",getenv("HOME"),folder);
  f=fopen(FOLDER,"ab");
  msg=fopen(_msg,"rb");
  fseek(msg,0,SEEK_SET);
  fh=fileno(msg);
  while((bytes=read(fh,buf,4096))>0) {
    fwrite(buf,bytes,1,f);
  }
  fclose(f);
  fclose(msg);
return true;

#endif
}

void kmail::kmailStop(filterInfo *info)
{
  dcopReload();
  info->log("kmail has adopted the (new) folders and messages");
}

bool kmail::kmailFolder(filterInfo *info,char */*folder*/,FILE */*fldr*/)
{
  info->alert(cap,"kmailFolder: Not implemented yet");
return false;
//return kmailMessage(info,folder,fldr);
}

//////////////////////////////////////////////////////////////////////////////////
//
// This is the kab class, it provides the interface to K AddressBook.
// This one uses kabAPI 10
//
//////////////////////////////////////////////////////////////////////////////////

kab::kab()
{
  api=addressbook();
  cap=i18n("Kmailcvt - K Adress Book API");
}

kab::~kab()
{
  api=NULL;
}

bool kab::kabStart(filterInfo *info)
{
//QWidget *parent=info->parent();

  if (api!=NULL) { return true; }

  if (init()!=AddressBook::NoError) {
     info->alert(cap,i18n("Close down Kab to import Addressbook files!"));
     api=NULL;
     return false;
  }
  else {
     api=addressbook();
     return true;
  }
}

void kab::kabStop(filterInfo */*info*/)
{
  save(true);     // do a forced save.
}

void kab::addIfNotExists(QStringList & l,QString & E)
{
QStringList::Iterator e;
   E=E.stripWhiteSpace();
   if (E.length()==0) { return; }
   for(e=l.begin();e!=l.end() && strcasecmp((*e).latin1(),E.latin1())!=0;++e);
   if (e!=l.end()) { return; }
   for(e=l.begin();e!=l.end() && (*e).length()!=0;++e);
   if (e==l.end()) { l.append(E); }
   else { *e=E; }
}

void kab::addIfNotExists(QStringList & l,QString & E,AddressBook::Telephone _T)
{
QString T;
QStringList::Iterator e;
   E=E.stripWhiteSpace();
   if (E=="") { return; }
   for(e=l.begin();e!=l.end() && strcasecmp((*e).latin1(),E.latin1())!=0;++e);
//   {
//      info->log(*e);
//   }
   if (e==l.end()) { char m[10];
     sprintf(m,"%d",_T-1);T=m;      
     l.append(T);l.append(E); 
   }
}

#define S(a,b)   b=b.stripWhiteSpace();if (b.length()!=0) { a=b; }
#define L(a)     (a.length()!=0) 

void kab::addIfNotExists(std::list<AddressBook::Entry::Address> & l,AddressBook::Entry::Address & A)
{
std::list<AddressBook::Entry::Address>::iterator it;

   A.headline=A.headline.stripWhiteSpace();
   if (A.headline.length()==0) { 
     info->alert(cap,i18n("Unexpected: Headline of address is empty"));
     return;
   }

   //info->alert(cap,A.headline);
   it=l.begin();
   if (it!=l.end()) {
     //info->alert(cap,(*it).headline);
     for(it=l.begin();
         it!=l.end() && 
         strcasecmp((*it).headline.latin1(),A.headline.latin1())!=0;
       ++it
      );
   }
   if (it==l.end()) {AddressBook::Entry::Address & B=A;
     if ( 
          L(B.position) ||
          L(B.org) ||
          L(B.orgUnit) ||
          L(B.orgSubUnit) ||
          L(B.deliveryLabel) ||
          L(B.address) ||
          L(B.zip) ||
          L(B.town) ||
          L(B.country) ||
          L(B.state) 
        ) {
       l.insert(l.end(),A);
     }
   }
   else {AddressBook::Entry::Address B;
     B=*it;
     S(B.position,A.position);
     S(B.org,A.org);
     S(B.orgUnit,A.orgUnit);
     S(B.orgSubUnit,A.orgSubUnit);
     S(B.deliveryLabel,A.deliveryLabel);
     S(B.address,A.address);
     S(B.zip,A.zip);
     S(B.town,A.town);
     S(B.country,A.country);
     S(B.state,A.state);
     if ( 
          L(B.position) ||
          L(B.org) ||
          L(B.orgUnit) ||
          L(B.orgSubUnit) ||
          L(B.deliveryLabel) ||
          L(B.address) ||
          L(B.zip) ||
          L(B.town) ||
          L(B.country) ||
          L(B.state) 
        ) {
        *it=B;
     }
   }
}

#undef S


#define _A(a,b)   if (b!=NULL) { QString s=b; s=s.stripWhiteSpace(); if (s.length()!=0) { a=s; } }
#define _B(a,b)   if (b!=NULL) { QString s=b; s=s.stripWhiteSpace(); if (s.length()!=0) { addIfNotExists(a,s); } }
#define _T(a,b,c) if (b!=NULL) { QString s=b; s=s.stripWhiteSpace(); if (s.length()!=0) { addIfNotExists(a,s,c); } }

bool kab::kabAddress(filterInfo *_info,const char *adrbookname,
                      char *givenname, char *email,
                      char *title,char *firstName,char *additionalName,char *lastName,
                      char *address,char *town,char */*state*/,char *zip,char *country,
                      char *organization,char *department,char *subDep,char *job,
                      char *tel,char *fax,char *mobile,char *modem,
                      char *homepage,char *talk,
                      char *comment,char */*birthday*/
                     )
{
//QWidget *parent=_info->parent();
KabKey                      key;
unsigned int                i,N;
bool                        found;
AddressBook::Entry          e;
AddressBook::Entry::Address A;

  // initialize

  info=_info;
  N=api->noOfEntries();

  // first check if givenname already exists...

  i=0;
  found=false;
  while(i<N && !found) {
    api->getKey(i,key);
    api->getEntry(key,e);

    found=strcasecmp(e.fn.latin1(),givenname)==0;

    if (!found) { i+=1; }
  }

  // If not add it.

  if (!found) {AddressBook::Entry empty;
    //info->alert(cap,i18n("not found"));
    api->add(empty,key);
    api->getEntry(key,e);
  }

  // Now we've got a valid addressbook entry, fill it up.

  e.fn=givenname;

  _B(e.emails,email)

  _A(e.title,title);
  _A(e.firstname,firstName);
  _A(e.middlename,additionalName);
  _A(e.lastname,lastName);

  _A(A.address,address);
  _A(A.town,town);
  _A(A.country,country);
  _A(A.zip,zip);
  _A(A.org,organization);
  _A(A.orgUnit,department);
  _A(A.orgSubUnit,subDep);
  _A(A.position,job);

  _T(e.telephone,tel,AddressBook::Fixed);
  _T(e.telephone,fax,AddressBook::Fax);
  _T(e.telephone,mobile,AddressBook::Mobile);
  _T(e.telephone,modem,AddressBook::Modem);

  _B(e.URLs,homepage);
  _B(e.talk,talk);

  _A(e.comment,comment);

  // Change the entry at the key.

  A.headline=adrbookname;
  addIfNotExists(e.addresses,A);

  api->change(key,e);

return true;
}

#undef _A
#undef _B
#undef _T


