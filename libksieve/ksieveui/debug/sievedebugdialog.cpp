/*
    sievedebugdialog.cpp

    Copyright (c) 2005 Martijn Klingens <klingens@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "sievedebugdialog.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"

#include <agentinstance.h>
#include <qdebug.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util/util.h>

#include <QtCore/QTimer>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KSieveUi;

SieveDebugDialog::SieveDebugDialog(QWidget *parent)
    : QDialog(parent),
      mSieveJob(0)
{
    setWindowTitle(i18n("Sieve Diagnostics"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveDebugDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveDebugDialog::reject);

    // Collect all accounts
    const Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    foreach(const Akonadi::AgentInstance & type, lst) {
        mResourceIdentifier << type.identifier();
    }

    mEdit = new PimCommon::PlainTextEditorWidget(this);
    mEdit->setReadOnly(true);
    mainLayout->addWidget(mEdit);
    mainLayout->addWidget(buttonBox);

    if (!mResourceIdentifier.isEmpty()) {
        mEdit->editor()->setPlainText(i18n("Collecting diagnostic information about Sieve support...\n\n"));
        QTimer::singleShot(0, this, SLOT(slotDiagNextAccount()));
    } else {
        mEdit->editor()->setPlainText(i18n("No Imap Resource found."));
    }
    readConfig();
}

SieveDebugDialog::~SieveDebugDialog()
{
    if (mSieveJob) {
        mSieveJob->kill();
        mSieveJob = 0;
    }
    qDebug();
    writeConfig();
}

void SieveDebugDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(640, 480));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void SieveDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveDebugDialog");
    group.writeEntry("Size", size());
}

void SieveDebugDialog::slotDiagNextAccount()
{
    if (mResourceIdentifier.isEmpty()) {
        return;
    }
    QString ident = mResourceIdentifier.first();

    mEdit->editor()->appendPlainText(i18n("Collecting data for account '%1'...\n", ident));
    mEdit->editor()->appendPlainText(i18n("------------------------------------------------------------\n"));

    // Detect URL for this IMAP account
    const QUrl url = KSieveUi::Util::findSieveUrlForAccount(ident);
    if (!url.isValid()) {
        mEdit->editor()->appendPlainText(i18n("(Account does not support Sieve)\n\n"));
    } else {
        mUrl = url;

        mSieveJob = KManageSieve::SieveJob::list(mUrl);

        connect(mSieveJob, &KManageSieve::SieveJob::gotList, this, &SieveDebugDialog::slotGetScriptList);

        // Bypass the singleShot timer -- it's fired when we get our data
        return;
    }

    // Handle next account async
    mResourceIdentifier.pop_front();
    QTimer::singleShot(0, this, SLOT(slotDiagNextAccount()));
}

void SieveDebugDialog::slotDiagNextScript()
{
    if (mScriptList.isEmpty()) {
        // Continue handling accounts instead
        mScriptList.clear();
        mResourceIdentifier.pop_front();
        QTimer::singleShot(0, this, SLOT(slotDiagNextAccount()));
        return;
    }

    QString scriptFile = mScriptList.first();
    mScriptList.pop_front();

    mEdit->editor()->appendPlainText(i18n("Contents of script '%1':\n", scriptFile));

    mUrl = KSieveUi::Util::findSieveUrlForAccount(mResourceIdentifier.first());

    mUrl = mUrl.adjusted(QUrl::RemoveFilename);
    mUrl.setPath(mUrl.path() + scriptFile);

    mSieveJob = KManageSieve::SieveJob::get(mUrl);

    connect(mSieveJob, &KManageSieve::SieveJob::gotScript, this, &SieveDebugDialog::slotGetScript);
}

void SieveDebugDialog::slotGetScript(KManageSieve::SieveJob * /* job */, bool success,
                                     const QString &script, bool active)
{
    qDebug() << "( ??," << success
             << ", ?," << active << ")" << endl
             << "script:" << endl
             << script;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    if (script.isEmpty()) {
        mEdit->editor()->appendPlainText(i18n("(This script is empty.)\n\n"));
    } else {
        mEdit->editor()->appendPlainText(i18n(
                                             "------------------------------------------------------------\n"
                                             "%1\n"
                                             "------------------------------------------------------------\n\n", script));
    }

    // Fetch next script
    QTimer::singleShot(0, this, SLOT(slotDiagNextScript()));
}

void SieveDebugDialog::slotGetScriptList(KManageSieve::SieveJob *job, bool success,
        const QStringList &scriptList, const QString &activeScript)
{
    qDebug() << "Success:" << success << ", List:" << scriptList.join(QLatin1String(",")) <<
             ", active:" << activeScript;
    mSieveJob = 0; // job deletes itself after returning from this slot!

    mEdit->editor()->appendPlainText(i18n("Sieve capabilities:\n"));
    const QStringList caps = job->sieveCapabilities();
    if (caps.isEmpty()) {
        mEdit->editor()->appendPlainText(i18n("(No special capabilities available)"));
    } else {
        QStringList::const_iterator end = caps.constEnd();
        for (QStringList::const_iterator it = caps.constBegin(); it != end; ++it) {
            mEdit->editor()->appendPlainText(QLatin1String("* ") + *it + QLatin1Char('\n'));
        }
        mEdit->editor()->appendPlainText(QLatin1String("\n"));
    }

    mEdit->editor()->appendPlainText(i18n("Available Sieve scripts:\n"));

    if (scriptList.isEmpty()) {
        mEdit->editor()->appendPlainText(i18n("(No Sieve scripts available on this server)\n\n"));
    } else {
        mScriptList = scriptList;
        QStringList::const_iterator end = scriptList.constEnd();
        for (QStringList::const_iterator it = scriptList.constBegin(); it != end; ++it) {
            mEdit->editor()->appendPlainText(QLatin1String("* ") + *it + QLatin1Char('\n'));
        }
        mEdit->editor()->appendPlainText(QLatin1String("\n"));
        mEdit->editor()->appendPlainText(i18n("Active script: %1\n\n", activeScript));
    }

    // Handle next job: dump scripts for this server
    QTimer::singleShot(0, this, SLOT(slotDiagNextScript()));
}

