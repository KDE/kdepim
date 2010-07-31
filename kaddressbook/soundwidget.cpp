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

#include <kabc/sound.h>
#include <kaudioplayer.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kurlrequester.h>

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include "soundwidget.h"

SoundWidget::SoundWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name ), mReadOnly( false )
{
  TQGridLayout *topLayout = new TQGridLayout( this, 2, 3, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "multimedia",
                    KIcon::Desktop, KIcon::SizeMedium ) );
  label->setAlignment( Qt::AlignTop );
  topLayout->addMultiCellWidget( label, 0, 1, 0, 0 );

  mPlayButton = new TQPushButton( i18n( "Play" ), this );
  mPlayButton->setEnabled( false );
  topLayout->addWidget( mPlayButton, 0, 1 );

  mSoundUrl = new KURLRequester( this );
  topLayout->addWidget( mSoundUrl, 0, 2 );

  mUseSoundUrl = new TQCheckBox( i18n( "Store as URL" ), this );
  mUseSoundUrl->setEnabled( false );
  topLayout->addWidget( mUseSoundUrl, 1, 2 );

  connect( mSoundUrl, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( setModified() ) );
  connect( mSoundUrl, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( urlChanged( const TQString& ) ) );
  connect( mUseSoundUrl, TQT_SIGNAL( toggled( bool ) ),
           TQT_SLOT( setModified() ) );
  connect( mUseSoundUrl, TQT_SIGNAL( toggled( bool ) ),
           mPlayButton, TQT_SLOT( setDisabled( bool ) ) );
  connect( mSoundUrl, TQT_SIGNAL( urlSelected( const TQString& ) ),
           TQT_SLOT( loadSound() ) );
  connect( mSoundUrl, TQT_SIGNAL( urlSelected( const TQString& ) ),
           TQT_SLOT( updateGUI() ) );
  connect( mPlayButton, TQT_SIGNAL( clicked() ),
           TQT_SLOT( playSound() ) );

  TQWhatsThis::add( this, i18n( "This field stores a sound file which contains the name of the contact to clarify the pronunciation." ) );
  TQWhatsThis::add( mUseSoundUrl, i18n( "Save only the URL to the sound file, not the whole object." ) );
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
  TQString fileName;

  KURL url( mSoundUrl->url() );

  if ( url.isEmpty() )
    return;

  if ( url.isLocalFile() )
    fileName = url.path();
  else if ( !KIO::NetAccess::download( url, fileName, this ) )
    return;

  TQFile file( fileName );
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

void SoundWidget::urlChanged( const TQString &url )
{
  if ( !mUseSoundUrl->isChecked() ) {
    bool state = !url.isEmpty();
    mPlayButton->setEnabled( state );
    mUseSoundUrl->setEnabled( state && !mSound.isIntern() );
  }
}

#include "soundwidget.moc"
