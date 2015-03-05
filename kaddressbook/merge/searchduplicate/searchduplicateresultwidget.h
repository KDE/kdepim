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
#include <Akonadi/Item>
#include "merge/job/mergecontacts.h"
#include "kaddressbook_export.h"
class QTreeWidget;
class KPushButton;
namespace KAddressBookGrantlee {
class GrantleeContactViewer;
}
namespace Akonadi {
class CollectionComboBox;
}
namespace KABMergeContacts {
class ResultDuplicateTreeWidget;
class MergeContactLoseInformationWarning;

struct MergeConflictResult {
    Akonadi::Item::List list;
    MergeContacts::ConflictInformations conflictInformation;
};

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
    void customizeMergeContact(const QVector<KABMergeContacts::MergeConflictResult> &, const Akonadi::Collection &col);


private slots:
    void slotMergeContact();
    void slotMergeDone(const Akonadi::Item &item);
    void slotUpdateMergeButton();

    void slotAutomaticMerging();
    void slotCustomizeMergingContacts();
private:
    void mergeContact();
    QVector<MergeConflictResult> mResultConflictList;
    QList<Akonadi::Item::List> mListContactToMerge;
    ResultDuplicateTreeWidget *mResult;
    KAddressBookGrantlee::GrantleeContactViewer *mContactViewer;
    KPushButton *mMergeContact;
    Akonadi::CollectionComboBox *mCollectionCombobox;
    MergeContactLoseInformationWarning *mMergeContactWarning;
    int mIndexListContact;
};
}
Q_DECLARE_TYPEINFO(KABMergeContacts::MergeConflictResult, Q_MOVABLE_TYPE);


#endif // SEARCHDUPLICATERESULTWIDGET_H
