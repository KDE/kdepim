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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kabc/picture.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <libkdepim/kpixmapregionselectordialog.h>

#include <qcheckbox.h>
#include <qdragobject.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtooltip.h>

#include "imagewidget.h"

ImageLabel::ImageLabel( const QString &title, QWidget *parent )
  : QLabel( title, parent ), mReadOnly( false )
{
  setAcceptDrops( true );
}

void ImageLabel::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void ImageLabel::startDrag()
{
  if ( pixmap() && !pixmap()->isNull() ) {
    QImageDrag *drag = new QImageDrag( pixmap()->convertToImage(), this );
    drag->dragCopy();
  }
}

void ImageLabel::dragEnterEvent( QDragEnterEvent *event )
{
  event->accept( QImageDrag::canDecode( event ) );
}

void ImageLabel::dropEvent( QDropEvent *event )
{
  QPixmap pm;
  if ( QImageDrag::decode( event, pm ) && !mReadOnly ) {
    setPixmap( pm );
    emit changed();
  }
}

void ImageLabel::mousePressEvent( QMouseEvent *event )
{
  mDragStartPos = event->pos();
  QLabel::mousePressEvent( event );
}

void ImageLabel::mouseMoveEvent( QMouseEvent *event )
{
  if ( (event->state() & LeftButton) &&
       (event->pos() - mDragStartPos).manhattanLength() >
       KGlobalSettings::dndEventDelay() ) {
    startDrag();
  }
}


ImageBaseWidget::ImageBaseWidget( const QString &title, QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly( false )
{
  QHBoxLayout *topLayout = new QHBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  QGroupBox *box = new QGroupBox( 0, Qt::Vertical, title, this );
  QGridLayout *boxLayout = new QGridLayout( box->layout(), 3, 3,
                                            KDialog::spacingHint() );
  boxLayout->setRowStretch( 2, 1 );

  mImageLabel = new ImageLabel( i18n( "Picture" ), box );
  mImageLabel->setFixedSize( 100, 140 );
  mImageLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  boxLayout->addMultiCellWidget( mImageLabel, 0, 2, 0, 0, AlignTop );

  mImageUrl = new KURLRequester( box );
  mImageUrl->setFilter( KImageIO::pattern() );
  mImageUrl->setMode( KFile::File );
  boxLayout->addWidget( mImageUrl, 0, 1 );

  mClearButton = new QPushButton( box );
  mClearButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
  mClearButton->setPixmap( SmallIcon( "clear_left" ) );
  mClearButton->setEnabled( false );
  boxLayout->addWidget( mClearButton, 0, 2 );

  mUseImageUrl = new QCheckBox( i18n( "Store as URL" ), box );
  mUseImageUrl->setEnabled( false );
  boxLayout->addMultiCellWidget( mUseImageUrl, 1, 1, 1, 2 );

  topLayout->addWidget( box );

  QToolTip::add( mClearButton, i18n( "Reset" ) );

  connect( mImageLabel, SIGNAL( changed() ),
           SIGNAL( changed() ) );
  connect( mImageUrl, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( loadImage() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SIGNAL( changed() ) );
  connect( mImageUrl, SIGNAL( urlSelected( const QString& ) ),
           SLOT( updateGUI() ) );
  connect( mUseImageUrl, SIGNAL( toggled( bool ) ),
           SIGNAL( changed() ) );
  connect( mClearButton, SIGNAL( clicked() ),
           SLOT( clear() ) );
}

ImageBaseWidget::~ImageBaseWidget()
{
}

void ImageBaseWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  mImageLabel->setReadOnly( mReadOnly );
  mImageUrl->setEnabled( !mReadOnly );
}

void ImageBaseWidget::setImage( const KABC::Picture &photo )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  if ( photo.isIntern() ) {
    QPixmap px = photo.data();

    if ( px.height() != 140 || px.width() != 100 ) {
      if ( px.height() > px.width() )
        px = px.convertToImage().scaleHeight( 140 );
      else
        px = px.convertToImage().scaleWidth( 100 );
    }

    mImageLabel->setPixmap( px );
    mUseImageUrl->setChecked( false );
  } else {
    mImageUrl->setURL( photo.url() );
    if ( !photo.url().isEmpty() )
      mUseImageUrl->setChecked( true );
    loadImage();
  }

  blockSignals( blocked );

  updateGUI();
}

