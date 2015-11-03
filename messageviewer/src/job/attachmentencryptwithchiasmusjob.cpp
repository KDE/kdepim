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

#include "attachmentencryptwithchiasmusjob.h"
#include "viewer/chiasmuskeyselector.h"
#include "messageviewer/messageviewerutil.h"
#include "settings/messageviewersettings.h"

#include <gpgme++/error.h>

#include <kio/jobuidelegate.h>
#include <KIO/Job>
#include <KMessageBox>
#include <KLocalizedString>
#include <QFileDialog>
#include <KJobWidgets>

#include <Libkleo/CryptoBackendFactory>
#include <Libkleo/CryptoBackend>
#include <Libkleo/SpecialJob>

using namespace MessageViewer;

static const QString chomp(const QString &base, const QString &suffix, bool cs)
{
    return base.endsWith(suffix, cs ? (Qt::CaseSensitive) : (Qt::CaseInsensitive)) ? base.left(base.length() - suffix.length()) : base;
}

AttachmentEncryptWithChiasmusJob::AttachmentEncryptWithChiasmusJob(QObject *parent)
    : QObject(parent),
      mJob(0),
      mMainWindow(0),
      mContent(0)
{

}

AttachmentEncryptWithChiasmusJob::~AttachmentEncryptWithChiasmusJob()
{

}

void AttachmentEncryptWithChiasmusJob::setContent(KMime::Content *content)
{
    mContent = content;
}

void AttachmentEncryptWithChiasmusJob::setCurrentFileName(const QString &currentFileName)
{
    mCurrentFileName = currentFileName;
}

void AttachmentEncryptWithChiasmusJob::setMainWindow(QWidget *mainWindow)
{
    mMainWindow = mainWindow;
}

void AttachmentEncryptWithChiasmusJob::start()
{
    Q_UNUSED(mContent);

    // FIXME: better detection of mimetype??
    if (!mCurrentFileName.endsWith(QStringLiteral(".xia"), Qt::CaseInsensitive)) {
        deleteLater();
        return;
    }

    const Kleo::CryptoBackend::Protocol *chiasmus =
        Kleo::CryptoBackendFactory::instance()->protocol("Chiasmus");
    Q_ASSERT(chiasmus);
    if (!chiasmus) {
        deleteLater();
        return;
    }

    const std::unique_ptr<Kleo::SpecialJob> listjob(chiasmus->specialJob("x-obtain-keys", QMap<QString, QVariant>()));
    if (!listjob.get()) {
        const QString msg = i18n("Chiasmus backend does not offer the "
                                 "\"x-obtain-keys\" function. Please report this bug.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    if (listjob->exec()) {
        listjob->showErrorDialog(mMainWindow, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    const QVariant result = listjob->property("result");
    if (result.type() != QVariant::StringList) {
        const QString msg = i18n("Unexpected return value from Chiasmus backend: "
                                 "The \"x-obtain-keys\" function did not return a "
                                 "string list. Please report this bug.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    const QStringList keys = result.toStringList();
    if (keys.empty()) {
        const QString msg = i18n("No keys have been found. Please check that a "
                                 "valid key path has been set in the Chiasmus "
                                 "configuration.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }
    QScopedPointer<ChiasmusKeySelector> selectorDlg(new ChiasmusKeySelector(mMainWindow,
            i18n("Chiasmus Decryption Key Selection"),
            keys, MessageViewer::MessageViewerSettings::chiasmusDecryptionKey(),
            MessageViewer::MessageViewerSettings::chiasmusDecryptionOptions()));
    if (selectorDlg->exec() != QDialog::Accepted || !selectorDlg) {
        deleteLater();
        return;
    }

    MessageViewer::MessageViewerSettings::setChiasmusDecryptionOptions(selectorDlg->options());
    MessageViewer::MessageViewerSettings::setChiasmusDecryptionKey(selectorDlg->key());
    assert(!MessageViewer::MessageViewerSettings::chiasmusDecryptionKey().isEmpty());
    Kleo::SpecialJob *job = chiasmus->specialJob("x-decrypt", QMap<QString, QVariant>());
    if (!job) {
        const QString msg = i18n("Chiasmus backend does not offer the "
                                 "\"x-decrypt\" function. Please report this bug.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    //PORT IT
    const QByteArray input;// = node->msgPart().bodyDecodedBinary();

    if (!job->setProperty("key", MessageViewer::MessageViewerSettings::chiasmusDecryptionKey()) ||
            !job->setProperty("options", MessageViewer::MessageViewerSettings::chiasmusDecryptionOptions()) ||
            !job->setProperty("input", input)) {
        const QString msg = i18n("The \"x-decrypt\" function does not accept "
                                 "the expected parameters. Please report this bug.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    if (job->start()) {
        job->showErrorDialog(mMainWindow, i18n("Chiasmus Decryption Error"));
        deleteLater();
        return;
    }

    mJob = job;
    connect(job, SIGNAL(result(GpgME::Error,QVariant)),
            this, SLOT(slotAtmDecryptWithChiasmusResult(GpgME::Error,QVariant)));
}

void AttachmentEncryptWithChiasmusJob::slotAtmDecryptWithChiasmusResult(const GpgME::Error &err, const QVariant &result)
{
    if (!mJob) {
        deleteLater();
        return;
    }
    Q_ASSERT(mJob == sender());
    if (mJob != sender()) {
        deleteLater();
        return;
    }
    Kleo::Job *job = mJob;
    mJob = 0;
    if (err.isCanceled()) {
        deleteLater();
        return;
    }
    if (err) {
        job->showErrorDialog(mMainWindow, i18n("Chiasmus Decryption Error"));
        deleteLater();
        return;
    }

    if (result.type() != QVariant::ByteArray) {
        const QString msg = i18n("Unexpected return value from Chiasmus backend: "
                                 "The \"x-decrypt\" function did not return a "
                                 "byte array. Please report this bug.");
        KMessageBox::error(mMainWindow, msg, i18n("Chiasmus Backend Error"));
        deleteLater();
        return;
    }

    const QUrl url = QFileDialog::getSaveFileUrl(mMainWindow, QString(), QUrl::fromLocalFile(chomp(mCurrentFileName, QStringLiteral(".xia"), false)));
    if (url.isEmpty()) {
        deleteLater();
        return;
    }

    bool overwrite = MessageViewer::Util::checkOverwrite(url, mMainWindow);
    if (!overwrite)  {
        deleteLater();
        return;
    }

    KIO::Job *uploadJob = KIO::storedPut(result.toByteArray(), url, -1, KIO::Overwrite);
    KJobWidgets::setWindow(uploadJob, mMainWindow);
    connect(uploadJob, &KJob::result,
            this, &AttachmentEncryptWithChiasmusJob::slotAtmDecryptWithChiasmusUploadResult);
}

void AttachmentEncryptWithChiasmusJob::slotAtmDecryptWithChiasmusUploadResult(KJob *job)
{
    if (job->error()) {
        static_cast<KIO::Job *>(job)->ui()->showErrorMessage();
    }
    deleteLater();
}
