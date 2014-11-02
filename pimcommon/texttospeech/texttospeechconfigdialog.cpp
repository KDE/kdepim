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

#include "texttospeechconfigdialog.h"
#include <KLocalizedString>
#include "texttospeechconfigwidget.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <kconfiggroup.h>
#include <KSharedConfig>

using namespace PimCommon;

TextToSpeechConfigDialog::TextToSpeechConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mTextToSpeechConfigWidget = new TextToSpeechConfigWidget(parent);
    mainLayout->addWidget(mTextToSpeechConfigWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TextToSpeechConfigDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TextToSpeechConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    mTextToSpeechConfigWidget->readConfig();
    readConfig();
}

TextToSpeechConfigDialog::~TextToSpeechConfigDialog()
{
    writeConfig();
}

void TextToSpeechConfigDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "TextToSpeechConfigDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void TextToSpeechConfigDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "TextToSpeechConfigDialog");
    group.writeEntry("Size", size());
}

void TextToSpeechConfigDialog::slotAccepted()
{
    mTextToSpeechConfigWidget->writeConfig();
    accept();
}
