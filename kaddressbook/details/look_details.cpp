/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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

#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qcursor.h>
#include <qdir.h>
#include <qpainter.h>
#include <qpopupmenu.h>

#include "global.h"
#include "kabentrypainter.h"

#include "look_details.h"

#define GRID 5

const QString KABDetailedView::mBorderedBGDir = "kab3part/backgrounds/bordered/";
const QString KABDetailedView::mTiledBGDir = "kab3part/backgrounds/tiled/";

KABDetailedView::KABDetailedView( QWidget *parent, const char *name )
  : KABBasicLook( parent, name ), mPainter( 0 ), mBackgroundStyle( None ),
    mDefaultBGColor( white ), mHeadLineBGColor( darkBlue ),
    mHeadLineTextColor( yellow ), mGrid( 3 ), mMenuBorderedBG( 0 ),
    mMenuTiledBG( 0 )
{
  KToggleAction** actions[] = {
    &mActionShowAddresses,
    &mActionShowEmails,
    &mActionShowPhones,
    &mActionShowURLs
  };

  QString actionTexts[] = {
    i18n( "Show Postal Addresses" ),
    i18n( "Show Email Addresses" ),
    i18n( "Show Telephone Numbers" ),
    i18n( "Show Web Pages (URLs)" )
  };

  QFont general = KGlobalSettings::generalFont();
  QFont fixed = KGlobalSettings::fixedFont();
  QString gfont = general.family();
  QString ffont = fixed.family();

  int gpointsize = general.pixelSize();
  if ( gpointsize == -1 )
    gpointsize = general.pointSize();

  int fpointsize = fixed.pixelSize();
  if ( fpointsize == -1 )
    fpointsize = fixed.pointSize();

  mPainter = new KABEntryPainter;

  mPainter->setForegroundColor( black );
  mPainter->setHeaderColor( mHeadLineTextColor );
  mPainter->setUseHeaderColor( mUseHeadLineBGColor );
  mPainter->setBackgroundColor( mHeadLineBGColor );

  mPainter->setHeaderFont( QFont( gfont, gpointsize + 4, QFont::Bold, true ) );
  mPainter->setHeadLineFont( QFont( gfont, gpointsize + 2, QFont::Bold, true ) );
  mPainter->setBodyFont( QFont( gfont, gpointsize, QFont::Normal, false ) );
  mPainter->setFixedFont( QFont( ffont, fpointsize, QFont::Normal, false ) );
  mPainter->setCommentFont( QFont( gfont, gpointsize, QFont::Normal, false ) );

  const int numActions = sizeof( actions ) / sizeof( actions[ 0 ] );

  for ( int count = 0; count < numActions; ++count ) {
    *actions[ count ] = new KToggleAction( actionTexts[ count ] );
    (*actions[ count ])->setChecked( true );
  }

  setMouseTracking( true );

  setBackgroundMode( NoBackground );
}

KABDetailedView::~KABDetailedView()
{
  delete mPainter;
  mPainter = 0;
}

bool KABDetailedView::getBackground( QString path, QPixmap& image )
{
  QMap<QString, QPixmap>::iterator pos;

  pos = mBackgroundMap.find( path );
  if ( pos == mBackgroundMap.end() ) { // the image has not been loaded previously
    if ( image.load( path ) ) {
      mBackgroundMap[ path ] = image;
      return true;
    } else
      return false;
  } else { // image found in cache
    image = pos.data();
    return true;
  }
}

void KABDetailedView::paintEvent( QPaintEvent* )
{
  const int BorderSpace = mGrid;
  QPixmap pm( width(), height() );
  QPainter p;

  QRect entryArea = QRect( BorderSpace, mGrid, width() - mGrid - BorderSpace,
                           height() - 2 * mGrid );
  p.begin( &pm );

  p.setPen( darkBlue );
  p.setBrush( mDefaultBGColor );
  p.drawRect( 0, 0, width(), height() );
  switch ( mBackgroundStyle ) {
    case Tiled:
      p.drawTiledPixmap( 1, 1, width() - 2, height() - 2, mCurrentBackground );
      break;
    case Bordered:
      p.drawTiledPixmap( 1, 1, QMIN( width() - 2, mCurrentBackground.width() ),
                         height() - 2, mCurrentBackground );
      break;
    case None: // no BG image defined for this entry:
    default:
      if ( mUseDefaultBGImage )
        p.drawTiledPixmap( 1, 1, width() - 2, height() - 2, mDefaultBGImage );
      break;
  };

  p.setViewport( entryArea );

  mPainter->setShowAddresses( mActionShowAddresses->isChecked() );
  mPainter->setShowEmails( mActionShowEmails->isChecked() );
  mPainter->setShowPhones( mActionShowPhones->isChecked() );
  mPainter->setShowURLs( mActionShowURLs->isChecked() );
  mPainter->printAddressee( addressee(), QRect( 0, 0, entryArea.width(),
                            entryArea.height() ), &p );
  p.end();
  bitBlt( this, 0, 0, &pm );
}

