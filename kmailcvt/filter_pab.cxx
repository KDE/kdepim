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

filter_pab::filter_pab() : filter(i18n("Import MS Exchange Personal Address Book (.PAB)"),"Hans Dijkema")
{}

filter_pab::~filter_pab()
{}

void filter_pab::import(filterInfo *info)
{
QString  file;
QWidget *parent=info->parent();

   file=KFileDialog::getOpenFileName(QDir::homeDirPath(),"*.pab *.PAB *.Pab",parent);
   if (file.isEmpty()) {
     info->alert(name(),i18n("No address book chosen"));
     return;
   }

   {pab PAB(QFile::encodeName(file),this,info);
    QString from=i18n("Source: "),to=i18n("Destination: ");
      from+="\t";
      from+=file;
      to+="\t";
      to+=i18n("the KAddressBook");
      info->from(from);
      info->to(to);
      info->current(i18n("Currently converting .PAB addresses to Kab"));
      PAB.convert();
      info->current(i18n("Finished converting .PAB addresses to Kab"));
   }
}
