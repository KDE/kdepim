/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MERGECONTACTSHOWRESULTDIALOG_H
#define MERGECONTACTSHOWRESULTDIALOG_H

#include <KDialog>
#include "kaddressbook_export.h"

#include <Akonadi/Item>
#include <QTabWidget>

namespace KABMergeContacts {

class KADDRESSBOOK_EXPORT MergeContactShowResultTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit MergeContactShowResultTabWidget(QWidget *parent=0);
    ~MergeContactShowResultTabWidget();

    void updateTabWidget();

    bool tabBarVisible() const;

};

class KADDRESSBOOK_EXPORT MergeContactShowResultDialog : public KDialog
{
    Q_OBJECT
public:
    explicit MergeContactShowResultDialog(QWidget *parent = 0);
    ~MergeContactShowResultDialog();

    void setContacts(const Akonadi::Item::List &lstItem);

private:
    void updateTabWidget();
    void readConfig();
    void writeConfig();
    MergeContactShowResultTabWidget *mTabWidget;
};
}

#endif // MERGECONTACTSHOWRESULTDIALOG_H
