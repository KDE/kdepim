/***************************************************************************
                          knreadgensettings.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
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


#ifndef KNREADGENSETTINGS_H
#define KNREADGENSETTINGS_H

#include "knsettingsdialog.h"

class QCheckBox;
class QSpinBox;
class QComboBox;


class KNReadGenSettings : public KNSettingsWidget  {
  
  public:
    KNReadGenSettings(QWidget *p);
    ~KNReadGenSettings();
    
    void apply();

  protected:
    void init();
    
    QCheckBox *autoCB, *markCB, *sigCB, *inlineCB, *openAttCB, *expCB, *altAttCB;
    QSpinBox *markSecs, *maxFetch;
    QComboBox *browser;   
    
};

#endif
