/***************************************************************************
       knpostspellsettings.h  -  config page for the spellchecker
                                  -------------------
   
    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>

#include <kspell.h>

#include "knpostspellsettings.h"


KNPostSpellSettings::KNPostSpellSettings(QWidget *parent)
  : KNSettingsWidget(parent)
{
  QVBoxLayout *topL=new QVBoxLayout(this, 5);

  spellConf = new KSpellConfig( this, "spell", 0, false );
  topL->addWidget(spellConf);

  topL->addStretch(1);
}


KNPostSpellSettings::~KNPostSpellSettings()
{
}


void KNPostSpellSettings::apply()
{
}


