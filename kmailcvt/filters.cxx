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

#include <qlayout.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kabc/stdaddressbook.h>

//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

filterInfo::filterInfo(QWidget *parent,char */*name*/)
: QWidget( parent )
{
  QGridLayout *grid1 = new QGridLayout(this,20,4,15,7);
   _parent=parent;

   _log=new QListBox(this);
   grid1->addMultiCellWidget(_log,5,19,0,4);
   _done_overall=new QProgressBar(100,this);
   grid1->addMultiCellWidget(_done_overall,4,4,0,4);
   _done_current=new QProgressBar(100,this);
   grid1->addMultiCellWidget(_done_current,3,3,0,4);

   _current=new QLabel(this);
   _current->setText(i18n("Current:"));
   grid1->addMultiCellWidget(_current,2,2,0,4);
   _to=new QLabel(this);
   _to->setText(i18n("Destination:"));
      grid1->addMultiCellWidget(_to,0,0,0,4);
   _from=new QLabel(this);
   _from->setText(i18n("Source:"));
   grid1->addMultiCellWidget(_from,1,1,0,4);
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
  KMessageBox::information(_parent,message,conversion);
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
    msg=i18n("Cannot make folder %1 in kmail").arg(folder);
    info->alert(cap,msg);
    return false;
  }
  else if (result==-2) { QString msg;
    msg.sprintf(" '%s' ",folder);
    msg=i18n("Cannot add message to folder %1 in kmail").arg(folder);
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
    msg=i18n("Error while adding message to folder %1 in kmail").arg(folder);
    info->alert(cap,msg);
    return false;
  }

  if (result>0) { added+=1; }//fprintf(stderr,"added+1\n"); }

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
  mAddressBook = 0;
  cap=i18n("Kmailcvt - K Address Book API");
}

kab::~kab()
{
}

bool kab::kabStart(filterInfo *info)
{
  mAddressBook = KABC::StdAddressBook::self();
  mTicket = mAddressBook->requestSaveTicket();
  if ( !mTicket ) {
     info->alert(cap,i18n("Close down Kab to import Addressbook files!"));
     return false;
  } else {
     return true;
  }
}

void kab::kabStop(filterInfo */*info*/)
{
  mAddressBook->save( mTicket );
}

bool kab::checkStr( QString &str )
{
  if ( str == KAB_NIL ) return false;
  str = str.stripWhiteSpace();
  if ( str.length() ) return true;
  return false;
}

bool kab::kabAddress(filterInfo *_info,QString adrbookname,
                      QString givenname, QString email,
                      QString title,QString firstname,
                      QString additionalname,QString lastname,
                      QString address,QString town,
                      QString /*state*/,QString zip,QString country,
                      QString organization,QString department,
                      QString subDep,QString job,
                      QString tel,QString fax,QString mobile,QString modem,
                      QString homepage,QString talk,
                      QString comment,QString /*birthday*/
                     )
{
  // initialize

  info=_info;

  // first check if givenname already exists...

  KABC::Addressee a;

  QString note;

  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    if ( givenname == (*it).formattedName() ) {
      a = *it;
      break;
    }
  }

  // Now we've got a valid addressbook entry, fill it up.

  a.setFormattedName( givenname );

  if ( checkStr( email ) ) a.insertEmail( email );

  if ( checkStr( title ) ) a.setTitle( title );
  if ( checkStr( firstname ) ) a.setGivenName ( firstname );
  if ( checkStr( additionalname ) ) a.setAdditionalName( additionalname );
  if ( checkStr( lastname ) ) a.setFamilyName( lastname );

  KABC::Address addr;
  
  addr.setId( adrbookname );
  
  if ( checkStr( town ) ) addr.setLocality( town );
  if ( checkStr( country ) ) addr.setCountry( country );
  if ( checkStr( zip ) ) addr.setPostalCode( zip );
  if ( checkStr( address ) ) addr.setLabel( address );
  a.insertAddress( addr );

  if ( checkStr( organization ) ) a.setOrganization( organization );

  if ( checkStr( department ) ) note.append( "Department: " + department + "\n" );
  if ( checkStr( subDep ) ) note.append( "Sub Department: " + subDep + "\n" );
  if ( checkStr( job ) ) note.append( job + "\n" );

  if ( checkStr( tel ) )
    a.insertPhoneNumber( KABC::PhoneNumber( tel, KABC::PhoneNumber::Voice ) );
  if ( checkStr( fax ) )
    a.insertPhoneNumber( KABC::PhoneNumber( fax, KABC::PhoneNumber::Fax ) );
  if ( checkStr( mobile ) )
    a.insertPhoneNumber( KABC::PhoneNumber( mobile, KABC::PhoneNumber::Cell ) );
  if ( checkStr( modem ) )
    a.insertPhoneNumber( KABC::PhoneNumber( modem, KABC::PhoneNumber::Modem ) );

  if ( checkStr( homepage ) ) a.setUrl( KURL( homepage ) );
  if ( checkStr( talk ) ) note.append( "Talk: " + talk + "\n" );

  if ( checkStr( comment ) ) note.append( comment );

  a.setNote( note );

  mAddressBook->insertAddressee( a );

  a.dump();

  return true;
}
