/***************************************************************************
                          knsettingsdialog.h  -  description
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


#ifndef KNSETTINGSDIALOG_H
#define KNSETTINGSDIALOG_H

#include <kdialogbase.h>

class KNSettingsWidget;


class KNSettingsDialog : public KDialogBase  {
  
  Q_OBJECT  

  public:
    KNSettingsDialog(QWidget *parent=0, const char *name=0);
    ~KNSettingsDialog();
    
  protected:
    QList<KNSettingsWidget> widgets;

    virtual void slotOk();
    virtual void slotApply();

};

#endif
