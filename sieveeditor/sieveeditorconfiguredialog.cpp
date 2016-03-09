/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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

#include "sieveeditorconfiguredialog.h"
#include "serversievelistwidget.h"
#include "sieveeditorconfigureserverwidget.h"
#include "PimCommon/ConfigureImmutableWidgetUtils"
#include "sieveeditorglobalconfig.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QVBoxLayout>
#include <QCheckBox>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGroupBox>

SieveEditorConfigureDialog::SieveEditorConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveEditorConfigureDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveEditorConfigureDialog::reject);

    mTabWidget = new QTabWidget(this);
    mTabWidget->setObjectName(QStringLiteral("tabwidget"));

    //Server page
    QWidget *w = new QWidget;
    mTabWidget->addTab(w, i18n("Sieve Server"));

    QVBoxLayout *layout = new QVBoxLayout;
    w->setLayout(layout);
    mServerWidget = new SieveEditorConfigureServerWidget;
    layout->addWidget(mServerWidget);

    mCloseWallet = new QCheckBox(i18n("Close wallet when close application"));
    layout->addWidget(mCloseWallet);

    QWidget *editorWidget = new QWidget;
    editorWidget->setObjectName(QStringLiteral("editorwidget"));
    mTabWidget->addTab(editorWidget, i18n("Editor"));

    layout = new QVBoxLayout;
    editorWidget->setLayout(layout);
    mWrapText = new QCheckBox(i18n("Wrap Text"));
    mWrapText->setObjectName(QStringLiteral("wraptext"));
    layout->addWidget(mWrapText);
    layout->addStretch(100);
    mainLayout->addWidget(mTabWidget);
    mainLayout->addWidget(buttonBox);
    loadServerSieveConfig();
    readConfig();
}

SieveEditorConfigureDialog::~SieveEditorConfigureDialog()
{
    writeConfig();
}

void SieveEditorConfigureDialog::loadServerSieveConfig()
{
    mServerWidget->readConfig();
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
    PimCommon::ConfigureImmutableWidgetUtils::loadWidget(mWrapText, SieveEditorGlobalConfig::self()->wrapTextItem());
}

void SieveEditorConfigureDialog::saveServerSieveConfig()
{
    mServerWidget->writeConfig();
    PimCommon::ConfigureImmutableWidgetUtils::saveCheckBox(mCloseWallet, SieveEditorGlobalConfig::self()->closeWalletItem());
    PimCommon::ConfigureImmutableWidgetUtils::saveCheckBox(mWrapText, SieveEditorGlobalConfig::self()->wrapTextItem());
    SieveEditorGlobalConfig::self()->save();
}

void SieveEditorConfigureDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditorConfigureDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void SieveEditorConfigureDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditorConfigureDialog");
    group.writeEntry("Size", size());
    group.sync();
}
