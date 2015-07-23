/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "sievescriptdebuggerwidget.h"
#include "sievescriptdebuggerfontendwidget.h"

#include <KLocalizedString>

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardPaths>

using namespace KSieveUi;
SieveScriptDebuggerWidget::SieveScriptDebuggerWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);
    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName(QStringLiteral("stackedwidget"));
    mainLayout->addWidget(mStackedWidget);

    mSieveScriptFrontEnd = new SieveScriptDebuggerFontEndWidget;
    mSieveScriptFrontEnd->setObjectName(QStringLiteral("sievescriptfrontend"));
    mStackedWidget->addWidget(mSieveScriptFrontEnd);

    // kf5.1 add i18n
    mSieveNoExistingFrontEnd = new QLabel(QStringLiteral("sieve-test was not found on system. Please install it."));
    mSieveNoExistingFrontEnd->setObjectName(QStringLiteral("sievenoexistingfrontend"));
    mStackedWidget->addWidget(mSieveNoExistingFrontEnd);

    checkSieveTestApplication();
}

SieveScriptDebuggerWidget::~SieveScriptDebuggerWidget()
{

}

void SieveScriptDebuggerWidget::setScript(const QString &script)
{
    if (mStackedWidget->currentWidget() == mSieveScriptFrontEnd) {
        mSieveScriptFrontEnd->setScript(script);
    }
}

QString SieveScriptDebuggerWidget::script() const
{
    if (mStackedWidget->currentWidget() == mSieveScriptFrontEnd) {
        return mSieveScriptFrontEnd->script();
    } else {
        return QString();
    }
}

void SieveScriptDebuggerWidget::checkSieveTestApplication()
{
    if (QStandardPaths::findExecutable(QStringLiteral("sieve-test")).isEmpty()) {
        mStackedWidget->setCurrentWidget(mSieveNoExistingFrontEnd);
    } else {
        mStackedWidget->setCurrentWidget(mSieveScriptFrontEnd);
    }
}
