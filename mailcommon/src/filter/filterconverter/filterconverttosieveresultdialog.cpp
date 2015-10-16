/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "filterconverttosieveresultdialog.h"
#include "PimCommon/SieveSyntaxHighlighter"
#include "PimCommon/SieveSyntaxHighlighterUtil"
#include "kpimtextedit/plaintexteditor.h"
#include "kpimtextedit/plaintexteditorwidget.h"
#include "PimCommon/PimUtil"

#include <KLocalizedString>

#include <QHBoxLayout>

#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace MailCommon;

FilterConvertToSieveResultDialog::FilterConvertToSieveResultDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Convert to sieve script"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->setObjectName(QStringLiteral("buttonbox"));
    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);
    QPushButton *saveButton = new QPushButton;
    buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FilterConvertToSieveResultDialog::reject);
    saveButton->setText(i18n("Save..."));
    saveButton->setObjectName(QStringLiteral("savebutton"));
    saveButton->setDefault(true);
    setModal(true);
    connect(saveButton, &QPushButton::clicked, this, &FilterConvertToSieveResultDialog::slotSave);

    mEditor = new KPIMTextEdit::PlainTextEditorWidget;
    mEditor->editor()->setSpellCheckingSupport(false);
    mEditor->setObjectName(QStringLiteral("editor"));
    PimCommon::SieveSyntaxHighlighter *syntaxHighlighter = new PimCommon::SieveSyntaxHighlighter(mEditor->editor()->document());
    PimCommon::SieveSyntaxHighlighterUtil sieveHighlighterutil;
    const QStringList capabilities = sieveHighlighterutil.fullCapabilities();
    syntaxHighlighter->addCapabilities(capabilities);
    topLayout->addWidget(mEditor);
    topLayout->addWidget(buttonBox);

    readConfig();
}

FilterConvertToSieveResultDialog::~FilterConvertToSieveResultDialog()
{
    writeConfig();
}

void FilterConvertToSieveResultDialog::slotSave()
{
    const QString filter = i18n("Sieve Files (*.siv);;All Files (*)");
    PimCommon::Util::saveTextAs(mEditor->editor()->toPlainText(), filter, this, QUrl(),
                                i18nc("@title:window", "Convert to Script Sieve"));
}

void FilterConvertToSieveResultDialog::setCode(const QString &code)
{
    mEditor->editor()->setPlainText(code);
}

static const char myConfigGroupName[] = "FilterConvertToSieveResultDialog";

void FilterConvertToSieveResultDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), myConfigGroupName);

    const QSize size = group.readEntry("Size", QSize(500, 300));
    if (size.isValid()) {
        resize(size);
    }
}

void FilterConvertToSieveResultDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), myConfigGroupName);
    group.writeEntry("Size", size());
    group.sync();
}

