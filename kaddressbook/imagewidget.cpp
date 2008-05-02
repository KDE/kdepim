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
#include <kpixmapregionselectordialog.h>

#include <QtGui/QGroupBox>
#include <QtGui/QImage>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>

#include "imagewidget.h"

ImageLoader::ImageLoader( QWidget *parent )
  : QObject( 0 ), mParent( parent )
{
}

KABC::Picture ImageLoader::loadPicture( const KUrl &url, bool *ok )
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
  } else if ( KIO::NetAccess::download( url, tempFile, mParent ) ) {
    image.load( tempFile );
    picture.setData( image );
    (*ok) = true;
    KIO::NetAccess::removeTempFile( tempFile );
  }

  if ( !(*ok) ) {
    // image does not exist (any more)
    KMessageBox::sorry( mParent, i18n( "This contact's image cannot be found." ) );
    return picture;
  }

  QPixmap pixmap = QPixmap::fromImage( picture.data() );

  image = KPixmapRegionSelectorDialog::getSelectedImage( pixmap, 100, 140, mParent );
  if ( image.isNull() ) {
    (*ok) = false;
    return picture;
  }

  if ( image.height() != 140 || image.width() != 100 ) {
    if ( image.height() > image.width() )
      image = image.scaledToHeight( 140 );
    else
      image = image.scaledToWidth( 100 );
  }

  picture.setData( image );
  (*ok) = true;

  return picture;
}

ImageButton::ImageButton( QWidget *parent )
  : QPushButton( parent ),
    mReadOnly( false ), mImageLoader( 0 )
{
  setAcceptDrops( true );
  setIconSize( QSize( 100, 140 ) );

  connect( this, SIGNAL( clicked() ), SLOT( load() ) );
}

void ImageButton::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void ImageButton::setPicture( const KABC::Picture &picture )
{
  mPicture = picture;
  updateGui();
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
    QDrag *drag = new QDrag( this );
    drag->setMimeData( new QMimeData() );
    drag->mimeData()->setImageData( mPicture.data() );
    drag->start();
  }
}

void ImageButton::updateGui()
{
  if ( mPicture.data().isNull() ) {
    setIcon( KIcon( "personal" ) );
  } else {
    setIcon( QPixmap::fromImage( mPicture.data() ) );
  }
}

void ImageButton::dragEnterEvent( QDragEnterEvent *event )
{
  const QMimeData *md = event->mimeData();
  event->setAccepted( md->hasImage() || md->hasUrls());
}

void ImageButton::dropEvent( QDropEvent *event )
{
  if ( mReadOnly )
    return;

  const QMimeData *md = event->mimeData();
  if ( md->hasImage() ) {
    QImage image = qvariant_cast<QImage>(md->imageData());
    mPicture.setData( image );
    updateGui();
    emit changed();
  }

  KUrl::List urls = KUrl::List::fromMimeData( md );
  if ( urls.isEmpty() ) { // oops, no data
    event->setAccepted( false );
  } else {
    if ( mImageLoader ) {
      bool ok = false;
      KABC::Picture pic = mImageLoader->loadPicture( urls.first(), &ok );
      if ( ok ) {
        mPicture = pic;
        updateGui();
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
  if ( (event->buttons() & Qt::LeftButton) &&
       (event->pos() - mDragStartPos).manhattanLength() >
       KGlobalSettings::dndEventDelay() ) {
    startDrag();
  }
}

void ImageButton::contextMenuEvent( QContextMenuEvent *event )
{
  QMenu menu;
  menu.addAction( i18n( "Reset" ), this, SLOT( clear() ) );
  menu.exec( event->globalPos() );
}

void ImageButton::load()
{
  KUrl url = KFileDialog::getOpenUrl( QString(), KImageIO::pattern(), this );
  if ( url.isValid() ) {
    if ( mImageLoader ) {
      bool ok = false;
      KABC::Picture pic = mImageLoader->loadPicture( url, &ok );
      if ( ok ) {
        mPicture = pic;
        updateGui();
        emit changed();
      }
    }
  }
}

void ImageButton::clear()
{
  mPicture = KABC::Picture();
  updateGui();

  emit changed();
}

ImageBaseWidget::ImageBaseWidget( const QString &title, QWidget *parent )
  : QWidget( parent ), mReadOnly( false )
{
  mImageLoader = new ImageLoader( this );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QGroupBox *box = new QGroupBox( title, this );
  QVBoxLayout *layout = new QVBoxLayout( box );
  layout->setSpacing( KDialog::spacingHint() );

  mImageButton = new ImageButton( box );
  mImageButton->setFixedSize( 120, 160 );
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


ImageWidget::ImageWidget( KABC::AddressBook *ab, QWidget *parent )
  : KAB::ContactEditorWidget( ab, parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

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
