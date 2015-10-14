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

#include "akonadisearchdebugwidget.h"
#include "job/akonadisearchdebugsearchjob.h"
#include "akonadisearchdebugsearchpathcombobox.h"
#include "akonadisearchsyntaxhighlighter.h"
#include <KLineEdit>
#include <QPushButton>

#include <QVBoxLayout>
#include <QLabel>

#include "texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "texteditor/plaintexteditor/plaintexteditor.h"

using namespace PimCommon;

AkonadiSearchDebugWidget::AkonadiSearchDebugWidget(QWidget *parent)
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
    connect(mLineEdit, &KLineEdit::textChanged, this, &AkonadiSearchDebugWidget::slotSearchLineTextChanged);
    hbox->addWidget(mLineEdit);

    mSearchPathComboBox = new PimCommon::AkonadiSearchDebugSearchPathComboBox;
    hbox->addWidget(mSearchPathComboBox);
    mSearchPathComboBox->setObjectName(QStringLiteral("searchpathcombo"));

    mSearchButton = new QPushButton(QStringLiteral("Search"));
    mSearchButton->setObjectName(QStringLiteral("searchbutton"));
    connect(mSearchButton, &QPushButton::clicked, this, &AkonadiSearchDebugWidget::slotSearch);
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    mPlainTextEditor = new PimCommon::PlainTextEditorWidget;
    new PimCommon::AkonadiSearchSyntaxHighlighter(mPlainTextEditor->editor()->document());
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName(QStringLiteral("plaintexteditor"));

    connect(mLineEdit, &KLineEdit::returnPressed, this, &AkonadiSearchDebugWidget::slotSearch);

}

AkonadiSearchDebugWidget::~AkonadiSearchDebugWidget()
{

}

void AkonadiSearchDebugWidget::slotSearchLineTextChanged(const QString &text)
{
    mSearchButton->setEnabled(!text.trimmed().isEmpty());
}

void AkonadiSearchDebugWidget::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    mLineEdit->setText(QString::number(akonadiId));
}

void AkonadiSearchDebugWidget::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    mSearchPathComboBox->setSearchType(type);
}

void AkonadiSearchDebugWidget::doSearch()
{
    slotSearch();
}

QString AkonadiSearchDebugWidget::plainText() const
{
    return QStringLiteral("Item: %1\n").arg(mLineEdit->text()) + mPlainTextEditor->toPlainText();
}

void AkonadiSearchDebugWidget::slotSearch()
{
    const QString searchId = mLineEdit->text();
    if (searchId.isEmpty()) {
        return;
    }
    PimCommon::AkonadiSearchDebugSearchJob *job = new PimCommon::AkonadiSearchDebugSearchJob(this);
    job->setAkonadiId(searchId);
    job->setSearchPath(mSearchPathComboBox->searchPath());
    connect(job, &PimCommon::AkonadiSearchDebugSearchJob::result, this, &AkonadiSearchDebugWidget::slotResult);
    connect(job, &PimCommon::AkonadiSearchDebugSearchJob::error, this, &AkonadiSearchDebugWidget::slotError);
    job->start();
}

void AkonadiSearchDebugWidget::slotResult(const QString &result)
{
    mPlainTextEditor->setPlainText(result);
}

void AkonadiSearchDebugWidget::slotError(const QString &errorStr)
{
    mPlainTextEditor->setPlainText(QStringLiteral("Error found:\n") + errorStr);
}
