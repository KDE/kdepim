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


#ifndef KNPOSTSPELLSETTINGS_H
#define KNPOSTSPELLSETTINGS_H

#include "knsettingsdialog.h"

class KSpellConfig;


class KNPostSpellSettings : public KNSettingsWidget  {

  public:
    KNPostSpellSettings(QWidget *parent);
    virtual ~KNPostSpellSettings();
    
    void apply();
    
  protected:
  
    KSpellConfig *spellConf;

};

#endif
