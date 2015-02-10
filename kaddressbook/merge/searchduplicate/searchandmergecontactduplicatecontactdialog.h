/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef SEARCHANDMERGECONTACTDUPLICATECONTACTDIALOG_H
#define SEARCHANDMERGECONTACTDUPLICATECONTACTDIALOG_H

#include <QDialog>
#include <AkonadiCore/Item>

#include "kaddressbook_export.h"
class QStackedWidget;
class QLabel;
namespace KABMergeContacts
{
class SearchDuplicateResultWidget;
class MergeContactShowResultTabWidget;
class KADDRESSBOOK_EXPORT SearchAndMergeContactDuplicateContactDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchAndMergeContactDuplicateContactDialog(QWidget *parent = Q_NULLPTR);
    ~SearchAndMergeContactDuplicateContactDialog();

    void searchPotentialDuplicateContacts(const Akonadi::Item::List &list);

private Q_SLOTS:
    void slotDuplicateFound(const QList<Akonadi::Item::List> &duplicate);
    void slotMergeDone();
    void slotContactMerged(const Akonadi::Item &item);
private:
    void readConfig();
    void writeConfig();
    SearchDuplicateResultWidget *mSearchResult;
    MergeContactShowResultTabWidget *mMergeContactResult;
    QLabel *mNoContactSelected;
    QLabel *mNoDuplicateContactFound;
    QLabel *mNoEnoughContactSelected;
    QStackedWidget *mStackedWidget;
};
}

#endif // SEARCHANDMERGECONTACTDUPLICATECONTACTDIALOG_H
