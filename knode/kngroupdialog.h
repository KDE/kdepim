/*
    kngroupdialog.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNGROUPDIALOG_H
#define KNGROUPDIALOG_H

#include "kngroupbrowser.h"


class KNGroupDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    KNGroupDialog(QWidget *parent, KNNntpAccount *a);
    ~KNGroupDialog();

    void toSubscribe(QSortedList<KNGroupInfo> *l);
    void toUnsubscribe(QStringList *l);

  protected:
    enum arrowDirection { right, left };
    enum arrowButton { btn1, btn2 };
    void updateItemState(CheckItem *it);
    void itemChangedState(CheckItem *it, bool s);
    void setButtonDirection(arrowButton b, arrowDirection d);
    QPushButton *newListBtn;
    QListView *subView, *unsubView;
    arrowDirection dir1, dir2;

  protected slots:
    void slotItemSelected(QListViewItem *it);
    /** deactivates the button when a root item is selected */
    void slotSelectionChanged();
    void slotArrowBtn1();
    void slotArrowBtn2();
    /** new list */
    void slotUser1();
    /** new groups */
    void slotUser2();

  signals:
    void fetchList(KNNntpAccount *a);
    void checkNew(KNNntpAccount *a,QDate date);
};

#endif
