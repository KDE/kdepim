/***************************************************************************
                          knreadhdrsettings.h  -  description
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


#ifndef KNREADHDRSETTINGS_H
#define KNREADHDRSETTINGS_H

#include <qlistbox.h>
#include <kdialogbase.h>
#include "knsettingsdialog.h"

class QComboBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class KNViewHeader;


class KNReadHdrSettings : public KNSettingsWidget  {
  
  Q_OBJECT

  public:
    KNReadHdrSettings(QWidget *p);
    ~KNReadHdrSettings();

  protected:

    class ConfDialog;

    class HdrItem : public QListBoxText {

      public:
        HdrItem( const QString &, KNViewHeader * );
        ~HdrItem();

        KNViewHeader *hdr;
    };

    HdrItem* generateItem(KNViewHeader *);

    QListBox *lb;
    QPushButton *addBtn, *delBtn, *editBtn, *upBtn, *downBtn;
    bool save;

  protected slots:
    void slotItemSelected(int);
    void slotSelectionChanged();
    void slotAddBtnClicked();
    void slotDelBtnClicked();
    void slotEditBtnClicked();
    void slotUpBtnClicked();
    void slotDownBtnClicked();

};


class KNReadHdrSettings::ConfDialog  : public KDialogBase {

  Q_OBJECT

  public:
    ConfDialog(KNViewHeader *, QWidget *parent=0, char *name=0);
    ~ConfDialog();

  protected slots:
    void slotOk();
    void slotActivated(int);
    void slotNameChanged(const QString&);

  protected:
    KNViewHeader *hdr;
    QComboBox *hdrC;
    QLineEdit *nameE;
    QCheckBox *nameCB[4], *valueCB[4];
};


#endif
