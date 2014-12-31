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

#include "mergecontactshowresultdialog.h"
#include "mergecontactshowresulttabwidget.h"
#include "mergecontactinfowidget.h"

#include <KContacts/Addressee>

#include <KLocalizedString>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KABMergeContacts;

MergeContactShowResultDialog::MergeContactShowResultDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Merged Contact"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MergeContactShowResultDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MergeContactShowResultDialog::reject);
    readConfig();
    mTabWidget = new MergeContactShowResultTabWidget(this);
    mTabWidget->setObjectName(QLatin1String("tabwidget"));
    mainLayout->addWidget(mTabWidget);
    mainLayout->addWidget(buttonBox);

    updateTabWidget();
}

MergeContactShowResultDialog::~MergeContactShowResultDialog()
{
    writeConfig();
}

void MergeContactShowResultDialog::updateTabWidget()
{
    mTabWidget->updateTabWidget();
}

void MergeContactShowResultDialog::setContacts(const Akonadi::Item::List &lstItem)
{
    mTabWidget->setContacts(lstItem);
}

void MergeContactShowResultDialog::readConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactShowResultDialog");
    const QSize size = grp.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void MergeContactShowResultDialog::writeConfig()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "MergeContactShowResultDialog");
    grp.writeEntry("Size", size());
    grp.sync();
}
