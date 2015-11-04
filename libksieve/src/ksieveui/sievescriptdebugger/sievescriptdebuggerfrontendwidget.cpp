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
#include "sievescriptdebuggerresulteditor.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTemporaryFile>
#include <QProcess>
#include <QTextStream>
#include <QFormLayout>
#include "editor/sievetexteditwidget.h"
#include "editor/sievetextedit.h"
#include <kpimtextedit/plaintexteditorwidget.h>
#include <kpimtextedit/texttospeechwidget.h>
#include <QLabel>
#include <KUrlRequester>
#include <KLineEdit>
#include <KLocalizedString>
#include <QDebug>
#include <QDate>

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
    mExtension->setObjectName(QStringLiteral("extension"));
    mExtension->setPlaceholderText(i18n("Activate extension with \"+<name of extension>\", deactivate it with \"-<name of extension>\""));
    mExtension->setClearButtonEnabled(true);
    mExtension->setTrapReturnKey(true);

    formLayout->addRow(extensionLab, mExtension);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName(QStringLiteral("splitter"));
    mainLayout->addWidget(splitter);

    QWidget *sieveEditorWidget = new QWidget(this);
    QVBoxLayout *vboxSieveEditorLayout = new QVBoxLayout;
    sieveEditorWidget->setLayout(vboxSieveEditorLayout);
    vboxSieveEditorLayout->setMargin(0);

    KPIMTextEdit::TextToSpeechWidget *textToSpeechWidget = new KPIMTextEdit::TextToSpeechWidget(this);
    textToSpeechWidget->setObjectName(QStringLiteral("texttospeechwidget"));
    vboxSieveEditorLayout->addWidget(textToSpeechWidget);

    mSieveTextEditWidget = new KSieveUi::SieveTextEditWidget(new KSieveUi::SieveScriptDebuggerTextEdit(this), this);
    mSieveTextEditWidget->setObjectName(QStringLiteral("sievetexteditwidget"));
    vboxSieveEditorLayout->addWidget(mSieveTextEditWidget);
    connect(mSieveTextEditWidget->textEdit(), &SieveTextEdit::say, textToSpeechWidget, &KPIMTextEdit::TextToSpeechWidget::say);

    splitter->addWidget(sieveEditorWidget);
    splitter->setChildrenCollapsible(false);

    mSieveTestResult = new KPIMTextEdit::PlainTextEditorWidget(new KSieveUi::SieveScriptDebuggerResultEditor(this), this);
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
        mSieveScriptDebuggerWarning->setWarningMessage(i18n("Email file must be installed locally."));
        return;
    }

    QTemporaryFile *temporaryFile = new QTemporaryFile();
    if (!temporaryFile->open()) {
        mSieveScriptDebuggerWarning->setErrorMessage(i18n("Impossible to open temporary file."));
        return;
    }
    mDebugScript->setEnabled(false);
    QTextStream stream(temporaryFile);
    stream << mSieveTextEditWidget->textEdit()->toPlainText();
    temporaryFile->flush();
    mProcess = new QProcess(this);
    temporaryFile->setParent(mProcess);

    QString extensionList;
    if (!mExtension->text().trimmed().isEmpty()) {
        extensionList = QStringLiteral("-x \"%1\"").arg(mExtension->text());
    }

    QStringList arguments;
    if (!extensionList.isEmpty()) {
        arguments << extensionList;
    }

    arguments << temporaryFile->fileName() << mEmailPath->url().toLocalFile();
    mProcess->start(QStringLiteral("sieve-test"), QStringList() << arguments);
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &SieveScriptDebuggerFrontEndWidget::slotReadStandardOutput);
    connect(mProcess, &QProcess::readyReadStandardError, this, &SieveScriptDebuggerFrontEndWidget::slotReadErrorOutput);
    connect(mProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotDebugFinished()));
    //TODO port to new connect api
    //connect(mProcess, &QProcess::finished, this, &SieveScriptDebuggerFrontEndWidget::slotDebugFinished);
    mSieveTestResult->editor()->appendPlainText(QStringLiteral("--------------------------------------"));
    mSieveTestResult->editor()->appendPlainText(QLocale().toString(QDateTime::currentDateTime()));
    mSieveTestResult->editor()->appendPlainText(QStringLiteral("\n"));
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
    mSieveTextEditWidget->textEdit()->verticalScrollBar()->setValue(mSieveTextEditWidget->textEdit()->verticalScrollBar()->maximum());
}

bool SieveScriptDebuggerFrontEndWidget::canAccept() const
{
    const QString script = mSieveTextEditWidget->textEdit()->toPlainText();
    if (script.contains(QStringLiteral("debug_log")) || script.contains(QStringLiteral("vnd.dovecot.debug"))) {
        mSieveScriptDebuggerWarning->setErrorMessage(i18n("Script still contains debug method. Remove it please."));
        return false;
    }
    mSieveScriptDebuggerWarning->hide();
    return true;
}

