/*
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "contactselectiondialog.h"
#include "contactselectionwidget.h"

#include <QVBoxLayout>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>

ContactSelectionDialog::ContactSelectionDialog(QItemSelectionModel *selectionModel, bool allowToSelectTypeToExport,
        QWidget *parent)
    : QDialog(parent),
      mVCardExport(Q_NULLPTR)
{
    setWindowTitle(i18n("Select Contacts"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ContactSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ContactSelectionDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout;
    mainWidget->setLayout(layout);

    mSelectionWidget = new ContactSelectionWidget(selectionModel, this);
    if (allowToSelectTypeToExport) {
        layout->addWidget(mSelectionWidget);
        mVCardExport = new VCardExportSelectionWidget;
        layout->addWidget(mVCardExport);
    } else {
        layout->addWidget(mSelectionWidget);
    }
    mainLayout->addWidget(buttonBox);

    resize(QSize(450, 220));
}

void ContactSelectionDialog::setMessageText(const QString &message)
{
    mSelectionWidget->setMessageText(message);
}

void ContactSelectionDialog::setDefaultAddressBook(const Akonadi::Collection &addressBook)
{
    mSelectionWidget->setDefaultAddressBook(addressBook);
}

ContactList ContactSelectionDialog::selectedContacts() const
{
    return mSelectionWidget->selectedContacts();
}

VCardExportSelectionWidget::ExportFields ContactSelectionDialog::exportType() const
{
    if (mVCardExport) {
        return mVCardExport->exportType();
    } else {
        return VCardExportSelectionWidget::None;
    }
}

void ContactSelectionDialog::setAddGroupContact(bool addGroupContact)
{
    mSelectionWidget->setAddGroupContact(addGroupContact);
}

