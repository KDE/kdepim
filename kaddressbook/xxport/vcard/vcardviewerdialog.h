/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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
#ifndef VCARDVIEWERDIALOG_H
#define VCARDVIEWERDIALOG_H

#include <QDialog>
#include <KContacts/Addressee>

namespace KAddressBookGrantlee
{
class GrantleeContactViewer;
}
class QPushButton;
class VCardViewerDialog : public QDialog
{
    Q_OBJECT
public:
    VCardViewerDialog(const KContacts::Addressee::List &list,
                      QWidget *parent);
    ~VCardViewerDialog();

    KContacts::Addressee::List contacts() const;

protected Q_SLOTS:
    void slotYes();
    void slotNo();
    void slotApply();
    void slotCancel();

private:
    void readConfig();
    void writeConfig();
    void updateView();

    KAddressBookGrantlee::GrantleeContactViewer *mView;

    KContacts::Addressee::List mContacts;
    KContacts::Addressee::List::Iterator mIt;
    QPushButton *mApplyButton;
};

#endif // VCARDVIEWERDIALOG_H
