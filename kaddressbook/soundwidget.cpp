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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/sound.h>
#include <kaudioplayer.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include "soundwidget.h"

SoundWidget::SoundWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name ), mReadOnly( false )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 3, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "multimedia",
                    KIcon::Desktop, KIcon::SizeMedium ) );
  label->setAlignment( Qt::AlignTop );
  topLayout->addMultiCellWidget( label, 0, 1, 0, 0 );

  mPlayButton = new QPushButton( i18n( "Play" ), this );
  mPlayButton->setEnabled( false );
  topLayout->addWidget( mPlayButton, 0, 1 );

  mSoundUrl = new KURLRequester( this );
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
  connect( mSoundUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( loadSound() ) );
  connect( mSoundUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( updateGUI() ) );
  connect( mPlayButton, SIGNAL( clicked() ),
           SLOT( playSound() ) );

  QWhatsThis::add( this, i18n( "This field stores a sound file which contains the name of the contact to clarify the pronunciation." ) );
  QWhatsThis::add( mUseSoundUrl, i18n( "Save only the URL to the sound file, not the whole object." ) );
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
    mSoundUrl->setURL( sound.url() );
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
    sound.setUrl( mSoundUrl->url() );
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
  KTempFile tmp;

  tmp.file()->writeBlock( mSound.data() );
  tmp.close();

  KAudioPlayer::play( tmp.name() );

  // we can't remove the sound file from within the program, because
  // KAudioPlay uses a async dcop call... :(
}

void SoundWidget::loadSound()
{
  QString fileName;

  KURL url( mSoundUrl->url() );

  if ( url.isEmpty() )
    return;

  if ( url.isLocalFile() )
    fileName = url.path();
  else if ( !KIO::NetAccess::download( url, fileName, this ) )
    return;

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) )
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
