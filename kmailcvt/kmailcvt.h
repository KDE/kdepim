/***************************************************************************
                          kmailcvt2.h  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
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

#ifndef KMAILCVT2_H
#define KMAILCVT2_H

#define KMAILCVT_DCOP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <qwidget.h>
#include <stdio.h>


#define KMAILCVT_VERSION " v"VERSION
#define KMAILCVT         "KMail & K Addressbook import filters"

void procEvents(void);
int  dcopAddMessage(QString folderName,QString message);
void dcopReload(void);

#include <kapp.h>
#include <qwidget.h>
#include <qpushbutton.h>

#include "filters.hxx"

class Kmailcvt2 : public QWidget
{
  Q_OBJECT 
  public:
    /** construtor */
    Kmailcvt2(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~Kmailcvt2();
  private:
     QPushButton *import;
     QPushButton *quit;
     QPushButton *about;
     filters     *imports;
     filterInfo  *info;
  private slots:
     void startFilter();
     void Quit();
     void About();
  private:
     void doFilters(void);
};

#endif
