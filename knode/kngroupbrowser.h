/***************************************************************************
                       kngroupbrowser.h  -  description
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


#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H

#include <qlistview.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qstrlist.h>
#include <qlayout.h>
#include <qlabel.h>


class KNNntpAccount;

class KNGroupBrowser : public QDialog {

  Q_OBJECT

  public:
    class CheckItem : public QCheckListItem {

      public:
        CheckItem(QListView *v, const QString &t, KNGroupBrowser *b);
        CheckItem(QListViewItem *i, const QString &t, KNGroupBrowser *b);
        ~CheckItem();
        void setChecked(bool c);

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;
    };

    KNGroupBrowser(QWidget *parent, KNNntpAccount *a);
    ~KNGroupBrowser();

    KNNntpAccount* account()      { return a_ccount; }
    virtual void itemChangedState(CheckItem *it, bool s)=0;

  protected:
    virtual void updateItemState(CheckItem *it, bool isSub)=0;
    void changeItemState(const QString &text, bool s);
    bool itemInListView(QListView *view, const QString &text);
    void removeListItem(QListView *view, const QString &text);
    void createListItems(QListViewItem *parent=0);

    QListView *groupView;
    QLineEdit *filterEdit;
    QCheckBox *subCB;
    QPushButton   *helpBtn,*cancelBtn,
                  *okBtn, *arrowBtn1,
                  *arrowBtn2;
    QPixmap pmRight, pmLeft, pmGroup;
    QHBoxLayout *btnL;
    QGridLayout *listL;
    QLabel *rightLabel;

    KNNntpAccount *a_ccount;
    QStrList *allList, *subList, *matchList, *listPtr;

  protected slots:
    void slotLoadList();
    void slotItemExpand(QListViewItem *it);
    void slotFilter(const QString &txt);
    void slotSubCBClicked();
};


#endif
