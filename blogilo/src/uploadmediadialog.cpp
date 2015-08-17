/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "uploadmediadialog.h"

#include "settings.h"
#include "bilboblog.h"
#include "bilbomedia.h"
#include "backend.h"

#include <KApplication>
#include "blogilo_debug.h"
#include <KMessageBox>
#include <QFileDialog>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <KLocalizedString>
#include <QIcon>

#include <QUrl>
#include <QClipboard>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMimeDatabase>

UploadMediaDialog::UploadMediaDialog(QWidget *parent)
    : QDialog(parent), mCurrentBlog(Q_NULLPTR)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UploadMediaDialog::slotOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);
    setAttribute(Qt::WA_DeleteOnClose);

    okButton->setText(i18n("Upload"));
    setWindowTitle(i18n("Upload Media..."));
    ui.kcfg_FtpPath->setText(Settings::ftpServerPath());
    ui.kcfg_httpUrl->setText(Settings::httpUrl());
    setWindowModality(Qt::ApplicationModal);
    ui.kcfg_urlBrowser->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(ui.kcfg_urlBrowser, &QPushButton::clicked, this, &UploadMediaDialog::selectNewFile);
    connect(ui.kcfg_uploadType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &UploadMediaDialog::slotUploadTypeChanged);
    connect(ui.kcfg_urlLineEdit, &QLineEdit::textChanged, this, &UploadMediaDialog::currentMediaChanged);
}

UploadMediaDialog::~UploadMediaDialog()
{
    Settings::setFtpServerPath(ui.kcfg_FtpPath->text());
    Settings::setHttpUrl(ui.kcfg_httpUrl->text());
    Settings::self()->save();
}

void UploadMediaDialog::init(const BilboBlog *currentBlog)
{
    if (!selectNewFile()) {
        deleteLater();
        return;
    }
    if (currentBlog) {
        mCurrentBlog = currentBlog;
        if (mCurrentBlog->supportUploadMedia()) {
            ui.kcfg_uploadType->addItem(i18n("Blog API"), BlogAPI);
        }
    }
    ui.kcfg_uploadType->addItem(i18n("FTP"), FTP);
    slotUploadTypeChanged(ui.kcfg_uploadType->currentIndex());
    this->show();
}

void UploadMediaDialog::currentMediaChanged(const QString &newPath)
{
    ui.kcfg_previewer->showPreview(QUrl::fromLocalFile(newPath));
}

bool UploadMediaDialog::selectNewFile()
{
    const QString mediaPath = QFileDialog::getOpenFileName(this, i18n("Select Media to Upload"));
    if (mediaPath.isEmpty()) {
        return false;
    }

    QUrl mediaUrl(mediaPath);
    ui.kcfg_urlLineEdit->setText(mediaPath);
    ui.kcfg_Name->setText(mediaUrl.fileName());
    ui.kcfg_previewer->showPreview(mediaUrl);
    return true;
}

void UploadMediaDialog::slotUploadTypeChanged(int index)
{
    UploadType type = static_cast<UploadType>(ui.kcfg_uploadType->itemData(index).toInt());
    if (type == FTP) {
        ui.kcfg_ftpBox->setEnabled(true);
    } else {
        ui.kcfg_ftpBox->setEnabled(false);
    }
}

