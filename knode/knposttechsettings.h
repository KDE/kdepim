/***************************************************************************
                          knposttechsettings.h  -  description
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


#ifndef KNPOSTTECHSETTINGS_H
#define KNPOSTTECHSETTINGS_H

#include <kdialogbase.h>
#include "knsettingsdialog.h"

class QComboBox;
class QCheckBox;
class QListBox;
class QPushButton;
class QLineEdit;
class QLabel;


class KNPostTechSettings : public KNSettingsWidget  {
  
  Q_OBJECT

  public:
    KNPostTechSettings(QWidget *p);
    ~KNPostTechSettings();
    
    void apply();
    
  protected:

    class XHeadDlg : public KDialogBase {

      public:
        XHeadDlg(QWidget *parent=0, const QString &header=QString::null);
        ~XHeadDlg();

        QString headResult();

      protected:
        QLineEdit *name, *value;
    };

    void init();
    QComboBox *charset, *encoding;
    QCheckBox *allow8bitCB, *genMIdCB, *incUaCB;
    QListBox *lb;
    QPushButton *addBtn, *delBtn, *editBtn;
    QLineEdit *host;
    QLabel *hostL;
    bool saveHdrs;
    
  protected slots:
    void slotHeadEnctoggled(bool b);
    void slotGenMIdCBtoggled(bool b);
    void slotSelectionChanged();
    void slotItemSelected(int id);
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();

};

#endif
