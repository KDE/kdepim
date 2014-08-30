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

#include "templateeditdialog.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>


using namespace PimCommon;

TemplateEditDialog::TemplateEditDialog(QWidget *parent, bool defaultTemplate)
    : QDialog(parent), mOkButton(0)
{
    setWindowTitle( defaultTemplate ? i18n("Default template") : i18n("Template") );
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox=0;
    if (defaultTemplate) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &TemplateEditDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &TemplateEditDialog::reject);
    } else {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
        mOkButton = buttonBox->button(QDialogButtonBox::Ok);
        mOkButton->setDefault(true);
        mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &TemplateEditDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &TemplateEditDialog::reject);
        mOkButton->setFocus();
    }
    QWidget *w = new QWidget;
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Name:"));
    hbox->addWidget(label);

    mTemplateNameEdit = new QLineEdit;
    mTemplateNameEdit->setEnabled(!defaultTemplate);
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    mTextEdit = new PimCommon::RichTextEditorWidget;
    mTextEdit->setAcceptRichText(false);
    mTextEdit->setReadOnly(defaultTemplate);
    vbox->addWidget(mTextEdit);

    w->setLayout(vbox);
    if (!defaultTemplate) {
        if (mOkButton)
           mOkButton->setEnabled(false);
        connect(mTemplateNameEdit, &QLineEdit::textChanged, this, &TemplateEditDialog::slotTemplateChanged);
        connect(mTextEdit->editor(), SIGNAL(textChanged()),SLOT(slotTemplateChanged()));
        mTemplateNameEdit->setFocus();
    }
    readConfig();
}

TemplateEditDialog::~TemplateEditDialog()
{
    writeConfig();
}


void TemplateEditDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "TemplateEditDialog" );
    group.writeEntry( "Size", size() );
}

void TemplateEditDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "TemplateEditDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}


void TemplateEditDialog::slotTemplateChanged()
{
    mOkButton->setEnabled(!mTemplateNameEdit->text().trimmed().isEmpty() && !mTextEdit->editor()->toPlainText().trimmed().isEmpty());
}

void TemplateEditDialog::setScript(const QString &text)
{
    mTextEdit->setPlainText(text);
}

QString TemplateEditDialog::script() const
{
    return mTextEdit->toPlainText();
}

void TemplateEditDialog::setTemplateName(const QString &name)
{
    mTemplateNameEdit->setText(name);
}

QString TemplateEditDialog::templateName() const
{
    return mTemplateNameEdit->text();
}

