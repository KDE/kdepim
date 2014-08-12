/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "customheadersettingdialog.h"
#include "header/customheadersettingwidget.h"
#include "settings/globalsettings.h"
#include <QDialogButtonBox>

using namespace MessageViewer;

CustomHeaderSettingDialog::CustomHeaderSettingDialog(QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mCustomHeaderWidget = new CustomHeaderSettingWidget();
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), mCustomHeaderWidget, SLOT(resetToDefault()) );
    mCustomHeaderWidget->readConfig();
    mainLayout->addWidget (mCustomHeaderWidget );
    mainLayout->addWidget(buttonBox);

    readConfig();
}

CustomHeaderSettingDialog::~CustomHeaderSettingDialog()
{
    writeConfig();
}

void CustomHeaderSettingDialog::readConfig()
{
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "CustomHeaderSettingDialog" );
    const QSize size = group.readEntry( "Size", QSize(500, 250) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void CustomHeaderSettingDialog::writeConfig()
{
    KConfigGroup group( MessageViewer::GlobalSettings::self()->config(), "CustomHeaderSettingDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void CustomHeaderSettingDialog::writeSettings()
{
    mCustomHeaderWidget->writeConfig();
}

