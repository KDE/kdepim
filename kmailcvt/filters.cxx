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
#include <kapplication.h>
#include <dcopclient.h>
#include <krun.h>

//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

filterInfo::filterInfo(KImportPageDlg *dlg, QWidget *parent)
{
   _dlg = dlg;
   _parent = parent;
}

filterInfo::~filterInfo()
{
}

void  filterInfo::from(const char *from)
{
  _dlg->_from->setText(from);
}

void filterInfo::from(QString from)
{
  _dlg->_from->setText(from);
}

void  filterInfo::to(const char *to)
{
  _dlg->_to->setText(to);
}

void filterInfo::to(QString to)
{
  _dlg->_to->setText(to);
}

void  filterInfo::current(const char *current)
{
  _dlg->_current->setText(current);
}

void filterInfo::current(QString current)
{
  _dlg->_current->setText(current);
}


void  filterInfo::current(float percent)
{
  int p=(int) (percent+0.5);
  if (percent<0) { _dlg->_done_current->reset(); }
  _dlg->_done_current->setProgress(p);
  kapp->processEvents(50);
}

void  filterInfo::overall(float percent)
{
  int p=(int) (percent+0.5);
  if (percent<0) { _dlg->_done_overall->reset(); }
  _dlg->_done_overall->setProgress(p);
}

void filterInfo::log(const char *toLog)
{
  _dlg->_log->insertItem(toLog);
  _dlg->_log->setCurrentItem(_dlg->_log->count()-1);
  _dlg->_log->centerCurrentItem();
  kapp->processEvents(50);
}

void filterInfo::log(QString toLog)
{
  _dlg->_log->insertItem(toLog);
  _dlg->_log->setCurrentItem(_dlg->_log->count()-1);
  _dlg->_log->centerCurrentItem();
  kapp->processEvents(50);
}

void filterInfo::clear(void)
{
  _dlg->_log->clear();
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
  info->alert(  i18n("class filter"),
		i18n("no import function implemented") );
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
  const QByteArray kmData;
  QByteArray kmRes;
  QDataStream kmArg(kmData,IO_WriteOnly);
  KURL message(msg);
  QCString type;

  kmArg << folderName << message;

  DCOPClient *c=kapp->dcopClient();
  if (!c->call("kmail", "KMailIface", "dcopAddMessage(QString,KURL)", kmData, type, kmRes)) {
    // Maybe KMail isn't already running, so try starting it
    KApplication::startServiceByDesktopName("kmail", QString::null); // Will wait until kmail is started
    if (!c->call("kmail", "KMailIface", "dcopAddMessage(QString,KURL)", kmData, type, kmRes)) {
      msg=i18n("FATAL: Unable to start KMail for DCOP communication.\n"
               "       Make sure 'kmail' is installed.");
      info->alert(cap,msg);
      return false;
    }
  }

  QDataStream KmResult(kmRes,IO_ReadOnly);
  int result;
  KmResult >> result;
    
  if (result==-1) {
    msg=i18n("Cannot make folder %1 in KMail").arg(folder);
    info->alert(cap,msg);
    return false;
  } else if (result==-2) {
    msg=i18n("Cannot add message to folder %1 in KMail").arg(folder);
    info->alert(cap,msg);
    return false;
  } else if (result==0) {
    msg=i18n("Error while adding message to folder %1 in KMail").arg(folder);
    info->alert(cap,msg);
    return false;
  }

  if (result>0) {
    added+=1;
  }

  return true;

#else

FILE *f,*msg;
QString FOLDER;
int   fh,bytes;
char  buf[4096];
//QWidget *parent=info->parent();

  FOLDER = QDir::homeDirPath() + "/Mail";

  mkdir(QFile::encodeName(FOLDER),S_IRUSR|S_IWUSR|S_IXUSR);    // This makes $HOME/Mail if itdoesn't exist
  FOLDER = FOLDER + "/" + folder;
  f=fopen(QFile::encodeName(FOLDER),"ab");
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
  info->log("kmail has adopted the (new) folders and messages");
}

//////////////////////////////////////////////////////////////////////////////////
//
// This is the kab class, it provides the interface to KAddressBook.
// This one uses kabAPI 10
//
//////////////////////////////////////////////////////////////////////////////////

kab::kab()
{
  mAddressBook = 0;
  cap=i18n("Kmailcvt - KAddressBook API");
}

kab::~kab()
{
}

bool kab::kabStart(filterInfo *info)
{
  mAddressBook = KABC::StdAddressBook::self();
  mTicket = mAddressBook->requestSaveTicket();
  if ( !mTicket ) {
     info->alert(cap,i18n("Unable to store imported data in address book."));
     return false;
  }
  return true;
}

void kab::kabStop(filterInfo *info)
{
  Q_UNUSED(info);
  mAddressBook->save( mTicket );
}

bool kab::checkStr( QString &str )
{
  if ( str == QString::null ) return false;
  str = str.stripWhiteSpace();
  if ( str.length() ) return true;
  return false;
}

bool kab::kabAddress(filterInfo *_info,QString adrbookname,
                      QString givenname, QString email,
                      QString title,QString firstname,
                      QString additionalname,QString lastname,
		      QString nickname,
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
  if ( checkStr( nickname ) ) a.setNickName( nickname );

  KABC::Address addr;
  
  addr.setId( adrbookname );
  
  if ( checkStr( town ) ) addr.setLocality( town );
  if ( checkStr( country ) ) addr.setCountry( country );
  if ( checkStr( zip ) ) addr.setPostalCode( zip );
  if ( checkStr( address ) ) addr.setLabel( address );
  a.insertAddress( addr );

  if ( checkStr( organization ) ) a.setOrganization( organization );

  if ( checkStr( department ) ) a.insertCustom("KADDRESSBOOK", "X-Department", department);
  if ( checkStr( subDep ) ) a.insertCustom("KADDRESSBOOK", "X-Office", subDep);
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
