/***************************************************************************
                          FilterPAB.cxx  -  description
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

FilterPAB::FilterPAB() : Filter(i18n("Import MS Exchange Personal Address Book (.PAB)"),"Hans Dijkema")
{
}

FilterPAB::~FilterPAB()
{
}

void FilterPAB::import(FilterInfo *info)
{
  QString file = KFileDialog::getOpenFileName(QDir::homeDirPath(),"*.[pP][aA][bB]|MS Personal Address Book Files (*.pab)", info->parent());
  
  if (file.isEmpty()) {
    info->alert(i18n("No address book chosen"));
    return;
  }

  {
    pab PAB(QFile::encodeName(file),this,info);
    info->setFrom(file);
    info->setTo(i18n("KAddressBook"));
    info->setCurrent(i18n("Currently converting .PAB addresses to Kab"));
    PAB.convert();
    info->setCurrent(i18n("Finished converting .PAB addresses to Kab"));
  }
}

// vim: ts=2 sw=2 et
