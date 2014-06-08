/*
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "soundtestwidget.h"

#include <KFileDialog>
#include <KIconLoader>
#include <KLocalizedString>
#include <KUrlRequester>
#include <KLineEdit>
#include <KStandardDirs>
#include <QIcon>
#include <KGlobal>
#include <KUrl>

#include <QHBoxLayout>
#include <QPushButton>

#include <phonon/mediaobject.h>

using namespace MailCommon;

SoundTestWidget::SoundTestWidget( QWidget *parent )
    : QWidget( parent )
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    m_playButton = new QPushButton( this );
    m_playButton->setIcon( QIcon::fromTheme( QLatin1String("arrow-right") ) );
    m_playButton->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    m_playButton->setToolTip(i18n("Play"));
    layout->addWidget( m_playButton );

    m_urlRequester = new KUrlRequester( this );
    layout->addWidget( m_urlRequester );

    connect( m_playButton, SIGNAL(clicked()),
             SLOT(playSound()) );
    connect( m_urlRequester, SIGNAL(openFileDialog(KUrlRequester*)),
             SLOT(openSoundDialog(KUrlRequester*)) );
    connect( m_urlRequester->lineEdit(), SIGNAL(textChanged(QString)),
             SLOT(slotUrlChanged(QString)) );

    slotUrlChanged( m_urlRequester->lineEdit()->text() );
}

SoundTestWidget::~SoundTestWidget()
{
}

void SoundTestWidget::slotUrlChanged( const QString &url )
{
    m_playButton->setEnabled( !url.isEmpty() );
    emit textChanged( url );
}

void SoundTestWidget::openSoundDialog( KUrlRequester * )
{
    static bool init = true;
    if ( !init ) {
        return;
    }

    init = false;
    QFileDialog *fileDialog = m_urlRequester->fileDialog();
    fileDialog->setWindowTitle( i18n( "Select Sound File" ) );

    QStringList filters;
    filters << QLatin1String("audio/x-wav")
            << QLatin1String("audio/mpeg")
            << QLatin1String("application/ogg")
            << QLatin1String("audio/x-adpcm");

    fileDialog->setMimeTypeFilters( filters );

    const QStringList soundDirs = KGlobal::dirs()->resourceDirs( "sound" );

    if ( !soundDirs.isEmpty() ) {
        KUrl soundURL;
        QDir dir;
        dir.setFilter( QDir::Files | QDir::Readable );

        foreach ( const QString &soundDir, soundDirs ) {
            dir = soundDir;
            if ( dir.isReadable() && dir.count() > 2 ) {
                soundURL.setPath( soundDir );
                fileDialog->setDirectoryUrl( soundURL );
                break;
            }
        }
    }
}

void SoundTestWidget::playSound()
{
    const QString parameter = m_urlRequester->lineEdit()->text();
    if ( parameter.isEmpty() ) {
        return ;
    }

    const QString file = QLatin1String( "file:" );
    const QString play = ( parameter.startsWith( file ) ?
                               parameter.mid( file.length() ) :
                               parameter );
    Phonon::MediaObject *player = Phonon::createPlayer( Phonon::NotificationCategory, QUrl::fromLocalFile(play) );
    player->play();
    connect( player, SIGNAL(finished()), player, SLOT(deleteLater()) );
}

QString SoundTestWidget::url() const
{
    return m_urlRequester->lineEdit()->text();
}

void SoundTestWidget::setUrl( const QString &url )
{
    m_urlRequester->lineEdit()->setText(url);
}

void SoundTestWidget::clear()
{
    m_urlRequester->lineEdit()->clear();
}

