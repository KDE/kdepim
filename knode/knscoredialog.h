/*
    knscoredialog.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNSCOREDIALOG_H
#define KNSCOREDIALOG_H

#include <kdialogbase.h>

class QRadioButton;
class KIntSpinBox;


class KNScoreDialog : public KDialogBase  {

  Q_OBJECT
  
  public:
    KNScoreDialog(short sc=50, QWidget *parent=0, const char *name=0);
    ~KNScoreDialog();
    
    short score();   // uhmm, sizeOf(int)==sizeOf(short) on most systems afaik
    
  protected:
    QRadioButton *iBtn, *nBtn, *wBtn, *cBtn;
    KIntSpinBox *spin;
    
};

#endif

