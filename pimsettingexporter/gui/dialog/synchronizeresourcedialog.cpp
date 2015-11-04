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
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mListResourceWidget = new QListWidget(this);
    mListResourceWidget->setObjectName(QStringLiteral("listresourcewidget"));
    KListWidgetSearchLine *listWidgetSearchLine = new KListWidgetSearchLine(this, mListResourceWidget);
    listWidgetSearchLine->setObjectName(QStringLiteral("listwidgetsearchline"));

    QLabel *lab = new QLabel(i18n("Some resources were added but data were not sync. Select resource that you want to sync:"));
    lab->setWordWrap(true);
    lab->setObjectName(QStringLiteral("label"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SynchronizeResourceDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SynchronizeResourceDialog::reject);
    okButton->setDefault(true);
    setModal(true);

    topLayout->addWidget(listWidgetSearchLine);
    topLayout->addWidget(mListResourceWidget);
    topLayout->addWidget(buttonBox);
    readConfig();
}

SynchronizeResourceDialog::~SynchronizeResourceDialog()
{
    writeConfig();
}

void SynchronizeResourceDialog::setResources(const QStringList &resources)
{
    //TODO
}

QStringList SynchronizeResourceDialog::resources() const
{
    return QStringList();
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