KABC::Picture ImageBaseWidget::image() const
{
  KABC::Picture photo;

  if ( mUseImageUrl->isChecked() )
    photo.setUrl( mImageUrl->url() );
  else {
    if ( mImageLabel->pixmap() )
      photo.setData( mImageLabel->pixmap()->convertToImage() );
  }

  return photo;
}

void ImageBaseWidget::loadImage()
{
  mImageLabel->setPixmap( loadPixmap( KURL( mImageUrl->url() ) ) );
}

void ImageBaseWidget::updateGUI()
{
  if ( !mReadOnly ) {
    mUseImageUrl->setEnabled( !mImageUrl->url().isEmpty() );
    mClearButton->setEnabled( !mImageUrl->url().isEmpty() || ( mImageLabel->pixmap() && !mImageLabel->pixmap()->isNull() ) );
  }
}

void ImageBaseWidget::clear()
{
  mImageLabel->clear();
  mImageUrl->clear();
  mUseImageUrl->setChecked( false );

  updateGUI();

  emit changed();
}

void ImageBaseWidget::imageChanged()
{
  updateGUI();
  
  emit changed();
}

QPixmap ImageBaseWidget::loadPixmap( const KURL &url )
{
  QString tempFile;
  QPixmap pixmap;

  if ( url.isEmpty() )
    return pixmap;

  if ( url.isLocalFile() )
    pixmap = QPixmap( url.path() );
  else if ( KIO::NetAccess::download( url, tempFile, this ) ) {
    pixmap = QPixmap( tempFile );
    KIO::NetAccess::removeTempFile( tempFile );
  }
  if(pixmap.isNull()) {
    //Image does not exist (any more)
    KMessageBox::sorry( this, i18n( "This contact's image cannot be found."));
    return pixmap;
  }
  QPixmap pixmap2 = KPIM::KPixmapRegionSelectorDialog::getSelectedImage( pixmap, 100, 140, this );
  if (!pixmap2.isNull()) 
  {
     pixmap=pixmap2;
     mImageUrl->clear();
  }

  if ( pixmap.height() != 140 || pixmap.width() != 100 ) {
    if ( pixmap.height() > pixmap.width() )
      pixmap = pixmap.convertToImage().scaleHeight( 140 );
    else
      pixmap = pixmap.convertToImage().scaleWidth( 100 );
  }

  return pixmap;
}

ImageWidget::ImageWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this, KDialog::marginHint(), 
                                         KDialog::spacingHint() );

  mPhotoWidget = new ImageBaseWidget( KABC::Addressee::photoLabel(), this );
  layout->addWidget( mPhotoWidget );

  mLogoWidget = new ImageBaseWidget( KABC::Addressee::logoLabel(), this );
  layout->addWidget( mLogoWidget );

  connect( mPhotoWidget, SIGNAL( changed() ), SLOT( setModified() ) );
  connect( mLogoWidget, SIGNAL( changed() ), SLOT( setModified() ) );
}

void ImageWidget::loadContact( KABC::Addressee *addr )
{
  mPhotoWidget->setImage( addr->photo() );
  mLogoWidget->setImage( addr->logo() );
}

void ImageWidget::storeContact( KABC::Addressee *addr )
{
  addr->setPhoto( mPhotoWidget->image() );
  addr->setLogo( mLogoWidget->image() );
}

void ImageWidget::setReadOnly( bool readOnly )
{
  mPhotoWidget->setReadOnly( readOnly );
  mLogoWidget->setReadOnly( readOnly );
}

#include "imagewidget.moc"
