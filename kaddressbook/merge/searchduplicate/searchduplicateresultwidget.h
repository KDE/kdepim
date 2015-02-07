/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SEARCHDUPLICATERESULTWIDGET_H
#define SEARCHDUPLICATERESULTWIDGET_H

#include <QWidget>
#include <AkonadiCore/Item>
#include "kaddressbook_export.h"
class QTreeWidget;
class QPushButton;
namespace KAddressBookGrantlee {
class GrantleeContactViewer;
}
namespace Akonadi {
class CollectionComboBox;
}
namespace KABMergeContacts {
class ResultDuplicateTreeWidget;
class KADDRESSBOOK_EXPORT SearchDuplicateResultWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchDuplicateResultWidget(QWidget *parent=0);
    ~SearchDuplicateResultWidget();
    void setContacts(const QList<Akonadi::Item::List> &lstItem);

Q_SIGNALS:
    void contactMerged(const Akonadi::Item &item);
    void mergeDone();


private slots:
    void slotMergeContact();
    void slotMergeDone(const Akonadi::Item &item);
private:
    void mergeContact();
    QList<Akonadi::Item::List> mListContactToMerge;
    ResultDuplicateTreeWidget *mResult;
    KAddressBookGrantlee::GrantleeContactViewer *mContactViewer;
    QPushButton *mMergeContact;
    Akonadi::CollectionComboBox *mCollectionCombobox;
    int mIndexListContact;
};
}

#endif // SEARCHDUPLICATERESULTWIDGET_H
