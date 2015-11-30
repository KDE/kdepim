/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "adblockshowlistdialog.h"
#include "adblocksyntaxhighlighter.h"
#include "kpimtextedit/plaintexteditorwidget.h"
#include "kpimtextedit/plaintexteditor.h"
#include "messageviewer_debug.h"
#include "Libkdepim/ProgressIndicatorLabel"

#include <KLocalizedString>
#include <KIO/Job>
#include <QTemporaryFile>
#include <KSharedConfig>
#include <QUrl>

#include <QHBoxLayout>

#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace MessageViewer;
AdBlockShowListDialog::AdBlockShowListDialog(QWidget *parent)
    : QDialog(parent),
      mTemporaryFile(Q_NULLPTR)
{
    setWindowTitle(i18n("Show adblock list"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mUser1Button = new QPushButton;
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AdBlockShowListDialog::reject);
    mUser1Button->setText(i18n("Delete List"));
    mUser1Button->setEnabled(false);
    connect(mUser1Button, &QPushButton::clicked, this, &AdBlockShowListDialog::slotDeleteBrokenList);
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mTextEdit = new KPIMTextEdit::PlainTextEditorWidget;
    (void)new MessageViewer::AdBlockSyntaxHighlighter(mTextEdit->editor()->document());
    mTextEdit->setReadOnly(true);
    lay->addWidget(mTextEdit);

    mProgress = new KPIM::ProgressIndicatorLabel(i18n("Download..."));
    lay->addWidget(mProgress);
    w->setLayout(lay);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

AdBlockShowListDialog::~AdBlockShowListDialog()
{
    delete mTemporaryFile;
    writeConfig();
}

void AdBlockShowListDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AdBlockShowListDialog");
    group.writeEntry("Size", size());
}

void AdBlockShowListDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AdBlockShowListDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void AdBlockShowListDialog::setAdBlockListPath(const QString &localPath, const QString &url)
{
    if (localPath.isEmpty()) {
        QFile file(localPath);
        if (file.exists()) {
            mTextEdit->editor()->setPlainText(QString::fromUtf8(file.readAll()));
        } else {
            downLoadList(url);
        }
    } else {
        downLoadList(url);
    }
}

void AdBlockShowListDialog::downLoadList(const QString &url)
{
    delete mTemporaryFile;
    mTemporaryFile = new QTemporaryFile;
    if (!mTemporaryFile->open()) {
        qCDebug(MESSAGEVIEWER_LOG) << "can not open temporary file";
        delete mTemporaryFile;
        mTemporaryFile = 0;
        return;
    }
    QUrl subUrl(url);

    QUrl destUrl = QUrl::fromLocalFile(mTemporaryFile->fileName());

    mProgress->start();
    KIO::FileCopyJob *job = KIO::file_copy(subUrl, destUrl, -1, KIO::HideProgressInfo | KIO::Overwrite);
    KIO::MetaData metadata = job->metaData();
    metadata.insert(QStringLiteral("ssl_no_client_cert"), QStringLiteral("TRUE"));
    metadata.insert(QStringLiteral("ssl_no_ui"), QStringLiteral("TRUE"));
    metadata.insert(QStringLiteral("UseCache"), QStringLiteral("false"));
    metadata.insert(QStringLiteral("cookies"), QStringLiteral("none"));
    metadata.insert(QStringLiteral("no-auth"), QStringLiteral("true"));
    job->setMetaData(metadata);

    connect(job, &KIO::FileCopyJob::finished, this, &AdBlockShowListDialog::slotFinished);
}

void AdBlockShowListDialog::slotFinished(KJob *job)
{
    mProgress->stop();
    if (job->error()) {
        mTextEdit->editor()->setPlainText(i18n("An error occurs during download list: \"%1\"", job->errorString()));
        mUser1Button->setEnabled(true);
    } else {
        QFile f(mTemporaryFile->fileName());
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            mTextEdit->editor()->setPlainText(QString::fromUtf8(f.readAll()));
        }
    }
    mTemporaryFile->close();
    delete mTemporaryFile;
    mTemporaryFile = 0;
}

void AdBlockShowListDialog::slotDeleteBrokenList()
{
    Q_EMIT deleteList(mListName);
    accept();
}

void AdBlockShowListDialog::setListName(const QString &listName)
{
    mListName = listName;
}
