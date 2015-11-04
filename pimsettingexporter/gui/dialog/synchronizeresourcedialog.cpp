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
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SynchronizeResourceDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SynchronizeResourceDialog::reject);
    okButton->setDefault(true);
    setModal(true);
    topLayout->addWidget(buttonBox);
}

SynchronizeResourceDialog::~SynchronizeResourceDialog()
{

}

void SynchronizeResourceDialog::slotAccepted()
{
    accept();
}
