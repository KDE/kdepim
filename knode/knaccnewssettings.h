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
				
		void addItem(KNNntpAccount *a);
		void removeItem(KNNntpAccount *a);
		void updateItem(KNNntpAccount *a);  // the settings dialog is not modal!
				
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
    KNAccNewsConfDialog(KNNntpAccount* acc=0, QWidget *parent=0, const char *name=0);
    ~KNAccNewsConfDialog();
		
    QString name()  const    { return n_ame->text(); }
    QString server() const   { return s_erver->text(); }
    int port() const         { return p_ort->text().toInt(); }
    int hold() const         { return h_old->value(); }
    int timeout() const      { return t_imeout->value(); }
    bool logonNeeded() const { return authCB->isChecked(); }
    QString user() const     { return u_ser->text(); }
    QString pass() const     { return p_ass->text(); }
				
  protected:
    QLineEdit *n_ame, *s_erver, *u_ser, *p_ass, *p_ort;
    QSpinBox *h_old, *t_imeout;
    QCheckBox *authCB;
		
  protected slots:
    void slotAuthChecked(bool b);
		
};


#endif