void KABDetailedView::mouseMoveEvent( QMouseEvent *e )
{
  QPoint bias( mGrid, mGrid );
  int rc;
  bool hit = false;

  if ( ( rc = mPainter->hitsEmail( e->pos() - bias ) ) != -1 )
    hit = true;
  else if ( ( rc = mPainter->hitsURL( e->pos() - bias ) ) != -1 )
    hit = true;
  else if ( ( rc = mPainter->hitsPhone( e->pos() - bias ) ) != -1 )
    hit = true;
  else if ( ( rc = mPainter->hitsTalk( e->pos() - bias ) ) != -1 )
    hit = true;

  if ( hit ) {
    if ( cursor().shape() != PointingHandCursor )
      setCursor( PointingHandCursor );
    else if( cursor().shape() != ArrowCursor )
      setCursor(ArrowCursor);
  }
}

void KABDetailedView::mousePressEvent( QMouseEvent *e )
{
  QPopupMenu menu( this );
  QPopupMenu *menuBG = new QPopupMenu( &menu );
  mMenuBorderedBG = new QPopupMenu( &menu );
  mMenuTiledBG = new QPopupMenu( &menu );

  menu.insertItem( i18n( "Select Background" ), menuBG );
  menuBG->insertItem( i18n( "Bordered Backgrounds" ), mMenuBorderedBG );
  menuBG->insertItem( i18n( "Tiled Backgrounds" ), mMenuTiledBG );
  menu.insertSeparator();

  QPoint point = e->pos() - QPoint( mGrid, mGrid );
  int rc;
  QStringList dirsBorderedBG, dirsTiledBG;
  QDir dir;

  switch( e->button() ) {
    case QMouseEvent::RightButton:
      if ( isReadOnly() )
        menu.setItemEnabled( menu.idAt( 0 ), false );
      else {
        // TODO: settings need to be saved in view options
        dirsBorderedBG = KGlobal::instance()->dirs()->findDirs( "data", mBorderedBGDir );
        if ( dirsBorderedBG.count() > 0 ) {
          dir.setPath( dirsBorderedBG[ 0 ] );
          mBorders = dir.entryList( QDir::Files );
          for ( uint count = 0; count < mBorders.count(); ++count )
            mMenuBorderedBG->insertItem( mBorders[ count ], count );

          connect( mMenuBorderedBG, SIGNAL( activated( int ) ),
                   SLOT( slotBorderedBGSelected( int ) ) );
        } else
          menuBG->setItemEnabled( menuBG->idAt( 0 ), false );

        dirsTiledBG = KGlobal::instance()->dirs()->findDirs( "data", mTiledBGDir );
        if ( dirsTiledBG.count() > 0 ) {
          dir.setPath( dirsTiledBG[ 0 ] );
          mTiles = dir.entryList( QDir::Files );
          for ( uint count = 0; count < mTiles.count(); ++count )
            mMenuTiledBG->insertItem( mTiles[ count ], count );

          connect( mMenuTiledBG, SIGNAL( activated( int ) ),
                   SLOT( slotTiledBGSelected( int ) ) );
        } else
          menuBG->setItemEnabled( menuBG->idAt( 1 ), false );
      }

      mActionShowAddresses->plug( &menu );
      mActionShowEmails->plug( &menu );
      mActionShowPhones->plug( &menu );
      mActionShowURLs->plug( &menu );

      menu.exec( e->globalPos() );
      break;

    case QMouseEvent::LeftButton:
      // find whether the pointer touches an email address, URL,
      // talk address or telephone number:
      if ( ( rc = mPainter->hitsEmail( point ) ) != -1 ) {
        emit sendEmail( addressee().emails()[ rc ] );
        break;
      }
      if ( ( rc = mPainter->hitsURL( point ) ) != -1 ) {
        emit browse( addressee().url().prettyURL() );
        break;
      }
      if ( ( rc = mPainter->hitsPhone( point ) ) != -1 ) {
        // not implemented yet
        break;
      }
      if ( ( rc = mPainter->hitsTalk( point ) ) != -1 ) {
        // not implemented yet
        break;
      }
      break;
    default:
      break;
  }

  mMenuBorderedBG = 0;
  mMenuTiledBG = 0;
}

