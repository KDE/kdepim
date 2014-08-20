/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicepropertiesdialog.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QGridLayout>
#include <QLabel>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace PimCommon;

StorageServicePropertiesDialog::StorageServicePropertiesDialog(const QMap<QString, QString> &information, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Properties"));

    createInformationWidget(information);
    readConfig();
}

StorageServicePropertiesDialog::~StorageServicePropertiesDialog()
{
    writeConfig();
}

void StorageServicePropertiesDialog::createInformationWidget(const QMap<QString, QString> &information)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QWidget *parent = new QWidget;
    QGridLayout *grid = new QGridLayout;

    QMapIterator<QString, QString> i(information);
    int row = 0;
    while (i.hasNext()) {
        i.next();
        QLabel *type = new QLabel;
        QFont font = type->font();
        font.setBold(true);
        type->setFont(font);
        type->setAlignment(Qt::AlignRight);
        type->setText(i.key());
        grid->addWidget(type, row, 0);

        QLabel *info = new QLabel;
        info->setTextInteractionFlags(Qt::TextBrowserInteraction);
        info->setAlignment(Qt::AlignLeft);
        info->setText(i.value());
        grid->addWidget(info, row, 1);

        ++row;
    }
    mainLayout->addWidget(parent);
    mainLayout->addWidget(buttonBox);

}

void StorageServicePropertiesDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageServicePropertiesDialog" );
    const QSize size = group.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServicePropertiesDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group( QLatin1String("StorageServicePropertiesDialog") );
    group.writeEntry( "Size", size() );
}
