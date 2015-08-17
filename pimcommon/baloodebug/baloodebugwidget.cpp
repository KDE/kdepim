/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "baloodebugwidget.h"
#include "job/baloodebugsearchjob.h"
#include "baloodebugsearchpathcombobox.h"
#include "baloosyntaxhighlighter.h"
#include <KLineEdit>
#include <QPushButton>

#include <QVBoxLayout>
#include <QLabel>

#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"

using namespace PimCommon;

BalooDebugWidget::BalooDebugWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    QLabel *lab = new QLabel(QStringLiteral("Item identifier:"));
    hbox->addWidget(lab);
    mLineEdit = new KLineEdit;
    mLineEdit->setTrapReturnKey(true);
    mLineEdit->setClearButtonShown(true);
    mLineEdit->setObjectName(QStringLiteral("lineedit"));
    connect(mLineEdit, &KLineEdit::textChanged, this, &BalooDebugWidget::slotSearchLineTextChanged);
    hbox->addWidget(mLineEdit);

    mSearchPathComboBox = new PimCommon::BalooDebugSearchPathComboBox;
    hbox->addWidget(mSearchPathComboBox);
    mSearchPathComboBox->setObjectName(QStringLiteral("searchpathcombo"));

    mSearchButton = new QPushButton(QStringLiteral("Search"));
    mSearchButton->setObjectName(QStringLiteral("searchbutton"));
    connect(mSearchButton, &QPushButton::clicked, this, &BalooDebugWidget::slotSearch);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    mPlainTextEditor = new PimCommon::PlainTextEditorWidget;
    new PimCommon::BalooSyntaxHighlighter(mPlainTextEditor->editor()->document());
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName(QStringLiteral("plaintexteditor"));

    connect(mLineEdit, &KLineEdit::returnPressed, this, &BalooDebugWidget::slotSearch);

}

BalooDebugWidget::~BalooDebugWidget()
{

}

void BalooDebugWidget::slotSearchLineTextChanged(const QString &text)
{
    mSearchButton->setEnabled(!text.trimmed().isEmpty());
}

void BalooDebugWidget::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    mLineEdit->setText(QString::number(akonadiId));
}

void BalooDebugWidget::setSearchType(BalooDebugSearchPathComboBox::SearchType type)
{
    mSearchPathComboBox->setSearchType(type);
}

void BalooDebugWidget::doSearch()
{
    slotSearch();
}

QString BalooDebugWidget::plainText() const
{
    return QStringLiteral("Item: %1\n").arg(mLineEdit->text()) + mPlainTextEditor->toPlainText();
}

void BalooDebugWidget::slotSearch()
{
    const QString searchId = mLineEdit->text();
    if (searchId.isEmpty()) {
        return;
    }
    PimCommon::BalooDebugSearchJob *job = new PimCommon::BalooDebugSearchJob(this);
    job->setAkonadiId(searchId);
    job->setSearchPath(mSearchPathComboBox->searchPath());
    connect(job, &PimCommon::BalooDebugSearchJob::result, this, &BalooDebugWidget::slotResult);
    connect(job, &PimCommon::BalooDebugSearchJob::error, this, &BalooDebugWidget::slotError);
    job->start();
}

void BalooDebugWidget::slotResult(const QString &result)
{
    mPlainTextEditor->setPlainText(result);
}

void BalooDebugWidget::slotError(const QString &errorStr)
{
    mPlainTextEditor->setPlainText(QStringLiteral("Error found:\n") + errorStr);
}
