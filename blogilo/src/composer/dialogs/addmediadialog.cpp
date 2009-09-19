/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
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
// #include <kio/jobuidelegate.h>

#include "addmediadialog.h"
#include "bilbomedia.h"
#include "settings.h"

AddMediaDialog::AddMediaDialog( QWidget *parent ) : KDialog( parent )
{
    QWidget *dialog = new QWidget( this );
    ui.setupUi( dialog );

    this->setMainWidget( dialog );
    this->resize( dialog->width(), dialog->height() );
    this->setWindowTitle( "Attach media" );

    connect( this, SIGNAL( okClicked() ), this, SLOT( sltOkClicked() ) );
    connect( ui.radiobtnLocalUrl, SIGNAL( toggled( bool ) ), this, 
             SLOT( sltMediaSourceChanged() ) );
    connect( ui.radiobtnRemoteUrl, SIGNAL( toggled( bool ) ), this, 
             SLOT( sltMediaSourceChanged() ) );
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
    path = QFileDialog::getOpenFileName( this, i18n("Choose a file") );
#else
    path = KFileDialog::getOpenFileName( KUrl(),
                                         QString(), this,
                                         i18n("Choose a file") );
#endif
    ui.urlReqLineEdit->setText(path);
}

void AddMediaDialog::sltOkClicked()
{
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
                        this,  SLOT( sltRemoteFileTypeFound( KIO::Job *, const QString & ) ) );

                addOtherMediaAttributes();

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

                addOtherMediaAttributes();
            }
        } else {
            KMessageBox::error( this, i18n( "The selected media address is an invalid url." ) );
        }
    }
}

void AddMediaDialog::sltRemoteFileTypeFound( KIO::Job *job, const QString &type )
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

void AddMediaDialog::addOtherMediaAttributes()
{
    kDebug() << "emmiting add media signal";
    Q_EMIT sigAddMedia( media );
}

void AddMediaDialog::sltMediaSourceChanged()
{
    if ( ui.radiobtnRemoteUrl->isChecked() ) {
        ui.urlReqBrowse->setEnabled( false );
    } else {
        ui.urlReqBrowse->setEnabled( true );
    }
}
#include "composer/dialogs/addmediadialog.moc"
