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

#include "sievetemplateeditdialog.h"
#include "editor/sievetextedit.h"

#include "kpimtextedit/plaintexteditfindbar.h"
#include "kpimtextedit/slidecontainer.h"

#include <KLocalizedString>
#include <KLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShortcut>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

#include <editor/sievetexteditwidget.h>

#include <kpimtextedit/texttospeechwidget.h>

using namespace KSieveUi;

SieveTemplateEditDialog::SieveTemplateEditDialog(QWidget *parent, bool defaultTemplate)
    : QDialog(parent), mOkButton(Q_NULLPTR)
{
    setWindowTitle(defaultTemplate ? i18n("Default template") : i18n("Template"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = Q_NULLPTR;
    if (defaultTemplate) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
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

    mTemplateNameEdit = new KLineEdit;
    mTemplateNameEdit->setEnabled(!defaultTemplate);
    mTemplateNameEdit->setTrapReturnKey(true);
    mTemplateNameEdit->setClearButtonEnabled(!defaultTemplate);
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    KPIMTextEdit::TextToSpeechWidget *textToSpeechWidget = new KPIMTextEdit::TextToSpeechWidget(this);
    vbox->addWidget(textToSpeechWidget);

    mTextEditWidget = new KSieveUi::SieveTextEditWidget;
    mTextEditWidget->textEdit()->setShowHelpMenu(false);
    mTextEditWidget->setReadOnly(defaultTemplate);
    vbox->addWidget(mTextEditWidget);
    connect(mTextEditWidget->textEdit(), &SieveTextEdit::say, textToSpeechWidget, &KPIMTextEdit::TextToSpeechWidget::say);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_F + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, mTextEditWidget, &SieveTextEditWidget::slotFind);

    shortcut = new QShortcut(this);
    shortcut->setKey(Qt::Key_R + Qt::CTRL);
    connect(shortcut, &QShortcut::activated, mTextEditWidget, &SieveTextEditWidget::slotReplace);

    w->setLayout(vbox);
    mainLayout->addWidget(w);
    if (!defaultTemplate) {
        if (mOkButton) {
            mOkButton->setEnabled(false);
        }
        connect(mTemplateNameEdit, &QLineEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);
        connect(mTextEditWidget->textEdit(), &SieveTextEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);
        mTemplateNameEdit->setFocus();
    }
    mainLayout->addWidget(buttonBox);

    readConfig();
}

SieveTemplateEditDialog::~SieveTemplateEditDialog()
{
    writeConfig();
    disconnect(mTemplateNameEdit, &QLineEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);
    disconnect(mTextEditWidget->textEdit(), &SieveTextEdit::textChanged, this, &SieveTemplateEditDialog::slotTemplateChanged);

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
    mOkButton->setEnabled(!mTemplateNameEdit->text().trimmed().isEmpty() && !mTextEditWidget->textEdit()->toPlainText().trimmed().isEmpty());
}

void SieveTemplateEditDialog::setScript(const QString &text)
{
    mTextEditWidget->textEdit()->setPlainText(text);
}

QString SieveTemplateEditDialog::script() const
{
    return mTextEditWidget->textEdit()->toPlainText();
}

void SieveTemplateEditDialog::setSieveCapabilities(const QStringList &capabilities)
{
    mTextEditWidget->textEdit()->setSieveCapabilities(capabilities);
}

void SieveTemplateEditDialog::setTemplateName(const QString &name)
{
    mTemplateNameEdit->setText(name);
}

QString SieveTemplateEditDialog::templateName() const
{
    return mTemplateNameEdit->text();
}

