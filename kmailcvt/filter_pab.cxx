/***************************************************************************
                          filter_pab.cxx  -  description
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

#include "filter_pab.hxx"
#include "pablib.hxx"

#include <kfiledialog.h>
#include <klocale.h>

filter_pab::filter_pab() : filter(i18n("Import MS Exchange Personal Adress Book (.PAB)"),"Hans Dijkema")
{}

filter_pab::~filter_pab()
{}

void filter_pab::import(filterInfo *info)
{
QString _file;
char     file[1024];
char     dir[1024];
QWidget *parent=info->parent();
   sprintf(dir,getenv("HOME"));

   _file=KFileDialog::getOpenFileName(dir,"*.pab *.PAB *.Pab",parent);
   if (_file.length()==0) {
     info->alert(name(),i18n("No Adressbook choosen"));
     return;
   }
   strcpy(file,_file.latin1());

   {pab PAB(file,this,info);
    QString from=i18n("from: "),to=i18n("to: ");
      from+="\t";
      from+=file;
      to+="\t";
      to+=i18n("the K Address Book");
      info->from(from);
      info->to(to);
      info->current(i18n("Currently converting .PAB addresses to Kab"));
      PAB.convert();
      info->current(i18n("Done converting .PAB addresses to Kab"));
   }
}
