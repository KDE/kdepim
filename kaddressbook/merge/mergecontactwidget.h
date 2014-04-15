/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef MERGECONTACTWIDGET_H
#define MERGECONTACTWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <Akonadi/Item>

class QListWidget;
class QPushButton;

class MergeContactWidgetItem : public QListWidgetItem
{
public:
    MergeContactWidgetItem(const Akonadi::Item &item, QListWidget *parent = 0);
    Akonadi::Item item() const;

private:
    Akonadi::Item mItem;
};

class MergeContactWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactWidget(const Akonadi::Item::List &items, QWidget *parent=0);
    ~MergeContactWidget();

Q_SIGNALS:
    void mergeContact(const Akonadi::Item::List &lst);

private slots:
    void slotUpdateMergeButton();
    void slotMergeContacts();

private:
    void fillListContact();
    Akonadi::Item::List mItems;
    QListWidget *mListWidget;
    QPushButton *mMergeButton;
};

#endif // MERGECONTACTWIDGET_H
