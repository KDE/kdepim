/***************************************************************************
                          filters.hxx  -  description
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

#ifndef __FILTERS__HXX
#define __FILTERS__HXX

#ifndef HAVE_SGI_STL
  #define HAVE_SGI_STL
#endif




#include <qcombobox.h>
#include <qprogressbar.h>
#include <kabc/addressbook.h>
#include <qlistbox.h>
#include <qlabel.h>
#include "harray.hxx"
#include <string>
#include <stdio.h>
#include "kimportpagedlg.h"

class filterInfo
{
  private:
    KImportPageDlg *_dlg;
    QWidget      *_parent;
  public:
    filterInfo(KImportPageDlg *dlg, QWidget *parent);
   ~filterInfo();
  public:
    void  from(const char *from);
    void  from(QString from);
    void  to(const char *to);
    void  to(QString to);
    void  current(const char *current);
    void  current(QString current);
    void  current(float percent=-0.3);
    void  overall(float percent=-0.3);
    void  log(const char *toLog);
    void  log(QString toLog);
    void  clear(void);
    void  alert(QString c,QString m);
    QWidget *parent(void) { return _parent; }
};

class kmail
{
  private:
    QString cap;
  public:
    kmail();
   ~kmail();
  public:
    bool kmailStart(filterInfo *info) { return true; }
    bool kmailMessage(filterInfo *info,char *folder,char *msg,unsigned long & added);
    bool kmailFolder(filterInfo *info,char *folder,FILE *_folder);
    void kmailStop(filterInfo *info);
};

class kab
{
  private:
    filterInfo  *info;    // tmp var
    KABC::AddressBook *mAddressBook;
    KABC::Ticket *mTicket;
    QString       tels;
    QString       cap;
  public:
    kab();
   ~kab();
    bool kabStart(filterInfo *info);
    bool kabAddress(filterInfo *info, QString adrbookname,
                    QString givenname, QString email=QString::null,
                    QString title=QString::null,
                    QString firstName=QString::null,QString additionalName=QString::null,
                    QString lastName=QString::null, QString nickname=QString::null,
                    QString adress=QString::null,QString town=QString::null,
                    QString state=QString::null,QString zip=QString::null,
                    QString country=QString::null,
                    QString organization=QString::null,QString department=QString::null,
                    QString subDep=QString::null,QString job=QString::null,
                    QString tel=QString::null,QString fax=QString::null,
                    QString mobile=QString::null,QString modem=QString::null,
                    QString homepage=QString::null,QString talk=QString::null,
                    QString comment=QString::null,QString birthday=QString::null
                   );
    void kabStop(filterInfo *info);
  private:
    bool checkStr( QString & );
};



class filter : public kmail, public kab
{
     QString myName;
     QString myAuthor;
   public:
     filter(const char *name,const char *author);
     filter(QString name,QString author);
     filter(QString name,const char *author);
     filter(const char *name,QString author);
     virtual ~filter();
   public:
     virtual void import(filterInfo *i);
   public:
     QString name(void);
     QString author(void);
};

#endif

