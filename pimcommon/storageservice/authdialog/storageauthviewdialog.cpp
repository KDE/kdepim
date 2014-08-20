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
#include "storageauthviewdialog.h"
#include "storageauthviewwidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
using namespace PimCommon;

StorageAuthViewDialog::StorageAuthViewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n( "Authorize" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mView = new StorageAuthViewWidget;
    mainLayout->addWidget(mView);
    mainLayout->addWidget(buttonBox);

    connect(mView, SIGNAL(urlChanged(QUrl)), SIGNAL(urlChanged(QUrl)));
    mainLayout->addWidget(mView);
    readConfig();
}

StorageAuthViewDialog::~StorageAuthViewDialog()
{
    writeConfig();
}

void StorageAuthViewDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageAuthViewDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageAuthViewDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageAuthViewDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void StorageAuthViewDialog::setUrl(const QUrl &url)
{
    mView->setUrl(url);
}

