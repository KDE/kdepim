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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/picture.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurldrag.h>
#include <libkdepim/kpixmapregionselectordialog.h>

#include <qapplication.h>
#include <qdragobject.h>
#include <qeventloop.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpopupmenu.h>

#include <unistd.h>

#include "imagewidget.h"

ImageLoader::ImageLoader()
  : QObject( 0, "ImageLoader" )
{
}

KABC::Picture ImageLoader::loadPicture( const KURL &url, bool *ok )
{
  KABC::Picture picture;
  QString tempFile;

  if ( url.isEmpty() )
    return picture;

  (*ok) = false;

  QImage image;
  if ( url.isLocalFile() ) {
    image.load( url.path() );
    picture.setData( image );
    (*ok) = true;
  } else if ( KIO::NetAccess::download( url, tempFile, 0 ) ) {
    image.load( tempFile );
    picture.setData( image );
    (*ok) = true;
    KIO::NetAccess::removeTempFile( tempFile );
  }

  if ( !(*ok) ) {
    // image does not exist (any more)
    KMessageBox::sorry( 0, i18n( "This contact's image cannot be found." ) );
    return picture;
  }

  QPixmap pixmap = picture.data();

  QPixmap selectedPixmap = KPIM::KPixmapRegionSelectorDialog::getSelectedImage( pixmap, 100, 140, 0 );
  if ( selectedPixmap.isNull() ) {
    (*ok) = false;
    return picture;
  }

  image = selectedPixmap;
  if ( image.height() != 140 || image.width() != 100 ) {
    if ( image.height() > image.width() )
      image = image.scaleHeight( 140 );
    else
      image = image.scaleWidth( 100 );
  }

  picture.setData( image );
  (*ok) = true;

  return picture;
}


ImageButton::ImageButton( const QString &title, QWidget *parent )
  : QPushButton( title, parent ),
    mReadOnly( false ), mImageLoader( 0 )
{
  setAcceptDrops( true );

  connect( this, SIGNAL( clicked() ), SLOT( load() ) );
}

void ImageButton::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void ImageButton::setPicture( const KABC::Picture &picture )
{
  mPicture = picture;
  updateGUI();
}

KABC::Picture ImageButton::picture() const
{
  return mPicture;
}

void ImageButton::setImageLoader( ImageLoader *loader )
{
  mImageLoader = loader;
}

void ImageButton::startDrag()
{
  if ( !mPicture.data().isNull() ) {
    QImageDrag *drag = new QImageDrag( mPicture.data(), this );
    drag->dragCopy();
  }
}

void ImageButton::updateGUI()
{
  if ( mPicture.data().isNull() )
    setPixmap( KGlobal::iconLoader()->iconPath( "personal", KIcon::Desktop ) );
  else
    setPixmap( mPicture.data() );
}

void ImageButton::dragEnterEvent( QDragEnterEvent *event )
{
  bool accepted = false;

  if ( QImageDrag::canDecode( event ) )
    accepted = true;

  if ( QUriDrag::canDecode( event ) )
    accepted = true;

  event->accept( accepted );
}

void ImageButton::dropEvent( QDropEvent *event )
{
  if ( mReadOnly )
    return;

  if ( QImageDrag::canDecode( event ) ) {
    QPixmap pm;

    if ( QImageDrag::decode( event, pm ) ) {
      mPicture.setData( pm.convertToImage() );
      updateGUI();
      emit changed();
    }
  }

  if ( QUriDrag::canDecode( event ) ) {
    KURL::List urls;
    if ( KURLDrag::decode( event, urls ) ) {
      if ( urls.isEmpty() ) { // oops, no data
        event->accept( false );
        return;
      }
    }

    if ( mImageLoader ) {
      bool ok = false;
      KABC::Picture pic = mImageLoader->loadPicture( urls[ 0 ], &ok );
      if ( ok ) {
        mPicture = pic;
        updateGUI();
        emit changed();
      }
    }
  }
}

void ImageButton::mousePressEvent( QMouseEvent *event )
{
  mDragStartPos = event->pos();
  QPushButton::mousePressEvent( event );
}

void ImageButton::mouseMoveEvent( QMouseEvent *event )
{
  if ( (event->state() & LeftButton) &&
       (event->pos() - mDragStartPos).manhattanLength() >
       KGlobalSettings::dndEventDelay() ) {
    startDrag();
  }
}

void ImageButton::contextMenuEvent( QContextMenuEvent *event )
{
  QPopupMenu menu( this );
  menu.insertItem( i18n( "Reset" ), this, SLOT( clear() ) );
  menu.exec( event->globalPos() );
}

void ImageButton::load()
{
  KURL url = KFileDialog::getOpenURL( QString(), KImageIO::pattern(), this );
  if ( url.isValid() ) {
    if ( mImageLoader ) {
      bool ok = false;
      KABC::Picture pic = mImageLoader->loadPicture( url, &ok );
      if ( ok ) {
        mPicture = pic;
        updateGUI();
        emit changed();
      }
    }
  }
}

void ImageButton::clear()
{
  mPicture = KABC::Picture();
  updateGUI();

  emit changed();
}

ImageBaseWidget::ImageBaseWidget( const QString &title,
                                  QWidget *parent, const char *name )
  : QWidget( parent, name ), mReadOnly( false )
{
  mImageLoader = new ImageLoader();

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );
  QGroupBox *box = new QGroupBox( 0, Qt::Vertical, title, this );
  QVBoxLayout *layout = new QVBoxLayout( box->layout(), KDialog::spacingHint() );

  mImageButton = new ImageButton( i18n( "Picture" ), box );
  mImageButton->setFixedSize( 100, 140 );
  mImageButton->setImageLoader( mImageLoader );
  layout->addWidget( mImageButton );

  topLayout->addWidget( box );

  connect( mImageButton, SIGNAL( changed() ), SIGNAL( changed() ) );
}

ImageBaseWidget::~ImageBaseWidget()
{
  delete mImageLoader;
  mImageLoader = 0;
}

void ImageBaseWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
  mImageButton->setReadOnly( mReadOnly );
}

void ImageBaseWidget::setImage( const KABC::Picture &photo )
{
  mImageButton->setPicture( photo );
}

KABC::Picture ImageBaseWidget::image() const
{
  return mImageButton->picture();
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