void KABDetailedView::setAddressee( const KABC::Addressee &addr )
{
  BackgroundStyle style = None;
  QString dir, file, styleSetting;
  KABBasicLook::setAddressee( addr );

  // TODO: preload path and styleSetting with possible preference values
  styleSetting = addressee().custom( "kab", "BackgroundStyle" );
  style = (BackgroundStyle)styleSetting.toInt();
  file = addressee().custom( "kab", "BackgroundImage" );
  if ( !file.isEmpty() ) {
    switch ( style ) {
      case Tiled:
        dir = mTiledBGDir;
        break;
      case Bordered:
        dir = mBorderedBGDir;
        break;
      case None:
      default:
        break;
    }

    QStringList dirs = KGlobal::instance()->dirs()->findDirs( "data", dir );
    mBackgroundStyle = None;
    if ( !dirs.isEmpty() ) {
      uint count = 0;
      for ( ; count < dirs.count(); ++count ) {
        QDir folder;
        folder.setPath( dirs[ count ] );
        file = folder.absPath() + "/" + file;
        if ( getBackground( file, mCurrentBackground ) ) {
          mBackgroundStyle = style;
          break;
        }
      }

      if ( count == dirs.count() ) {
        kdDebug(5720) << "KABDetailedView::setEntry: " << file
                      << " not locatable." << endl;
      }
    }
  } else { // no background here
    mBackgroundStyle = None;
    mCurrentBackground.resize( 0, 0 );
  }

  repaint( false );
}

void KABDetailedView::slotBorderedBGSelected( int index )
{
  if ( index >= 0 && (uint)index < mBorders.count() && !isReadOnly() ) {
    // get the selection and make it a full path
    QString path = mBorders[ index ];
    mBackgroundStyle = Bordered;
    addressee().insertCustom( "kab", "BackgroundStyle",
                              QString().setNum( mBackgroundStyle ) );
    addressee().insertCustom( "kab", "BackgroundImage", path );
    setAddressee( addressee() );
  }
}

void KABDetailedView::slotTiledBGSelected( int index )
{
  if ( index >= 0 && (uint)index < mTiles.count() && !isReadOnly() ) {
    QString path = mTiles[ index ];
    mBackgroundStyle = Tiled;
    addressee().insertCustom( "kab", "BackgroundStyle",
                              QString().setNum( mBackgroundStyle ) );
    addressee().insertCustom( "kab", "BackgroundImage", path );
    setAddressee( addressee() );
  }
}

void KABDetailedView::setReadOnly( bool state )
{
  KABBasicLook::setReadOnly( state );
  repaint( false );
}

void KABDetailedView::restoreSettings( KConfig *config )
{
  QFont general = KGlobalSettings::generalFont();
  QFont fixed = KGlobalSettings::fixedFont();
  QString gfont = general.family();
  QString ffont = fixed.family();

  int gpointsize = general.pixelSize();
  if ( gpointsize == -1 )
    gpointsize = general.pointSize();

  int fpointsize = fixed.pixelSize();
  if ( fpointsize == -1 )
    fpointsize = fixed.pointSize();

  config->setGroup( ConfigView );

  // load the default background image:
  QString bgImage;
  mUseDefaultBGImage = config->readBoolEntry( ConfigView_UseDefaultBackground, true );
  mDefaultBGColor = config->readColorEntry( ConfigView_DefaultBackgroundColor, &white );
  bgImage = config->readEntry( ConfigView_DefaultBackgroundImage, "konqueror/tiles/kenwimer.png" );

  if ( mUseDefaultBGImage ) {
    uint count = 0;
    QStringList dirs = KGlobal::instance()->dirs()->findDirs( "data", "/" );
    if ( !dirs.isEmpty() ) {
      for ( count = 0; count < dirs.count(); ++count )  {
        if ( getBackground( dirs[ count ] + "/" + bgImage, mDefaultBGImage ) )
          break;
      }
    }

    if ( count == dirs.count() ) {
      mUseDefaultBGImage = getBackground( bgImage, mDefaultBGImage );
      if ( !mUseDefaultBGImage )
        kdDebug(5720) << "KABDetailedView::configure: "
                      << "default BG image selected, but could not be loaded."
                      << endl;
    }
  }

  mDefaultBGColor = config->readColorEntry( ConfigView_DefaultBackgroundColor, &white );
  mHeadLineBGColor = config->readColorEntry( ConfigView_HeadlineBGColor, &darkBlue );
  mHeadLineTextColor = config->readColorEntry( ConfigView_HeadlineTextColor, &yellow );
  mUseHeadLineBGColor = config->readBoolEntry( ConfigView_UseHeadlineBGColor, true );

  if ( !mPainter )
    mPainter = new KABEntryPainter;

  mPainter->setForegroundColor( black );
  mPainter->setHeaderColor( mHeadLineTextColor );
  mPainter->setUseHeaderColor( mUseHeadLineBGColor );
  mPainter->setBackgroundColor( mHeadLineBGColor );

  mPainter->setHeaderFont( QFont( gfont, gpointsize + 4, QFont::Bold, true ) );
  mPainter->setHeadLineFont( QFont( gfont, gpointsize + 2, QFont::Bold, true ) );
  mPainter->setBodyFont( QFont( gfont, gpointsize, QFont::Normal, false ) );
  mPainter->setFixedFont( QFont( ffont, fpointsize, QFont::Normal, false ) );
  mPainter->setCommentFont( QFont( gfont, gpointsize, QFont::Normal, false ) );
}

#include "look_details.moc"