void UploadMediaDialog::slotOkClicked()
{
    UploadType type = static_cast<UploadType>(ui.kcfg_uploadType->itemData(ui.kcfg_uploadType->currentIndex()).toInt());
    if (type == BlogAPI) {  ///Using API!
        BilboMedia *media = new BilboMedia(this);
        QUrl mediaUrl(ui.kcfg_urlLineEdit->text());
        media->setLocalUrl(mediaUrl);
        media->setName(ui.kcfg_Name->text().isEmpty() ? mediaUrl.fileName() : ui.kcfg_Name->text());
        media->setBlogId(mCurrentBlog->id());
        QMimeDatabase mimeDb;
        media->setMimeType(mimeDb.mimeTypeForUrl(mediaUrl).name());
        Backend *b = new Backend(mCurrentBlog->id(), this);
        connect(b, SIGNAL(sigMediaUploaded(BilboMedia*)),
                this, SLOT(slotMediaObjectUploaded(BilboMedia*)));
        connect(b, &Backend::sigError, this, &UploadMediaDialog::slotError);
        connect(b, &Backend::sigMediaError, this, &UploadMediaDialog::slotError);
        b->uploadMedia(media);
        this->hide();
        Q_EMIT sigBusy(true);
    } else if (type == FTP) {  ///Upload via FTP
        if (ui.kcfg_FtpPath->text().isEmpty()) {
            KMessageBox::sorry(this, i18n("Please insert FTP URL."));
            return;
        }
        QUrl dest;
        dest.setUrl(ui.kcfg_FtpPath->text() , QUrl::TolerantMode);
        if (dest.isValid()) {
            if (dest.scheme() == QLatin1String("ftp") || dest.scheme() == QLatin1String("sftp")) {
                QUrl src(ui.kcfg_urlLineEdit->text());
                dest.setPath(dest.path() + QStringLiteral("/") + (ui.kcfg_Name->text().isEmpty() ? src.fileName() : ui.kcfg_Name->text()));
                KIO::FileCopyJob *job = KIO::file_copy(src, dest);
                connect(job, SIGNAL(result(KJob*)), this, SLOT(slotMediaObjectUploaded(KJob*)));
                job->start();
                this->hide();
                return;
            }
        }
        KMessageBox::error(this, i18n("Inserted FTP URL is not a valid URL.\n"
                                      "Note: The URL must start with \"ftp\" or \"sftp\", "
                                      "and end with a \"/\" that indicates the directory to which the file should be uploaded."));
        // > what is meant here?
        // edited coles 2009 - I think it makes sense now.
    }
}

void UploadMediaDialog::slotMediaObjectUploaded(KJob *job)
{
    Q_EMIT sigBusy(false);
    if (job->error()) {
        qCDebug(BLOGILO_LOG) << "Job error: " << job->errorString();
        slotError(job->errorString());
    } else {
        KIO::FileCopyJob *fcj = qobject_cast<KIO::FileCopyJob *>(job);
        QUrl tmpUrl(ui.kcfg_httpUrl->text());
        QString destUrl;
        if (tmpUrl.isValid()) {
            tmpUrl.setPath(tmpUrl.path() + QStringLiteral("/") + ui.kcfg_Name->text());
            destUrl = tmpUrl.toDisplayString(QUrl::FullyDecoded);
        } else {
            destUrl = fcj->destUrl().toDisplayString();
        }
        QString msg;
        if (Settings::copyMediaUrl()) {
            QApplication::clipboard()->setText(destUrl);
            msg = i18n("Media uploaded, and URL copied to clipboard.\nYou can find it here:\n%1",
                       destUrl);
        } else {
            msg = i18n("Media uploaded.\nYou can find it here:\n%1",
                       destUrl);
        }
        KMessageBox::information(this, msg, i18n("Successfully uploaded"), QString(), KMessageBox::AllowLink);
        accept();
    }
}

void UploadMediaDialog::slotMediaObjectUploaded(BilboMedia *media)
{
    QString msg;
    Q_EMIT sigBusy(false);
    if (Settings::copyMediaUrl()) {
        QApplication::clipboard()->setText(media->remoteUrl().toDisplayString());
        msg = i18n("Media uploaded, and URL copied to clipboard.\nYou can find it here:\n%1",
                   media->remoteUrl().toDisplayString());
    } else {
        msg = i18n("Media uploaded.\nYou can find it here:\n%1",
                   media->remoteUrl().toDisplayString());
    }
    KMessageBox::information(this, msg, i18n("Successfully uploaded"), QString(), KMessageBox::AllowLink);
    accept();
}

void UploadMediaDialog::slotError(const QString &msg)
{
    Q_EMIT sigBusy(false);
    if (KMessageBox::questionYesNo(this, i18n("Media uploading failed with this result:\n%1\nTry again?", msg))
            == KMessageBox::Yes) {
        show();
    } else {
        reject();
    }
}

