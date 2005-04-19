#include "kdhorizontalline.h"

#include <qstyle.h>
#include <qpainter.h>
#ifdef QT_ACCESSIBILITY_SUPPORT
#include <qaccessible.h>
#endif
#include <qfontmetrics.h>
#include <qapplication.h>

KDHorizontalLine::KDHorizontalLine( QWidget * parent, const char * name, WFlags f )
  : QFrame( parent, name, f ),
    mAlign( Qt::AlignAuto ),
    mLenVisible( 0 )
{
  QFrame::setFrameStyle( HLine | Sunken );
}

KDHorizontalLine::KDHorizontalLine( const QString & title, QWidget * parent, const char * name, WFlags f )
  : QFrame( parent, name, f ),
    mAlign( Qt::AlignAuto ),
    mLenVisible( 0 )
{
  QFrame::setFrameStyle( HLine | Sunken );
  setTitle( title );
}

KDHorizontalLine::~KDHorizontalLine() {}

void KDHorizontalLine::setFrameStyle( int style ) {
  QFrame::setFrameStyle( ( style & ~MShape ) | HLine ); // force HLine
}

void KDHorizontalLine::setTitle( const QString & title ) {
  if ( mTitle == title )
    return;
  mTitle = title;
  calculateFrame();
  update();
  updateGeometry();
#ifdef QT_ACCESSIBILITY_SUPPORT
  QAccessible::updateAccessibility( this, 0, QAccessible::NameChanged );
#endif
}

void KDHorizontalLine::calculateFrame() {
  mLenVisible = mTitle.length();
#if 0
  if ( mLenVisible ) {
    const QFontMetrics fm = fontMetrics();
    while ( mLenVisible ) {
      const int tw = fm.width( mTitle, mLenVisible ) + 4*fm.width(QChar(' '));
      if ( tw < width() )
        break;
      mLenVisible--;
    }
    qDebug( "mLenVisible = %d (of %d)", mLenVisible, mTitle.length() );
    if ( mLenVisible ) { // but do we also have a visible label?
      QRect r = rect();
      const int va = style().styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, this );
      if( va & AlignVCenter )
        r.setTop( fm.height() / 2 );		// frame rect should be
      else if( va & AlignTop )
        r.setTop( fm.ascent() );
      setFrameRect( r );			//   smaller than client rect
      return;
    }
  }
  // no visible label
  setFrameRect( QRect(0,0,0,0) );		//  then use client rect
#endif
}

QSizePolicy KDHorizontalLine::sizePolicy() const {
  return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

QSize KDHorizontalLine::sizeHint() const {
  return minimumSizeHint();
}

QSize KDHorizontalLine::minimumSizeHint() const {
  const int w = 2 * 8 // margins on both sides
                + fontMetrics().width( mTitle, mLenVisible )
                + fontMetrics().width( QChar( ' ' ) );
  const int h = fontMetrics().height();
  return QSize( w, h );
}

void KDHorizontalLine::paintEvent( QPaintEvent * e ) {
  QPainter paint( this );

  if ( mLenVisible ) {	// draw title
    const QFontMetrics & fm = paint.fontMetrics();
    const int h = fm.height();
    const int tw = fm.width( mTitle, mLenVisible ) + fm.width(QChar(' '));
    int x;
    const int marg = 8;
    if ( mAlign & AlignHCenter )		// center alignment
      x = frameRect().width()/2 - tw/2;
    else if ( mAlign & AlignRight )	// right alignment
      x = frameRect().width() - tw - marg;
    else if ( mAlign & AlignLeft )       // left alignment
      x = marg;
    else { // auto align
      if( QApplication::reverseLayout() )
        x = frameRect().width() - tw - marg;
      else
        x = marg;
    }
    QRect r( x, 0, tw, h );
    int va = style().styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, this );
    if ( va & AlignTop )
      r.moveBy( 0, fm.descent() );
    const QColor pen( (QRgb) style().styleHint( QStyle::SH_GroupBox_TextLabelColor, this ) );
#if QT_VERSION >= 0x030300
    if ( !style().styleHint( QStyle::SH_UnderlineAccelerator, this ) )
      va |= NoAccel;
#endif
    style().drawItem( &paint, r, ShowPrefix | AlignHCenter | va, colorGroup(),
                      isEnabled(), 0, mTitle, -1, ownPalette() ? 0 : &pen );
    paint.setClipRegion( e->region().subtract( r ) ); // clip everything but title
  }
  drawFrame( &paint );
  drawContents( &paint );
}

#include "kdhorizontalline.moc"
