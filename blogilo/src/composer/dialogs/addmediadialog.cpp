/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "addmediadialog.h"

#ifdef WIN32
#include <QFileDialog>
#else
#include <KFileDialog>
#endif

#include <klineedit.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kio/job.h>

#include "bilbomedia.h"
#include "settings.h"

AddMediaDialog::AddMediaDialog( QWidget *parent ) : KDialog( parent )
{
    QWidget *dialog = new QWidget( this );
    ui.setupUi( dialog );

    this->setMainWidget( dialog );
    this->resize( dialog->width(), dialog->height() );
    this->setWindowTitle( i18n("Attach media") );

    connect( ui.radiobtnLocalUrl, SIGNAL( toggled( bool ) ),
             ui.urlReqBrowse, SLOT( setEnabled(bool)) );
    connect( ui.urlReqBrowse, SIGNAL(clicked(bool)), SLOT(slotSelectLocalFile()) );
    ui.urlReqLineEdit->setFocus();
    ui.urlReqLineEdit->setToolTip( i18n( "Type media path here." ) );
    ui.urlReqBrowse->setToolTip( i18n( "Browse" ) );
    ui.urlReqBrowse->setIcon(KIcon("document-open"));
}

AddMediaDialog::~AddMediaDialog()
{
}

void AddMediaDialog::slotSelectLocalFile()
{
    QString path;
#ifdef WIN32
    path = QFileDialog::getOpenFileName( this, i18n("Choose a file") );//krazy:exclude=qclasses KFileDialog has problem on WIN32 now
#else
    path = KFileDialog::getOpenFileName( KUrl(),
                                         QString(), this,
                                         i18n("Choose a file") );
#endif
    ui.urlReqLineEdit->setText(path);
}

void AddMediaDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok){
        KUrl mediaUrl( ui.urlReqLineEdit->text() );
        kDebug() << "parent ok";
        if ( !mediaUrl.isEmpty() ) {
            if ( mediaUrl.isValid() ) {
                media = new BilboMedia();
                QString name = mediaUrl.fileName();

                media->setName( name );

                if ( !mediaUrl.isLocalFile() ) {
                    media->setRemoteUrl( mediaUrl.url() );
                    media->setUploaded( true );

                    KIO::MimetypeJob* typeJob = KIO::mimetype( mediaUrl, KIO::HideProgressInfo );

                    connect( typeJob, SIGNAL( mimetype( KIO::Job *, const QString & ) ),
                            this,  SLOT( slotRemoteFileTypeFound( KIO::Job *, const QString & ) ) );

//                     addOtherMediaAttributes();

                } else {
                    media->setLocalUrl( mediaUrl.toLocalFile() );
                    media->setRemoteUrl( mediaUrl.url() );
                    media->setUploaded( false );

                    KMimeType::Ptr typePtr;
                    typePtr = KMimeType::findByUrl( mediaUrl, 0, true, false );
                    name = typePtr.data()->name();
                    kDebug() << name ;
                    media->setMimeType( name );
                    Q_EMIT sigMediaTypeFound( media );

//                     addOtherMediaAttributes();
                }
                _selectedMedia["url"] = media->remoteUrl().url();
                accept();
            } else {
                KMessageBox::error( this, i18n( "The selected media address is an invalid URL." ) );
            }
        }
    } else {
        KDialog::slotButtonClicked(button);
    }
}
QVariantMap AddMediaDialog::selectedMediaProperties() const
{
    return _selectedMedia;
}

BilboMedia* AddMediaDialog::selectedMedia() const
{
    return media;
}

void AddMediaDialog::slotRemoteFileTypeFound( KIO::Job *job, const QString &type )
{
    kDebug() << type ;
    Q_UNUSED(job);
    media->setMimeType( type );
//     if ( !Settings::download_remote_media() ) {
//         media->setLocalUrl( media->remoteUrl() );
//         Q_EMIT signalAddMedia( media );
    Q_EMIT sigMediaTypeFound( media );
//         addOtherMediaAttributes();
//     }
}
/*
void AddMediaDialog::addOtherMediaAttributes()
{
    kDebug() << "emmiting add media signal";
    Q_EMIT sigAddMedia( media );
}*/

#include "composer/dialogs/addmediadialog.moc"
