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

#include "synchronizeresourcedialog.h"
#include <KLocalizedString>
#include <KListWidgetSearchLine>

#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <KConfigGroup>
#include <KSharedConfig>

SynchronizeResourceDialog::SynchronizeResourceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Synchronize Resources"));
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);

    QLabel *lab = new QLabel(i18n("Some resources were added but data were not sync. Select resources that you want to sync:"));
    lab->setWordWrap(true);
    lab->setObjectName(QStringLiteral("label"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mListResourceWidget = new QListWidget(this);
    mListResourceWidget->setObjectName(QStringLiteral("listresourcewidget"));
    KListWidgetSearchLine *listWidgetSearchLine = new KListWidgetSearchLine(this, mListResourceWidget);
    listWidgetSearchLine->setObjectName(QStringLiteral("listwidgetsearchline"));

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *selectAll = new QPushButton(i18n("Select All"));
    selectAll->setObjectName(QStringLiteral("selectall_button"));
    connect(selectAll, &QPushButton::clicked, this, &SynchronizeResourceDialog::slotSelectAll);
    hbox->addWidget(selectAll);

    QPushButton *unselectAll = new QPushButton(i18n("Unselect All"));
    unselectAll->setObjectName(QStringLiteral("unselectall_button"));
    connect(unselectAll, &QPushButton::clicked, this, &SynchronizeResourceDialog::slotUnselectAll);
    hbox->addWidget(unselectAll);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SynchronizeResourceDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SynchronizeResourceDialog::reject);
    okButton->setDefault(true);
    setModal(true);

    topLayout->addWidget(lab);
    topLayout->addWidget(listWidgetSearchLine);
    topLayout->addWidget(mListResourceWidget);
    topLayout->addLayout(hbox);
    topLayout->addWidget(buttonBox);
    readConfig();
}

SynchronizeResourceDialog::~SynchronizeResourceDialog()
{
    writeConfig();
}

void SynchronizeResourceDialog::slotSelectAll()
{
    selectItem(true);
}

void SynchronizeResourceDialog::slotUnselectAll()
{
    selectItem(false);
}

void SynchronizeResourceDialog::selectItem(bool state)
{
    for (int i = 0; i < mListResourceWidget->count(); ++i) {
        QListWidgetItem *item = mListResourceWidget->item(i);
        item->setCheckState(state ? Qt::Checked : Qt::Unchecked);
    }
}

void SynchronizeResourceDialog::setResources(const QHash<QString, QString> &resources)
{
    QHashIterator<QString, QString> i(resources);
    while (i.hasNext()) {
        i.next();
        QListWidgetItem *item = new QListWidgetItem(mListResourceWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setText(i.key());
        item->setData(ResourceIdentifier, i.value());
    }
}

QStringList SynchronizeResourceDialog::resources() const
{
    QStringList lst;
    for (int i = 0; i < mListResourceWidget->count(); ++i) {
        QListWidgetItem *item = mListResourceWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            lst << item->data(ResourceIdentifier).toString();
        }
    }
    return lst;
}

void SynchronizeResourceDialog::slotAccepted()
{
    accept();
}

void SynchronizeResourceDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SynchronizeResourceDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void SynchronizeResourceDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SynchronizeResourceDialog");
    group.writeEntry("Size", size());
}
