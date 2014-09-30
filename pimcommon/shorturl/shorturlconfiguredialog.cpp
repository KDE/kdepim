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

#include "shorturlconfiguredialog.h"
#include "shorturlconfigurewidget.h"

#include <KLocalizedString>
#include <KSeparator>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace PimCommon;
ShortUrlConfigureDialog::ShortUrlConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure engine"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ShortUrlConfigureDialog::slotOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ShortUrlConfigureDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ShortUrlConfigureDialog::slotDefaultClicked);

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    mConfigureWidget = new ShortUrlConfigureWidget();
    mConfigureWidget->loadConfig();
    lay->addWidget(mConfigureWidget);
    lay->addWidget(new KSeparator);
}

ShortUrlConfigureDialog::~ShortUrlConfigureDialog()
{

}

void ShortUrlConfigureDialog::slotOkClicked()
{
    mConfigureWidget->writeConfig();
    accept();
}

void ShortUrlConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->resetToDefault();
}

#include "moc_shorturlconfiguredialog.cpp"
