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
#include <qlabel.h>
#include <qprogressbar.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <kabapi.h>

#include "harray.hxx"
#include <string>
#include <stdio.h>

class filterInfo
{
  private:
    QWidget      *_parent;
  private:
    QListBox     *_log;
    QLabel       *_from;
    QLabel       *_to;
    QLabel       *_current;
    QProgressBar *_done_current;
    QProgressBar *_done_overall;
  public:
    filterInfo(QWidget *parent=0, char *name=0);
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
  public:
    void  alert(QString c,QString m);
  public:
    QWidget *parent(void) { return _parent; }
  public:
    void  adjustSize(void);
  private:
    void  adjWidth(QWidget *);
};


class kmail
{
  private:
    QString cap;
  public:
    kmail();
   ~kmail();
  public:
    bool kmailStart(filterInfo */*info*/) { return true; }
    bool kmailMessage(filterInfo *info,char *folder,char *msg,unsigned long & added);
    bool kmailFolder(filterInfo *info,char *folder,FILE *folder);
    void kmailStop(filterInfo *info);
};

class kab : public KabAPI
{
  private:
    filterInfo  *info;    // tmp var
    AddressBook *api;
    QString       tels;
    QString       cap;
  public:
    kab();
   ~kab();
  private:
    void addIfNotExists(QStringList & l,QString & E);
    void addIfNotExists(QStringList & l,QString & E,AddressBook::Telephone);
    void addIfNotExists(std::list<AddressBook::Entry::Address> & l,AddressBook::Entry::Address & A);
  public:
    bool kabStart(filterInfo *info);
    bool kabAddress(filterInfo *info, const char *adrbookname,
                      char *givenname, char *email=NULL,
                      char *title=NULL,char *firstName=NULL,char *additionalName=NULL,char *lastName=NULL,
                      char *adress=NULL,char *town=NULL,char *state=NULL,char *zip=NULL,char *country=NULL,
                      char *organization=NULL,char *department=NULL,char *subDep=NULL,char *job=NULL,
                      char *tel=NULL,char *fax=NULL,char *mobile=NULL,char *modem=NULL,
                      char *homepage=NULL,char *talk=NULL,
                      char *comment=NULL,char *birthday=NULL
                     );
    void kabStop(filterInfo *info);
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

class filters : public QComboBox
{
   private:
     filterInfo *info;
     harray<filter *> F;
     QWidget *parent;
   public:
     filters(filterInfo *i,QWidget *parent=0,char *name=0);
    ~filters();
   public:
     void add(filter *);
   public:
     void    import(void);
     QString getFilters(void);
};


#endif

