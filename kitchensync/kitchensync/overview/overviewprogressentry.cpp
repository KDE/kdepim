#include <kiconloader.h>

#include <qimage.h>

#include "overviewprogressentry.h"

using namespace KSync;
using namespace KSync::OverView;

OverViewProgressEntry::OverViewProgressEntry( QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  QHBoxLayout *layout = new QHBoxLayout( this, QBoxLayout::LeftToRight );
  layout->setSpacing( 5 );

  QWidget* spacer = new QWidget( this );
  spacer->setMinimumWidth( 5 );

  layout->addWidget( spacer, 0, AlignLeft );

  m_pixmapLabel = new QLabel( this );
  m_pixmapLabel->setFixedWidth( 16 );
  m_textLabel = new QLabel( this );
  m_progressField = new QLabel( this );

  spacer = new QWidget( this );
  spacer->setMinimumWidth( 10 );

  layout->addWidget( m_pixmapLabel, 0, AlignLeft );
  layout->addWidget( m_textLabel, 5, AlignLeft );
  layout->addStretch( 0 );
  layout->addWidget( m_progressField, 0, AlignLeft );
  layout->addWidget( spacer, 0, AlignRight );
}

OverViewProgressEntry::~OverViewProgressEntry()
{
}

void OverViewProgressEntry::setText( QString text )
{
  m_textLabel->setText( text );
  m_name = text;
}

void OverViewProgressEntry::setProgress( int status )
{
  // SyncStatus { SYNC_START=0, SYNC_PROGRESS=1,  SYNC_DONE=2,  SYNC_FAIL };

  if ( status == 0 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "player_play", KIcon::Desktop, 16 ) );
  } else if ( status == 1 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "reload", KIcon::Desktop, 16 ) );
  } else if ( status == 2 ) {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "ok", KIcon::Desktop, 16 ) );
  } else {
    m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon( "no", KIcon::Desktop, 16 ) );
  }
}

void OverViewProgressEntry::setPixmap( QPixmap pixmap )
{
  QImage test = pixmap.convertToImage();
  m_pixmapLabel->setPixmap( test.smoothScale( 16, 16, QImage::ScaleMin ) );
}

QString OverViewProgressEntry::name()
{
  return m_name;
}

#include "overviewprogressentry.moc"
