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

#include <QDialog>
#include <AkonadiCore/Item>
#include <KConfigGroup>
class QDialogButtonBox;
namespace KABMergeContacts
{
class MergeContactWidget;
class MergeContactsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MergeContactsDialog(const Akonadi::Item::List &lst, QWidget *parent = 0);
    ~MergeContactsDialog();

private slots:
    void slotMergeContact(const Akonadi::Item::List &lst, const Akonadi::Collection &col);

    void slotMergeContactFinished(const Akonadi::Item &item);

private:
    void readConfig();
    void writeConfig();
    MergeContactWidget *mContactWidget;
    QDialogButtonBox *mButtonBox;
};
}
#endif // MERGECONTACTSDIALOG_H
