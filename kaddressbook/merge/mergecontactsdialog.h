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


#ifndef MERGECONTACTSDIALOG_H
#define MERGECONTACTSDIALOG_H

#include <KDialog>
#include <Akonadi/Item>
class QItemSelectionModel;
namespace KABMergeContacts {
class MergeContactsDialog : public KDialog
{
    Q_OBJECT
public:
    explicit MergeContactsDialog(QItemSelectionModel *selectionModel, QWidget *parent=0);
    ~MergeContactsDialog();

private slots:
    void slotMergeContact(const Akonadi::Item::List &lst);

private:
    void readConfig();
    void writeConfig();
};
}
#endif // MERGECONTACTSDIALOG_H
