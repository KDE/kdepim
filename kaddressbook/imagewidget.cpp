/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kabc/picture.h>
#include <kaccelmanager.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>

#include "imagewidget.h"

ImageWidget::ImageWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 1, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QGroupBox *photoBox = new QGroupBox( 0, Qt::Vertical, i18n( "Photo" ), this );
  QGridLayout *boxLayout = new QGridLayout( photoBox->layout(), 3, 2,
                                            KDialog::spacingHint() );
  boxLayout->setRowStretch( 2, 1 );

  mPhotoLabel = new QLabel( photoBox );
  mPhotoLabel->setFixedSize( 50, 70 );
  mPhotoLabel->setScaledContents( true );
  mPhotoLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  boxLayout->addMultiCellWidget( mPhotoLabel, 0, 2, 0, 0 );

  mPhotoUrl = new KURLRequester( photoBox );
  boxLayout->addWidget( mPhotoUrl, 0, 1 );
  
  mUsePhotoUrl = new QCheckBox( i18n( "Store as URL" ), photoBox );
  boxLayout->addWidget( mUsePhotoUrl, 1, 1 );

  topLayout->addWidget( photoBox, 0, 0 );

  QGroupBox *logoBox = new QGroupBox( 0, Qt::Vertical, i18n( "Logo" ), this );
  boxLayout = new QGridLayout( logoBox->layout(), 3, 2, KDialog::spacingHint() );
  boxLayout->setRowStretch( 2, 1 );

  mLogoLabel = new QLabel( logoBox );
  mLogoLabel->setFixedSize( 50, 70 );
  mLogoLabel->setScaledContents( true );
  mLogoLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  boxLayout->addMultiCellWidget( mLogoLabel, 0, 2, 0, 0 );

  mLogoUrl = new KURLRequester( logoBox );
  boxLayout->addWidget( mLogoUrl, 0, 1 );
  
  mUseLogoUrl = new QCheckBox( i18n( "Store as URL" ), logoBox );
  boxLayout->addWidget( mUseLogoUrl, 1, 1 );

  topLayout->addWidget( logoBox, 1, 0 );

  connect( mPhotoUrl, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mPhotoUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( loadPhoto() ) );
  connect( mPhotoUrl, SIGNAL( urlSelected( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mUsePhotoUrl, SIGNAL( toggled( bool ) ),
           SIGNAL( changed() ) );

  connect( mLogoUrl, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mLogoUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( loadLogo() ) );
  connect( mLogoUrl, SIGNAL( urlSelected( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mUseLogoUrl, SIGNAL( toggled( bool ) ),
           SIGNAL( changed() ) );

  KAcceleratorManager::manage( this );
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::setPhoto( const KABC::Picture &photo )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  if ( photo.isIntern() ) {
    mPhotoLabel->setPixmap( photo.data() );
    mUsePhotoUrl->setChecked( false );
  } else {
    mPhotoUrl->setURL( photo.url() );
    mUsePhotoUrl->setChecked( true );
    loadPhoto();
  }

  blockSignals( blocked );
}

KABC::Picture ImageWidget::photo() const
{
  KABC::Picture photo;

  if ( mUsePhotoUrl->isChecked() )
    photo.setUrl( mPhotoUrl->url() );
  else {
    QPixmap *px = mPhotoLabel->pixmap();
    if ( px )
      photo.setData( px->convertToImage() );
  }

  return photo;
}

void ImageWidget::setLogo( const KABC::Picture &logo )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  if ( logo.isIntern() ) {
    mLogoLabel->setPixmap( logo.data() );
    mUseLogoUrl->setChecked( false );
  } else {
    mLogoUrl->setURL( logo.url() );
    mUseLogoUrl->setChecked( true );
    loadLogo();
  }

  blockSignals( blocked );
}

KABC::Picture ImageWidget::logo() const
{
  KABC::Picture logo;

  if ( mUseLogoUrl->isChecked() )
    logo.setUrl( mLogoUrl->url() );
  else {
    QPixmap *px = mLogoLabel->pixmap();
    if ( px )
      logo.setData( px->convertToImage() );
  }

  return logo;
}

void ImageWidget::loadPhoto()
{
  mPhotoLabel->setPixmap( loadPixmap( mPhotoUrl->url() ) );
}

void ImageWidget::loadLogo()
{
  mLogoLabel->setPixmap( loadPixmap( mLogoUrl->url() ) );
}

QPixmap ImageWidget::loadPixmap( const KURL &url )
{
  QString tempFile;
  QPixmap pixmap;

  if ( url.isEmpty() )
    return pixmap;

  if ( url.isLocalFile() )
    pixmap = QPixmap( url.path() );
  else if ( KIO::NetAccess::download( url, tempFile ) ) {
    pixmap = QPixmap( tempFile );
    KIO::NetAccess::removeTempFile( tempFile );
  }

  return pixmap;
}

#include "imagewidget.moc"
