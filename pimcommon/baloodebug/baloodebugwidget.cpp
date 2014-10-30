/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "baloodebugsearchjob.h"
#include <KLineEdit>
#include <QPushButton>

#include <QVBoxLayout>
#include <QLabel>

#include <texteditor/plaintexteditor/plaintexteditorwidget.h>

using namespace PimCommon;

BalooDebugWidget::BalooDebugWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QHBoxLayout *hbox = new QHBoxLayout;
    mainLayout->addLayout(hbox);
    QLabel *lab = new QLabel(QLatin1String("Item identifier:"));
    hbox->addWidget(lab);
    mLineEdit = new KLineEdit;
    mLineEdit->setObjectName(QLatin1String("lineedit"));
    connect(mLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotSearchLineTextChanged(QString)));
    hbox->addWidget(mLineEdit);

    mSearchButton = new QPushButton(QLatin1String("Search"));
    mSearchButton->setObjectName(QLatin1String("searchbutton"));
    connect(mSearchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    hbox->addWidget(mSearchButton);
    mSearchButton->setEnabled(false);

    mPlainTextEditor = new PimCommon::PlainTextEditorWidget;
    mPlainTextEditor->setReadOnly(true);
    mainLayout->addWidget(mPlainTextEditor);
    mPlainTextEditor->setObjectName(QLatin1String("plaintexteditor"));
}

BalooDebugWidget::~BalooDebugWidget()
{

}

void BalooDebugWidget::slotSearchLineTextChanged(const QString &text)
{
    mSearchButton->setEnabled(!text.trimmed().isEmpty());
}

void BalooDebugWidget::setAkonadiId(const QString &akonadiId)
{
    mLineEdit->setText(akonadiId);
}

void BalooDebugWidget::slotSearch()
{
    PimCommon::BalooDebugSearchJob *job = new PimCommon::BalooDebugSearchJob(this);
    job->setAkonadiId(mLineEdit->text());
    connect(job, SIGNAL(result(QString)), this, SLOT(slotResult(QString)));
}

void BalooDebugWidget::slotResult(const QString &result)
{
    mPlainTextEditor->setPlainText(result);
}
