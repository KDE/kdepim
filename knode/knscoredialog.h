/***************************************************************************
                          knscoredialog.h  -  description
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


#ifndef KNSCOREDIALOG_H
#define KNSCOREDIALOG_H

#include <kdialogbase.h>

class QRadioButton;
class QSpinBox;


class KNScoreDialog : public KDialogBase  {

  Q_OBJECT
  
  public:
    KNScoreDialog(short sc=50, QWidget *parent=0, const char *name=0);
    ~KNScoreDialog();
    
    short score();   // uhmm, sizeOf(int)==sizeOf(short) on most systems afaik
    
  protected:
    QRadioButton *iBtn, *nBtn, *wBtn, *cBtn;
    QSpinBox *spin;
    
};

#endif

