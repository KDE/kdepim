/*
    kngroupselectdialog.h

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

#ifndef KNGROUPSELECTDIALOG_H
#define KNGROUPSELECTDIALOG_H

#include "kngroupbrowser.h"


class KNGroupSelectDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    KNGroupSelectDialog(QWidget *parent, KNNntpAccount *a, const QString &act);
    ~KNGroupSelectDialog();

    QString selectedGroups()const;
    void itemChangedState(CheckItem *it, bool s);

  protected:
    void updateItemState(CheckItem *it);
    QListView *selView;

  protected slots:
    void slotItemSelected(QListViewItem *it);
    /** deactivates the button when a root item is selected */
    void slotSelectionChanged();
    void slotArrowBtn1();
    void slotArrowBtn2();

};


#endif















