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

#ifndef FILTERS_HXX
#define FILTERS_HXX

#include <qcombobox.h>
#include <qprogressbar.h>
#include <qlistbox.h>
#include <qlabel.h>

#include <kabc/addressbook.h>

#include "kimportpagedlg.h"

class FilterInfo
{
  private:
    KImportPageDlg *_dlg;
    QWidget      *_parent;
  public:
    FilterInfo(KImportPageDlg *dlg, QWidget *parent);
   ~FilterInfo();

    void  from(QString from);
    void  to(QString to);
    void  current(QString current);
    void  current(float percent=0.0f);
    void  overall(float percent=0.0f);
    void  log(QString toLog);
    void  clear(void);
    void  alert(QString c,QString m);
    QWidget *parent(void) { return _parent; }
};

class KMail
{
  public:
    KMail();
   ~KMail();
   
    bool kmailStart(FilterInfo *) { return true; }
    bool kmailMessage(FilterInfo *info,QString folder,QString msgFile);
    void kmailStop(FilterInfo *info);
  private:
    QString cap;

};

class KAb
{
  public:
    KAb();
   ~KAb();
    bool kabStart(FilterInfo *info);
    bool kabAddress(FilterInfo *info, QString adrbookname,
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
    void kabStop(FilterInfo *info);

  private:
    FilterInfo  *info;    // tmp var
    KABC::AddressBook *mAddressBook;
    KABC::Ticket *mTicket;
    QString       tels;
    QString       cap;
    bool checkStr( QString & );
};



class Filter : public KMail, public KAb
{
  public:
    Filter(QString name,QString author,QString info=QString::null);
    virtual ~Filter();
    virtual void import(FilterInfo *i);
    QString author(void);
    QString name(void);
    QString info(void);

  private:
    QString myName;
    QString myAuthor;
    QString myInfo;
};

#endif

