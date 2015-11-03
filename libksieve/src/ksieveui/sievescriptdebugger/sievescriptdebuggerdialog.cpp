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

#include "sievescriptdebuggerdialog.h"
#include "sievescriptdebuggerwidget.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KSieveUi;

SieveScriptDebuggerDialog::SieveScriptDebuggerDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    setWindowTitle(i18n("Debug Sieve Script"));

    mSieveScriptDebuggerWidget = new SieveScriptDebuggerWidget(this);
    mSieveScriptDebuggerWidget->setObjectName(QStringLiteral("sievescriptdebuggerwidget"));
    mainLayout->addWidget(mSieveScriptDebuggerWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveScriptDebuggerDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveScriptDebuggerDialog::reject);

    readConfig();
}

SieveScriptDebuggerDialog::~SieveScriptDebuggerDialog()
{
    writeConfig();
}

void SieveScriptDebuggerDialog::slotAccepted()
{
    if (mSieveScriptDebuggerWidget->canAccept()) {
        accept();
    }
}

void SieveScriptDebuggerDialog::setScript(const QString &script)
{
    mSieveScriptDebuggerWidget->setScript(script);
}

QString SieveScriptDebuggerDialog::script() const
{
    return mSieveScriptDebuggerWidget->script();
}

void SieveScriptDebuggerDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveScriptDebuggerDialog");
    group.writeEntry("Size", size());
}

void SieveScriptDebuggerDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveScriptDebuggerDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}
