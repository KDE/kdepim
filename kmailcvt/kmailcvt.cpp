/***************************************************************************
                          kmailcvt2.cpp  -  description
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

#include "kmailcvt.h"
#include <kaboutdialog.h>
#include <klocale.h>

//////////////////////////////////////////////////////////////////////////////
//
// Add filters here
//
//////////////////////////////////////////////////////////////////////////////

#include "filter_oe4.hxx"
#include "filter_oe5.hxx"
#include "filter_pab.hxx"

void Kmailcvt2::doFilters(void)
{
  imports->add(new filter_pab);
  imports->add(new filter_oe5);
  imports->add(new filter_oe4);
}


Kmailcvt2::Kmailcvt2(QWidget *parent, const char *name) : QWidget(parent, name)
{
int H;
  info=new filterInfo(this);

  imports=new filters(info,this);
  doFilters();
  imports->adjustSize();
  imports->move(10,10);

  import=new QPushButton(i18n("&Import"),this);
  import->adjustSize();
  import->move(imports->width()+imports->x()+10,10);

  H=max(import->height(),imports->height());

  quit=new QPushButton(i18n("&Quit"),this);
  quit->resize(import->width(),H);
  quit->move(import->x()+import->width()+10,10);

  about=new QPushButton(i18n("&About"),this);
  about->resize(import->width(),H);
  about->move(quit->x(),H+10+10);

  imports->resize(imports->width(),H);
  import->resize(import->width(),H);

  connect(import,SIGNAL(clicked()),SLOT(startFilter()));
  connect(quit,SIGNAL(clicked()),SLOT(Quit()));
  connect(about,SIGNAL(clicked()),SLOT(About()));

  resize(quit->x()+quit->width()+10,height());

  info->adjustSize();

  setCaption(KMAILCVT KMAILCVT_VERSION);
}

Kmailcvt2::~Kmailcvt2()
{
  delete imports;
  delete import;
  delete quit;
}


void Kmailcvt2::startFilter()
{
  import->setEnabled(false);
  imports->setEnabled(false);
  quit->setEnabled(false);
  about->setEnabled(false);

  imports->import();

  imports->setEnabled(true);
  import->setEnabled(true);
  quit->setEnabled(true);
  about->setEnabled(true);
}


void Kmailcvt2::Quit()
{
  close();
}

void Kmailcvt2::About()
{
QString cap=KMAILCVT KMAILCVT_VERSION;
/*
KAboutDialog dlg(KAboutDialog::AbtAppStandard,
                 cap,
                 KAboutDialog::Ok, 
                 KAboutDialog::Ok,
                 this,
                 "kmailcvt2_about",
                 true
                );

  dlg.setTitle("Welcome to KMailCvt");
  dlg.setProduct(KMAILCVT,
                 KMAILCVT_VERSION,
                 "Hans Dijkema",
                 "2000"
                );
  dlg.setImage();
  dlg.addTextPage("Description",
                  "Converts various formats to Kmail and K Addressbook\n\n"
                  "Currently supported:\n\n"
                  "  - MS Outlook Express 5.0 .DBX format\n"
                  "  - MS Exchange Personal Addressbook .PAB format\n"
                  "\n"
                  "You are invited to contribute other import (or output)\n"
                  "formats. I you want to contribute, please contact me at\n"
                  "kmailcvt@hum.org\n"
                 );
*/
KAboutDialog dlg(this,"KmailCvt2");
  dlg.setLogo(KApplication::kApplication()->icon());
  {QString name="(c) 2000 Hans Dijkema",
           email="kmailcvt@hum.org",
           url="http://www.hum.org/kmailcvt.html",
           work=i18n("\n"
                "Converts various import formats to Kmail and K Addressbook  \n"
                "Currently supported formats:\n\n"
                );
           work+=imports->getFilters();
           work+=i18n("\n"
                "You are invited to contribute other import (or output)\n"
                "formats. I you want to contribute, please contact me.\n"
                "Translations are welcome too!"
                );
    dlg.setAuthor(name,email,url,work);
    dlg.adjust();
  }
  dlg.setVersion(KMAILCVT KMAILCVT_VERSION);
  {int X,Y;
     dlg.adjustSize();
     X=(width()-dlg.width())/2+x();
     Y=(height()-dlg.height())/2+y();
     dlg.move(X,Y);
  }
  dlg.show();
}

