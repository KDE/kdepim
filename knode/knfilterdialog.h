/***************************************************************************
                          knfilterdialog.h  -  description
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


#ifndef KNFILTERDIALOG_H
#define KNFILTERDIALOG_H

#include <kdialogbase.h>

class KNFilterConfigWidget;
class KNArticleFilter;
class QLineEdit;
class QComboBox;
class QCheckBox;


class KNFilterDialog : public KDialogBase {
  
  Q_OBJECT

  friend class KNFilterManager;
  
  public:
    KNFilterDialog(KNArticleFilter *f=0, QWidget *parent=0, const char *name=0);
    ~KNFilterDialog();
    
    KNArticleFilter* filter() { return fltr; }
          
  protected:
    KNFilterConfigWidget *fw;
    QLineEdit *fname;
    QComboBox *apon;
    QCheckBox *enabled;
    
    KNArticleFilter *fltr;

  protected slots:
    void slotOk();
        
};

#endif
