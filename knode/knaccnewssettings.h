/***************************************************************************
                          knaccnewssettings.h  -  description
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


#ifndef KNACCNEWSSETTINGS_H
#define KNACCNEWSSETTINGS_H

#include <kdialogbase.h>

#include "knsettingsdialog.h"

class QPushButton;
class QPixMap;
class QLabel;
class QCheckBox;
class QSpinBox;
class QLineEdit;

class KNAccountManager;
class KNGroupManager;
class KNListBox;
class KNNntpAccount;


//===============================================================================


class KNAccNewsSettings : public KNSettingsWidget  {

  Q_OBJECT  

  public:
    KNAccNewsSettings(QWidget *p, KNAccountManager *am, KNGroupManager *gm);
    ~KNAccNewsSettings();
        
  public slots:
    void slotAddItem(KNNntpAccount *a);
    void slotRemoveItem(KNNntpAccount *a);
    void slotUpdateItem(KNNntpAccount *a);
            
  protected:
    KNListBox *lb;
    QPushButton *addBtn, *delBtn, *editBtn, *subBtn;
    KNAccountManager *aManager;
    KNGroupManager *gManager;
    QPixmap pm;
    QLabel *serverInfo, *portInfo;
    
  protected slots:
    void slotSelectionChanged();
    void slotItemSelected(int id);
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotSubBtnClicked();

};


//===============================================================================


class KNAccNewsConfDialog : public KDialogBase  {

  Q_OBJECT  

  public:
    KNAccNewsConfDialog(KNNntpAccount* acc, QWidget *parent=0, const char *name=0);
    ~KNAccNewsConfDialog();
        
  protected:
    QLineEdit *n_ame, *s_erver, *u_ser, *p_ass, *p_ort;
    QLabel *userLabel, *passLabel;
    QSpinBox *h_old, *t_imeout;
    QCheckBox *f_etchDes, *authCB;
    KNNntpAccount* acc;
    
  protected slots:
    void slotOk();
    void slotAuthChecked(bool b);
    
};


#endif
