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

#ifndef MERGECONTACTSELECTLISTWIDGET_H
#define MERGECONTACTSELECTLISTWIDGET_H

#include <Akonadi/Item>
#include "kaddressbook_export.h"
#include "merge/job/mergecontacts.h"
#include <QWidget>
#include <QIcon>
class QLabel;
class QListWidget;
namespace KABC {
class Addressee;
}
namespace KABMergeContacts {
class KADDRESSBOOK_EXPORT MergeContactSelectListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MergeContactSelectListWidget(QWidget *parent=0);
    ~MergeContactSelectListWidget();

    void setContacts(MergeContacts::ConflictInformation conflictType, const KABC::Addressee::List &lst);
    int selectedContact() const;
    MergeContacts::ConflictInformation conflictType() const;
    bool verifySelectedInfo() const;
private:
    void fillList(const KABC::Addressee::List &lst);
    void updateTitle();
    void addItem(const QString &str, const QIcon &icon = QIcon());
    QLabel *mTitle;
    QListWidget *mSelectListWidget;
    MergeContacts::ConflictInformation mConflictType;
};
}

#endif // MERGECONTACTSELECTLISTWIDGET_H
