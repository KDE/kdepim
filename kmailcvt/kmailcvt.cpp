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
#include <qgrid.h>
#include <qlayout.h>

//////////////////////////////////////////////////////////////////////////////
//
// Add filters here
//
//////////////////////////////////////////////////////////////////////////////

#include "filter_oe4.hxx"
#include "filter_oe5.hxx"
#include "filter_pmail.hxx"
#include "filter_pab.hxx"
#include "filter_eudora_ab.hxx"
#include "filter_ldif.hxx"

void Kmailcvt2::doFilters(void)
{
  imports->add(new filter_oe5);
  imports->add(new filter_pmail);
  imports->add(new filter_oe4);
  imports->add(new filter_pab);
  imports->add(new filter_ldif);
  imports->add(new filter_eudora_ab);
}


Kmailcvt2::Kmailcvt2(QWidget *parent, const char *name) : QWidget(parent, name)
{
  QGridLayout *grid1 = new QGridLayout(this,15,8,15,7);
  info=new filterInfo(this);

  grid1->addMultiCellWidget(info, 2,14,0,7);
  imports=new filters(info,this);
  grid1->addMultiCellWidget(imports, 0,0,0,5);
  doFilters();

  import=new QPushButton(i18n("&Import"),this);
  grid1->addWidget(import, 0,6);

  quit=new QPushButton(i18n("&Quit"),this);
  grid1->addWidget(quit, 0,7);

  about=new QPushButton(i18n("&About"),this);
  grid1->addWidget(about,1 ,7);

  connect(import,SIGNAL(clicked()),SLOT(startFilter()));
  connect(quit,SIGNAL(clicked()),SLOT(Quit()));
  connect(about,SIGNAL(clicked()),SLOT(About()));

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

#include "kmailcvt.moc"
