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

#include "sievetemplateeditdialog.h"
#include "editor/sievetextedit.h"

#include "pimcommon/texteditor/plaintexteditor/plaintexteditfindbar.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShortcut>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace KSieveUi;

SieveTemplateEditDialog::SieveTemplateEditDialog(QWidget *parent, bool defaultTemplate)
    : QDialog(parent), mOkButton(0)
{
    setWindowTitle(defaultTemplate ? i18n("Default template") : i18n("Template"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = 0;
    if (defaultTemplate) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveTemplateEditDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveTemplateEditDialog::reject);
    } else {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        mOkButton = buttonBox->button(QDialogButtonBox::Ok);
        mOkButton->setDefault(true);
        mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveTemplateEditDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveTemplateEditDialog::reject);
        mOkButton->setDefault(true);
    }
    QWidget *w = new QWidget;

    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Name:"));
    hbox->addWidget(label);

    mTemplateNameEdit = new QLineEdit;
    mTemplateNameEdit->setEnabled(!defaultTemplate);
    //QT5 mTemplateNameEdit->setTrapReturnKey(true);
    mTemplateNameEdit->setClearButtonEnabled(true);
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    mTextEdit = new KSieveUi::SieveTextEdit;
    mTextEdit->setShowHelpMenu(false);
    mTextEdit->setReadOnly(defaultTemplate);
    vbox->addWidget(mTextEdit);

    mFindBar = new PimCommon::PlainTextEditFindBar(mTextEdit, this);
    vbox->addWidget(mFindBar);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_F + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, this, &SieveTemplateEditDialog::slotFind);
    connect(mTextEdit, &SieveTextEdit::findText, this, &SieveTemplateEditDialog::slotFind);

    w->setLayout(vbox);
    mainLayout->addWidget(w);
    if (!defaultTemplate) {
        if (mOkButton) {
            mOkButton->setEnabled(false);
        }
        connect(mTemplateNameEdit, &QLineEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);
        connect(mTextEdit, &SieveTextEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);
        mTemplateNameEdit->setFocus();
    }
    mainLayout->addWidget(buttonBox);

    readConfig();
}

SieveTemplateEditDialog::~SieveTemplateEditDialog()
{
    writeConfig();
}

void SieveTemplateEditDialog::slotFind()
{
    if (mTextEdit->textCursor().hasSelection()) {
        mFindBar->setText(mTextEdit->textCursor().selectedText());
    }
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->show();
    mFindBar->focusAndSetCursor();
}

void SieveTemplateEditDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveTemplateEditDialog");
    group.writeEntry("Size", size());
}

void SieveTemplateEditDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveTemplateEditDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void SieveTemplateEditDialog::slotTemplateChanged()
{
    mOkButton->setEnabled(!mTemplateNameEdit->text().trimmed().isEmpty() && !mTextEdit->toPlainText().trimmed().isEmpty());
}

void SieveTemplateEditDialog::setScript(const QString &text)
{
    mTextEdit->setPlainText(text);
}

QString SieveTemplateEditDialog::script() const
{
    return mTextEdit->toPlainText();
}

void SieveTemplateEditDialog::setTemplateName(const QString &name)
{
    mTemplateNameEdit->setText(name);
}

QString SieveTemplateEditDialog::templateName() const
{
    return mTemplateNameEdit->text();
}

