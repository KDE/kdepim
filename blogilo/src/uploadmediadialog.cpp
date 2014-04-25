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
#include <KDebug>
#include <KMessageBox>
#include <KFileDialog>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <KLocalizedString>
#include <KMimeType>
#include <KIcon>

#include <QClipboard>

UploadMediaDialog::UploadMediaDialog( QWidget *parent )
    : KDialog(parent), mCurrentBlog(0)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    setMainWidget(widget);
    setAttribute(Qt::WA_DeleteOnClose);

    setButtonText(KDialog::Ok, i18n("Upload") );
    setWindowTitle( i18n( "Upload Media..." ) );
    ui.kcfg_FtpPath->setText(Settings::ftpServerPath());
    ui.kcfg_httpUrl->setText(Settings::httpUrl());
    setWindowModality(Qt::ApplicationModal);
    ui.kcfg_urlBrowser->setIcon(KIcon(QLatin1String("document-open")));
    connect( ui.kcfg_urlBrowser, SIGNAL(clicked(bool)), this, SLOT(selectNewFile()) );
    connect(ui.kcfg_uploadType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUploadTypeChanged(int)));
    connect( ui.kcfg_urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(currentMediaChanged(QString)) );
}

UploadMediaDialog::~UploadMediaDialog()
{
    Settings::setFtpServerPath(ui.kcfg_FtpPath->text());
    Settings::setHttpUrl(ui.kcfg_httpUrl->text());
    Settings::self()->writeConfig();
}

void UploadMediaDialog::init( const BilboBlog *currentBlog )
{
    if ( !selectNewFile() ) {
        deleteLater();
        return;
    }
    if (currentBlog) {
        mCurrentBlog = currentBlog;
        if (mCurrentBlog->supportUploadMedia()){
            ui.kcfg_uploadType->addItem( i18n("Blog API"), BlogAPI );
        }
    }
    ui.kcfg_uploadType->addItem( i18n("FTP"), FTP);
    slotUploadTypeChanged(ui.kcfg_uploadType->currentIndex());
    this->show();
}

void UploadMediaDialog::currentMediaChanged(const QString& newPath)
{
    ui.kcfg_previewer->showPreview(KUrl(newPath));
}

bool UploadMediaDialog::selectNewFile()
{
    const QString mediaPath = KFileDialog::getOpenFileName( KUrl("kfiledialog:///image?global"),
                                                            QString(), this,
                                                            i18n("Select Media to Upload"));
    if ( mediaPath.isEmpty() )
        return false;

    KUrl mediaUrl(mediaPath);
    ui.kcfg_urlLineEdit->setText(mediaPath);
    ui.kcfg_Name->setText(mediaUrl.fileName());
    ui.kcfg_previewer->showPreview( mediaUrl );
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

void UploadMediaDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok) {
        UploadType type = static_cast<UploadType>(ui.kcfg_uploadType->itemData(ui.kcfg_uploadType->currentIndex()).toInt());
        if ( type == BlogAPI ) {///Using API!
            BilboMedia *media = new BilboMedia(this);
            KUrl mediaUrl( ui.kcfg_urlLineEdit->text() );
            media->setLocalUrl(mediaUrl);
            media->setName( ui.kcfg_Name->text().isEmpty() ? mediaUrl.fileName() : ui.kcfg_Name->text() );
            media->setBlogId( mCurrentBlog->id() );
            media->setMimeType( KMimeType::findByUrl( mediaUrl, 0, true )->name() );
            Backend *b = new Backend( mCurrentBlog->id(), this);
            connect( b, SIGNAL(sigMediaUploaded(BilboMedia*)),
                     this, SLOT(slotMediaObjectUploaded(BilboMedia*)) );
            connect( b, SIGNAL(sigError(QString)), this, SLOT(slotError(QString)));
            connect( b, SIGNAL(sigMediaError(QString,BilboMedia*)), this, SLOT(slotError(QString)) );
            b->uploadMedia( media );
            this->hide();
            emit sigBusy(true);
        } else if ( type == FTP ) {///Upload via FTP
            if ( ui.kcfg_FtpPath->text().isEmpty() ) {
                KMessageBox::sorry(this, i18n("Please insert FTP URL."));
                return;
            }
            KUrl dest;
            dest.setUrl(ui.kcfg_FtpPath->text() , QUrl::TolerantMode);
            if ( dest.isValid() ) {
                if ( dest.scheme() == QLatin1String("ftp") || dest.scheme() == QLatin1String("sftp") ) {
                    KUrl src(ui.kcfg_urlLineEdit->text());
                    dest.addPath( ui.kcfg_Name->text().isEmpty() ? src.fileName() :
                                                                   ui.kcfg_Name->text() );
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
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void UploadMediaDialog::slotMediaObjectUploaded(KJob *job)
{
    emit sigBusy(false);
    if (job->error()) {
        kDebug()<<"Job error: "<<job->errorString();
        slotError(job->errorString());
    } else {
        KIO::FileCopyJob *fcj = qobject_cast<KIO::FileCopyJob*>(job);
        KUrl tmpUrl(ui.kcfg_httpUrl->text());
        QString destUrl;
        if (tmpUrl.isValid()){
            tmpUrl.adjustPath(KUrl::AddTrailingSlash);
            tmpUrl.setFileName(ui.kcfg_Name->text());
            destUrl = tmpUrl.prettyUrl();
        } else {
            destUrl = fcj->destUrl().toDisplayString();
        }
        QString msg;
        if ( Settings::copyMediaUrl() ) {
            KApplication::clipboard()->setText( destUrl );
            msg = i18n( "Media uploaded, and URL copied to clipboard.\nYou can find it here:\n%1",
                        destUrl );
        } else {
            msg = i18n( "Media uploaded.\nYou can find it here:\n%1",
                        destUrl );
        }
        KMessageBox::information(this, msg, i18n( "Successfully uploaded" ), QString(), KMessageBox::AllowLink);
        accept();
    }
}

void UploadMediaDialog::slotMediaObjectUploaded(BilboMedia *media)
{
    QString msg;
    emit sigBusy(false);
    if ( Settings::copyMediaUrl() ) {
        KApplication::clipboard()->setText( media->remoteUrl().prettyUrl() );
        msg = i18n( "Media uploaded, and URL copied to clipboard.\nYou can find it here:\n%1",
                    media->remoteUrl().prettyUrl() );
    } else {
        msg = i18n( "Media uploaded.\nYou can find it here:\n%1",
                    media->remoteUrl().prettyUrl() );
    }
    KMessageBox::information(this, msg, i18n( "Successfully uploaded" ), QString(), KMessageBox::AllowLink);
    accept();
}

void UploadMediaDialog::slotError( const QString &msg )
{
    emit sigBusy(false);
    if ( KMessageBox::questionYesNo( this, i18n( "Media uploading failed with this result:\n%1\nTry again?", msg) )
         == KMessageBox::Yes ) {
        show();
    } else {
        reject();
    }
}

