/*
    This file is part of KAddressBook.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "soundwidget.h"

#include <QtCore/QBuffer>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kabc/sound.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kurlrequester.h>
#include <phonon/mediaobject.h>

SoundWidget::SoundWidget( KABC::AddressBook *ab, QWidget *parent )
  : KAB::ContactEditorWidget( ab, parent ), mReadOnly( false )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( KDialog::marginHint() );

  QLabel *label = new QLabel( this );
  label->setPixmap( KIconLoader::global()->loadIcon( "audio-x-generic",
                    KIconLoader::Desktop, KIconLoader::SizeMedium ) );
  label->setAlignment( Qt::AlignTop );
  topLayout->addWidget( label, 0, 0, 2, 1);

  mPlayButton = new QPushButton( i18n( "Play" ), this );
  mPlayButton->setEnabled( false );
  topLayout->addWidget( mPlayButton, 0, 1 );

  mSoundUrl = new KUrlRequester( this );
  topLayout->addWidget( mSoundUrl, 0, 2 );

  mUseSoundUrl = new QCheckBox( i18n( "Store as URL" ), this );
  mUseSoundUrl->setEnabled( false );
  topLayout->addWidget( mUseSoundUrl, 1, 2 );

  connect( mSoundUrl, SIGNAL( textChanged( const QString& ) ),
           SLOT( setModified() ) );
  connect( mSoundUrl, SIGNAL( textChanged( const QString& ) ),
           SLOT( urlChanged( const QString& ) ) );
  connect( mUseSoundUrl, SIGNAL( toggled( bool ) ),
           SLOT( setModified() ) );
  connect( mUseSoundUrl, SIGNAL( toggled( bool ) ),
           mPlayButton, SLOT( setDisabled( bool ) ) );
  connect( mSoundUrl, SIGNAL( urlSelected( const KUrl& ) ),
           SLOT( loadSound() ) );
  connect( mSoundUrl, SIGNAL( urlSelected( const KUrl& ) ),
           SLOT( updateGUI() ) );
  connect( mPlayButton, SIGNAL( clicked() ),
           SLOT( playSound() ) );

  this->setWhatsThis( i18n( "This field stores a sound file which contains the name of the contact to clarify the pronunciation." ) );
  mUseSoundUrl->setWhatsThis( i18n( "Save only the URL to the sound file, not the whole object." ) );
}

SoundWidget::~SoundWidget()
{
}

void SoundWidget::loadContact( KABC::Addressee *addr )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  KABC::Sound sound = addr->sound();
  if ( sound.isIntern() ) {
    mSound.setData( sound.data() );
    mPlayButton->setEnabled( true );
    mUseSoundUrl->setChecked( false );
  } else {
    mSoundUrl->setUrl( sound.url() );
    mPlayButton->setEnabled( false );
    if ( !sound.url().isEmpty() )
      mUseSoundUrl->setChecked( true );
  }

  blockSignals( blocked );
}

void SoundWidget::storeContact( KABC::Addressee *addr )
{
  KABC::Sound sound;

  if ( mUseSoundUrl->isChecked() )
    sound.setUrl( mSoundUrl->url().url() );
  else
    sound.setData( mSound.data() );

  addr->setSound( sound );
}

void SoundWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  mSoundUrl->setEnabled( !mReadOnly );
}

void SoundWidget::playSound()
{
  Phonon::MediaObject* player = Phonon::createPlayer( Phonon::NotificationCategory );
  QBuffer* soundData = new QBuffer( player );
  soundData->setData( mSound.data() );
  player->setCurrentSource( soundData );
  player->setParent( this );
  connect( player, SIGNAL( finished() ), player, SLOT( deleteLater() ) );
  player->play();
}

void SoundWidget::loadSound()
{
  QString fileName;

  KUrl url( mSoundUrl->url() );

  if ( url.isEmpty() )
    return;

  if ( url.isLocalFile() )
    fileName = url.path();
  else if ( !KIO::NetAccess::download( url, fileName, this ) )
    return;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) )
    return;

  mSound.setData( file.readAll() );

  file.close();

  if ( !url.isLocalFile() )
    KIO::NetAccess::removeTempFile( fileName );
}

void SoundWidget::updateGUI()
{
  mUseSoundUrl->setEnabled( !mReadOnly );
}

void SoundWidget::urlChanged( const QString &url )
{
  if ( !mUseSoundUrl->isChecked() ) {
    bool state = !url.isEmpty();
    mPlayButton->setEnabled( state );
    mUseSoundUrl->setEnabled( state && !mSound.isIntern() );
  }
}

#include "soundwidget.moc"
