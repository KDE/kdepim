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
#include "sievetextedit.h"

#include <KLocale>
#include <KLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>


using namespace KSieveUi;

SieveTemplateEditDialog::SieveTemplateEditDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Templates") );
    setButtons( Ok |Close );

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Name:"));
    hbox->addWidget(label);

    mTemplateNameEdit = new KLineEdit;
    hbox->addWidget(mTemplateNameEdit);

    vbox->addLayout(hbox);

    mTextEdit = new KSieveUi::SieveTextEdit;
    vbox->addWidget(mTextEdit);

    setMainWidget(w);
    connect(mTemplateNameEdit, SIGNAL(textChanged(QString)),SLOT(slotTemplateNameChanged(QString)));
}

SieveTemplateEditDialog::~SieveTemplateEditDialog()
{

}

void SieveTemplateEditDialog::slotTemplateNameChanged(const QString &text)
{
    enableButtonOk(!text.trimmed().isEmpty());
}

void SieveTemplateEditDialog::setText(const QString &text)
{
    mTextEdit->setPlainText(text);
}

QString SieveTemplateEditDialog::text() const
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
