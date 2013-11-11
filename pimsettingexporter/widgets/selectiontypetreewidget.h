/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#ifndef SELECTIONTYPETREEWIDGET_H
#define SELECTIONTYPETREEWIDGET_H

#include <QTreeWidget>
#include <QHash>
#include "utils.h"

class QTreeWidgetItem;

class SelectionTypeTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit SelectionTypeTreeWidget(QWidget *parent=0);
    ~SelectionTypeTreeWidget();

    QHash<Utils::AppsType, Utils::importExportParameters> storedType() const;

    void selectAllItems();
    void unSelectAllItems();

private Q_SLOTS:
    void slotItemChanged(QTreeWidgetItem*,int);

private:
    enum ActionType {
        action = Qt::UserRole + 1
    };

    void initialize();
    void setSelectItems(bool b);
    void changeState(QTreeWidgetItem *item, bool b);
    void createSubItem(QTreeWidgetItem *parent, Utils::StoredType type);

    Utils::importExportParameters typeChecked(QTreeWidgetItem *parent) const;

    QTreeWidgetItem *mKmailItem;
    QTreeWidgetItem *mKalarmItem;
    QTreeWidgetItem *mKaddressbookItem;
    QTreeWidgetItem *mKorganizerItem;
    QTreeWidgetItem *mKjotsItem;
    QTreeWidgetItem *mKNotesItem;
    QTreeWidgetItem *mAkregatorItem;
    QTreeWidgetItem *mBlogiloItem;
};

#endif // SELECTIONTYPETREEWIDGET_H
