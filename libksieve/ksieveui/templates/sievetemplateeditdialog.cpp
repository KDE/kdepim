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
#include <KLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShortcut>


using namespace KSieveUi;

SieveTemplateEditDialog::SieveTemplateEditDialog(QWidget *parent, bool defaultTemplate)
    : KDialog(parent)
{
    setCaption( defaultTemplate ? i18n("Default template") : i18n("Template") );
    if (defaultTemplate) {
        setButtons( Close );
    } else {
        setButtons( Ok |Cancel );
    }
    setDefaultButton(Ok);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Name:"));
    hbox->addWidget(label);

    mTemplateNameEdit = new KLineEdit;
    mTemplateNameEdit->setEnabled(!defaultTemplate);
    mTemplateNameEdit->setTrapReturnKey(true);
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    mTextEdit = new KSieveUi::SieveTextEdit;
    mTextEdit->setReadOnly(defaultTemplate);
    vbox->addWidget(mTextEdit);

    mFindBar = new PimCommon::PlainTextEditFindBar( mTextEdit, this );
    vbox->addWidget(mFindBar);

    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_F+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
    connect( mTextEdit, SIGNAL(findText()), SLOT(slotFind()) );

    w->setLayout(vbox);
    setMainWidget(w);
    if (!defaultTemplate) {
        enableButtonOk(false);
        connect(mTemplateNameEdit, SIGNAL(textChanged(QString)),SLOT(slotTemplateChanged()));
        connect(mTextEdit, SIGNAL(textChanged()),SLOT(slotTemplateChanged()));
        mTemplateNameEdit->setFocus();
    }
    readConfig();
}

SieveTemplateEditDialog::~SieveTemplateEditDialog()
{
    writeConfig();
}

void SieveTemplateEditDialog::slotFind()
{
    if ( mTextEdit->textCursor().hasSelection() )
        mFindBar->setText( mTextEdit->textCursor().selectedText() );
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->show();
    mFindBar->focusAndSetCursor();
}


void SieveTemplateEditDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveTemplateEditDialog" );
    group.writeEntry( "Size", size() );
}

void SieveTemplateEditDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveTemplateEditDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void SieveTemplateEditDialog::slotTemplateChanged()
{
    enableButtonOk(!mTemplateNameEdit->text().trimmed().isEmpty() && !mTextEdit->toPlainText().trimmed().isEmpty());
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

