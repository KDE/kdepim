/***************************************************************************
                          kngrouppropdlg.h  -  description
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

#ifndef KNGROUPPROPDLG_H
#define KNGROUPPROPDLG_H

#include <kdialogbase.h>

class QLineEdit;
class KNGroup;

namespace KNConfig {
class IdentityWidget;
};

class KNGroupPropDlg : public KDialogBase  {

  public:
    KNGroupPropDlg(KNGroup *group, QWidget *parent=0, const char *name=0);
    ~KNGroupPropDlg();
    
    bool nickHasChanged() { return nChanged; }  
    
  protected:
    KNGroup *grp;
    bool nChanged;
    KNConfig::IdentityWidget* i_dWidget;
    QLineEdit *nick;
    
  protected slots:
    void slotOk();
  
};

#endif
