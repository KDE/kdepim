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

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <kdialogbase.h>

#include "knsettingswidget.h"

class QPushButton;
class QPixMap;

class KNAccountManager;
class KNListBox;
class KNNntpAccount;


//===============================================================================


class KNAccNewsSettings : public KNSettingsWidget  {

	Q_OBJECT	

	public:
		KNAccNewsSettings(QWidget *p, KNAccountManager *am);
		~KNAccNewsSettings();
				
	public slots:
		void slotAddItem(KNNntpAccount *a);
		void slotRemoveItem(KNNntpAccount *a);
		void slotUpdateItem(KNNntpAccount *a);
						
	protected:
		KNListBox *lb;
		QPushButton *addBtn, *delBtn, *editBtn;
		KNAccountManager *aManager;
		QPixmap pm;
		QLabel *serverInfo, *portInfo;
		
	protected slots:
	  void slotSelectionChanged();
		void slotItemSelected(int id);
		void slotAddBtnClicked();
		void slotDelBtnClicked();
		void slotEditBtnClicked();
};


//===============================================================================


class KNAccNewsConfDialog : public KDialogBase  {

  Q_OBJECT	

  public:
    KNAccNewsConfDialog(KNNntpAccount* acc, QWidget *parent=0, const char *name=0);
    ~KNAccNewsConfDialog();
	 			
  protected:
    QLineEdit *n_ame, *s_erver, *u_ser, *p_ass, *p_ort;
    QSpinBox *h_old, *t_imeout;
    QCheckBox *f_etchDes, *authCB;
    KNNntpAccount* acc;
		
  protected slots:
    void slotOk();
    void slotAuthChecked(bool b);
		
};


#endif
