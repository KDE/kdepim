/***************************************************************************
                          knuserentry.cpp  -  description
                             -------------------

    copyright            : (C) 1999 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "knglobals.h"
#include "knarticlecollection.h"   // KNFile
#include "knuserentry.h"


KNUserEntry::KNUserEntry()
{
}



KNUserEntry::~KNUserEntry()
{
}



const QCString& KNUserEntry::getSignature()
{
  s_igContents = "";      // don't cache file contents

  if (u_seSigFile) {
    if(!s_igPath.isEmpty()) {
      KNFile sigFile(s_igPath); 
      if(sigFile.open(IO_ReadOnly)) {
        s_igContents=sigFile.readLineWnewLine();
        while(!sigFile.atEnd())
          s_igContents += sigFile.readLineWnewLine();
        }
      else KMessageBox::error(knGlobals.topWidget, i18n("Cannot open the signature file!"));
    }
  } else
    s_igContents = s_igText;
  
  return s_igContents;
}



void KNUserEntry::load(KConfigBase *c)
{
  n_ame=c->readEntry("Name").local8Bit();
  e_mail=c->readEntry("Email").local8Bit();
  r_eplyTo=c->readEntry("Reply-To").local8Bit();
  o_rga=c->readEntry("Org").local8Bit();
  u_seSigFile=c->readBoolEntry("UseSigFile",false);
  s_igPath=c->readEntry("sigFile");
  s_igText=c->readEntry("sigText").local8Bit(); 
}



void KNUserEntry::save(KConfigBase *c)
{
  c->writeEntry("Name", n_ame.data());
  c->writeEntry("Email", e_mail.data());
  c->writeEntry("Reply-To", r_eplyTo.data());
  c->writeEntry("Org", o_rga.data());
  c->writeEntry("UseSigFile", u_seSigFile);
  c->writeEntry("sigFile", s_igPath);
  c->writeEntry("sigText", s_igText.data());
}



bool KNUserEntry::isValid()
{
  if (e_mail.isEmpty() || n_ame.isEmpty())
    return false;
  else
    return e_mail.contains(QRegExp("?*@?*.??*",true,true));
}



bool KNUserEntry::isEmpty()
{
  return (  n_ame.isEmpty() &&  e_mail.isEmpty() &&
            r_eplyTo.isEmpty()   && o_rga.isEmpty() &&
            s_igPath.isEmpty() );
}
