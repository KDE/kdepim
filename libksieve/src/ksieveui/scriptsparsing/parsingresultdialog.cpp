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

#include "parsingresultdialog.h"
#include "xmlprintingsyntaxhighlighter.h"
#include "PimCommon/PimUtil"
#include "kpimtextedit/plaintexteditorwidget.h"
#include "kpimtextedit/plaintexteditor.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KSieveUi;

ParsingResultDialog::ParsingResultDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Sieve Parsing"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ParsingResultDialog::reject);
    user1Button->setText(i18n("Save As..."));

    mTextEdit = new KPIMTextEdit::PlainTextEditorWidget(this);
    new XMLPrintingSyntaxHighLighter(mTextEdit->editor()->document());
    mTextEdit->setReadOnly(true);
    mainLayout->addWidget(mTextEdit);
    mainLayout->addWidget(buttonBox);

    connect(user1Button, &QPushButton::clicked, this, &ParsingResultDialog::slotSaveAs);
    readConfig();
}

ParsingResultDialog::~ParsingResultDialog()
{
    writeConfig();
}

void ParsingResultDialog::setResultParsing(const QString &result)
{
    mTextEdit->setPlainText(result);
}

void ParsingResultDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ParsingResultDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void ParsingResultDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ParsingResultDialog");
    group.writeEntry("Size", size());
}

void ParsingResultDialog::slotSaveAs()
{
    const QString filter = i18n("XML Files (*.xml);;All Files (*)");
    PimCommon::Util::saveTextAs(mTextEdit->toPlainText(), filter, this);
}

