/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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


#include "completionconfiguredialog.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <KConfigGroup>
#include <KGlobal>
#include <KSharedConfig>
#include <addressline/completionorder/completionorderwidget.h>

using namespace KPIM;
CompletionConfigureDialog::CompletionConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    //KF5 add i18n
    setWindowTitle(QLatin1String("Configure completion"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mTabWidget = new QTabWidget;
    mTabWidget->setObjectName(QLatin1String("tabwidget"));
    mainLayout->addWidget(mTabWidget);

    //TODO mCompletionOrderWidget = new KPIM::CompletionOrderWidget();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttonBox->setObjectName(QLatin1String("buttonbox"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);

    mainLayout->addWidget(buttonBox);
    readConfig();
}


CompletionConfigureDialog::~CompletionConfigureDialog()
{
    writeConfig();
}

void CompletionConfigureDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionConfigureDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void CompletionConfigureDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionConfigureDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void CompletionConfigureDialog::load()
{
    //TODO
}

void CompletionConfigureDialog::slotSave()
{
    //TODO
}
