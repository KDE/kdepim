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
#include "sievescriptdebuggertextedit.h"
#include "sievescriptdebuggerwarning.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextStream>
#include <QFormLayout>
#include "editor/sievetexteditwidget.h"
#include "editor/sievetextedit.h"
#include <texteditor/plaintexteditor/plaintexteditorwidget.h>
#include <QLabel>
#include <KUrlRequester>
#include <KLineEdit>
#include <KLocalizedString>
#include <QDebug>

using namespace KSieveUi;

SieveScriptDebuggerFrontEndWidget::SieveScriptDebuggerFrontEndWidget(QWidget *parent)
    : QWidget(parent),
      mProcess(Q_NULLPTR)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    QLabel *emailLab = new QLabel(i18n("Email path:"));
    emailLab->setObjectName(QStringLiteral("emaillab"));

    mEmailPath = new KUrlRequester(this);
    mEmailPath->setObjectName(QStringLiteral("emailpath"));
    mEmailPath->lineEdit()->setTrapReturnKey(true);
    mEmailPath->lineEdit()->setClearButtonEnabled(true);
    connect(mEmailPath->lineEdit(), &KLineEdit::textChanged, this, &SieveScriptDebuggerFrontEndWidget::slotEmailChanged);

    formLayout->addRow(emailLab, mEmailPath);

    QHBoxLayout *extensionLayout = new QHBoxLayout;
    mainLayout->addLayout(extensionLayout);

    QLabel *extensionLab = new QLabel(i18n("Extension:"));
    extensionLab->setObjectName(QStringLiteral("extensionlab"));

    mExtension = new KLineEdit(this);
    //add placeholderText
    mExtension->setObjectName(QStringLiteral("extension"));
    mExtension->setPlaceholderText(i18n("Activate extension with \"+<name of extension\", desactivate it with \"-<name of extension\"."));
    mExtension->setClearButtonEnabled(true);

    formLayout->addRow(extensionLab, mExtension);


    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName(QStringLiteral("splitter"));
    mainLayout->addWidget(splitter);

    mSieveTextEditWidget = new KSieveUi::SieveTextEditWidget(new KSieveUi::SieveScriptDebuggerTextEdit(this), this);
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

    mDebugScript = new QPushButton(i18n("Debug"));
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
        mSieveScriptDebuggerWarning->setErrorMessage(i18n("Script text is empty."));
        return;
    }
    if (!mEmailPath->url().isLocalFile()) {
        mSieveScriptDebuggerWarning->setWarningMessage(i18n("Email file must be install in local."));
        return;
    }

    QTemporaryFile *temporaryFile = new QTemporaryFile();
    if (!temporaryFile->open()) {
        mSieveScriptDebuggerWarning->setErrorMessage(i18n("Impossible to open temporary file."));
        return;
    }
    mDebugScript->setEnabled(false);
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
    //TODO port to new connect api
    //connect(mProcess, &QProcess::finished, this, &SieveScriptDebuggerFrontEndWidget::slotDebugFinished);
    if (!mProcess->waitForStarted()) {
        delete mProcess;
        mProcess = 0;
        mDebugScript->setEnabled(true);
    }
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
    mDebugScript->setEnabled(true);
}

QString SieveScriptDebuggerFrontEndWidget::script() const
{
    return mSieveTextEditWidget->textEdit()->toPlainText();
}

void SieveScriptDebuggerFrontEndWidget::setScript(const QString &script)
{
    mSieveTextEditWidget->textEdit()->setPlainText(script);
}

