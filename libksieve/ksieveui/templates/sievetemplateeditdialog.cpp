/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "editor/sievefindbar.h"
#include "editor/sievetextedit.h"

#include <KLocale>
#include <KLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QShortcut>


using namespace KSieveUi;

SieveTemplateEditDialog::SieveTemplateEditDialog(QWidget *parent, bool defaultTemplate)
    : KDialog(parent)
{
    setCaption( i18n("Templates") );
    if (defaultTemplate)
        setButtons( Close );
    else
        setButtons( Ok |Cancel );

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Name:"));
    hbox->addWidget(label);

    mTemplateNameEdit = new KLineEdit;
    mTemplateNameEdit->setEnabled(!defaultTemplate);
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    mTextEdit = new KSieveUi::SieveTextEdit;
    mTextEdit->setReadOnly(defaultTemplate);
    vbox->addWidget(mTextEdit);

    mFindBar = new SieveFindBar( mTextEdit, this );
    vbox->addWidget(mFindBar);

    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_F+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
    connect( mTextEdit, SIGNAL(findText()), SLOT(slotFind()) );

    w->setLayout(vbox);
    setMainWidget(w);
    if (!defaultTemplate) {
        enableButtonOk(false);
        connect(mTemplateNameEdit, SIGNAL(textChanged(QString)),SLOT(slotTemplateNameChanged(QString)));
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
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize(600,400);
    }
}

void SieveTemplateEditDialog::slotTemplateNameChanged(const QString &text)
{
    enableButtonOk(!text.trimmed().isEmpty());
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

#include "sievetemplateeditdialog.moc"
