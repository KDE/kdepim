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

#include "sievescriptdebuggerfrontendwidget.h"
#include "sievescriptdebuggerwarning.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextStream>
#include "editor/sievetexteditwidget.h"
#include "editor/sievetextedit.h"
#include <texteditor/plaintexteditor/plaintexteditorwidget.h>
#include <QLabel>
#include <KUrlRequester>
#include <KLineEdit>
#include <QDebug>

using namespace KSieveUi;

SieveScriptDebuggerFrontEndWidget::SieveScriptDebuggerFrontEndWidget(QWidget *parent)
    : QWidget(parent),
      mProcess(Q_NULLPTR)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    QHBoxLayout *emailPathLayout = new QHBoxLayout;
    mainLayout->addLayout(emailPathLayout);

    //KF5 add i18n
    QLabel *emailLab = new QLabel(QStringLiteral("Email path:"));
    emailLab->setObjectName(QStringLiteral("emaillab"));

    emailPathLayout->addWidget(emailLab);

    mEmailPath = new KUrlRequester(this);
    mEmailPath->setObjectName(QStringLiteral("emailpath"));
    emailPathLayout->addWidget(mEmailPath);
    connect(mEmailPath->lineEdit(), &KLineEdit::textChanged, this, &SieveScriptDebuggerFrontEndWidget::slotEmailChanged);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName(QStringLiteral("splitter"));
    mainLayout->addWidget(splitter);

    mSieveTextEditWidget = new KSieveUi::SieveTextEditWidget(this);
    mSieveTextEditWidget->setObjectName(QStringLiteral("sievetexteditwidget"));
    splitter->addWidget(mSieveTextEditWidget);
    splitter->setChildrenCollapsible(false);

    mSieveTestResult = new PimCommon::PlainTextEditorWidget(this);
    mSieveTestResult->setObjectName(QStringLiteral("sievetextresult"));
    mSieveTestResult->setReadOnly(true);
    splitter->addWidget(mSieveTestResult);


    mSieveScriptDebuggerWarning = new KSieveUi::SieveScriptDebuggerWarning(this);
    mSieveScriptDebuggerWarning->setObjectName(QStringLiteral("sievescriptdebuggerwarning"));
    mainLayout->addWidget(mSieveScriptDebuggerWarning);

    //KF5 add i18n
    mDebugScript = new QPushButton(QStringLiteral("Debug"));
    mDebugScript->setObjectName(QStringLiteral("debugbutton"));
    QHBoxLayout *debugButtonLayout = new QHBoxLayout;
    mainLayout->addLayout(debugButtonLayout);
    debugButtonLayout->addStretch();
    debugButtonLayout->addWidget(mDebugScript);
    mDebugScript->setEnabled(false);
    connect(mDebugScript, &QPushButton::clicked, this, &SieveScriptDebuggerFrontEndWidget::slotDebugScript);
}

SieveScriptDebuggerFrontEndWidget::~SieveScriptDebuggerFrontEndWidget()
{

}

void SieveScriptDebuggerFrontEndWidget::slotEmailChanged(const QString &text)
{
    mDebugScript->setEnabled(!text.trimmed().isEmpty());
}

void SieveScriptDebuggerFrontEndWidget::slotDebugScript()
{
    if (mSieveTextEditWidget->textEdit()->toPlainText().trimmed().isEmpty()) {
        //KF5 add i18n
        mSieveScriptDebuggerWarning->setErrorMessage(QStringLiteral("Script text is empty."));
        return;
    }
    if (!mEmailPath->url().isLocalFile()) {
        //KF5 add i18n improve it too
        mSieveScriptDebuggerWarning->setWarningMessage(QStringLiteral("Email file must be install in local."));
        return;
    }

    QTemporaryFile *temporaryFile = new QTemporaryFile();
    if (!temporaryFile->open()) {
        //KF5 add i18n
        mSieveScriptDebuggerWarning->setErrorMessage(QStringLiteral("Impossible to open temporary file."));
        return;
    }
    mSieveTestResult->clear();
    QTextStream stream(temporaryFile);
    stream << mSieveTextEditWidget->textEdit()->toPlainText();
    temporaryFile->flush();
    mProcess = new QProcess(this);
    temporaryFile->setParent(mProcess);
    mProcess->start(QStringLiteral("sieve-test"), QStringList() << temporaryFile->fileName() << mEmailPath->url().toLocalFile());
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &SieveScriptDebuggerFrontEndWidget::slotReadStandardOutput);
    connect(mProcess, &QProcess::readyReadStandardError, this, &SieveScriptDebuggerFrontEndWidget::slotReadErrorOutput);
    connect(mProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotDebugFinished()));
    //connect(mProcess, &QProcess::finished, this, &SieveScriptDebuggerFrontEndWidget::slotDebugFinished);
    if (!mProcess->waitForStarted()) {
        delete mProcess;
        mProcess = 0;
    }
    //Disable debug button;
    //TODO
}

void SieveScriptDebuggerFrontEndWidget::slotReadStandardOutput()
{
    const QByteArray result = mProcess->readAllStandardOutput();
    mSieveTestResult->editor()->appendPlainText(QString::fromLocal8Bit(result));
}

void SieveScriptDebuggerFrontEndWidget::slotReadErrorOutput()
{
    const QByteArray result = mProcess->readAllStandardError();
    mSieveTestResult->editor()->appendPlainText(QString::fromLocal8Bit(result));
}

void SieveScriptDebuggerFrontEndWidget::slotDebugFinished()
{
    delete mProcess;
    mProcess = 0;
}

QString SieveScriptDebuggerFrontEndWidget::script() const
{
    return mSieveTextEditWidget->textEdit()->toPlainText();
}

void SieveScriptDebuggerFrontEndWidget::setScript(const QString &script)
{
    mSieveTextEditWidget->textEdit()->setPlainText(script);
}

