/***************************************************************************
 *   snippet feature from kdevelop/plugins/snippet/                        *
 *                                                                         *
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetdialog_p.h"

#include "ui_snippetdialog.h"

#include <kactioncollection.h>
#include <QDialog>
#include <KLocalizedString>
#include <KConfigGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

using namespace MailCommon;

SnippetDialog::SnippetDialog(KActionCollection *actionCollection, bool inGroupMode, QWidget *parent)
    : QDialog(parent), mActionCollection(actionCollection)
{
    mUi = new Ui::SnippetDialog;
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SnippetDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SnippetDialog::reject);

    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mainLayout->addWidget(buttonBox);
    mUi->setupUi(mainWidget);

    mUi->keyWidget->setCheckActionCollections(QList<KActionCollection *>() << actionCollection);
    mOkButton->setEnabled(false);

    connect(mUi->nameEdit, &KLineEdit::textChanged, this, &SnippetDialog::slotTextChanged);
    connect(mUi->groupBox, static_cast<void (KComboBox::*)(const QString &)>(&KComboBox::currentIndexChanged), this, &SnippetDialog::slotGroupChanged);

    mUi->snippetText->setMinimumSize(500, 300);

    mUi->groupWidget->setVisible(!inGroupMode);
    mUi->nameEdit->setFocus();
}

SnippetDialog::~SnippetDialog()
{
    delete mUi;
}

void SnippetDialog::slotGroupChanged()
{
    mOkButton->setEnabled(snippetIsValid());
}

void SnippetDialog::setName(const QString &name)
{
    mUi->nameEdit->setText(name);
}

QString SnippetDialog::name() const
{
    return mUi->nameEdit->text();
}

void SnippetDialog::setText(const QString &text)
{
    mUi->snippetText->setText(text);
}

QString SnippetDialog::text() const
{
    return mUi->snippetText->toPlainText();
}

void SnippetDialog::setKeySequence(const QKeySequence &sequence)
{
    mUi->keyWidget->setKeySequence(sequence);
}

QKeySequence SnippetDialog::keySequence() const
{
    return mUi->keyWidget->keySequence();
}

void SnippetDialog::setGroupModel(QAbstractItemModel *model)
{
    mUi->groupBox->setModel(model);
}

void SnippetDialog::setGroupIndex(const QModelIndex &index)
{
    mUi->groupBox->setCurrentIndex(index.row());
}

QModelIndex SnippetDialog::groupIndex() const
{
    return mUi->groupBox->model()->index(mUi->groupBox->currentIndex(), 0);
}

void SnippetDialog::slotTextChanged()
{
    mOkButton->setEnabled(snippetIsValid());
}

bool SnippetDialog::snippetIsValid() const
{
    if (mUi->groupWidget->isVisible())
        return (!mUi->nameEdit->text().trimmed().isEmpty() &&
                !mUi->groupBox->currentText().trimmed().isEmpty());
    else {
        return (!mUi->nameEdit->text().trimmed().isEmpty());
    }
}

#include "moc_snippetdialog_p.cpp"
