/***************************************************************************
                          kncleanupsettings.h  -  description
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


#ifndef KNCLEANUPSETTINGS_H
#define KNCLEANUPSETTINGS_H

#include "knsettingsdialog.h"

class QCheckBox;
class QSpinBox;
class QLabel;


class KNCleanupSettings : public KNSettingsWidget  {
  
  Q_OBJECT

  public:
    KNCleanupSettings(QWidget *p);
    ~KNCleanupSettings();
    
    void apply();
    
  protected:
    void init();
    QCheckBox *folderCB, *groupCB, *thrCB;
    QSpinBox *folderDays, *groupDays, *readDays, *unreadDays;
    QLabel *folderDaysL,*groupDaysL, *readDaysL, *unreadDaysL;
    
  protected slots:
    void slotGroupCBtoggled(bool b);
    void slotFolderCBtoggled(bool b);
};

#endif
