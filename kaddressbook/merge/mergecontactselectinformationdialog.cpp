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

#include "mergecontactselectinformationdialog.h"
#include "mergecontactshowresulttabwidget.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KABMergeContacts;
MergeContactSelectInformationDialog::MergeContactSelectInformationDialog(const Akonadi::Item::List &lst, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Select Which Information to Use for new Contact"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MergeContactSelectInformationDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MergeContactSelectInformationDialog::reject);
    mTabWidget = new MergeContactShowResultTabWidget(this);
    mTabWidget->setObjectName(QStringLiteral("tabwidget"));
    mainLayout->addWidget(mTabWidget);
    mainLayout->addWidget(buttonBox);

    mTabWidget->setContacts(lst);
    updateTabWidget();
}

MergeContactSelectInformationDialog::~MergeContactSelectInformationDialog()
{

}

void MergeContactSelectInformationDialog::updateTabWidget()
{
    mTabWidget->updateTabWidget();
}

