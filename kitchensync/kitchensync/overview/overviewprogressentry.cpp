#include <kiconloader.h>
#include <kglobal.h>

#include <qimage.h>

#include "overviewprogressentry.h"

using namespace KSync;
using namespace KSync::OverView;

OverViewProgressEntry::OverViewProgressEntry( QWidget* parent, const char* name ) : QWidget( parent, name ) {

    m_layout = new QHBoxLayout( this, QBoxLayout::LeftToRight );
    m_layout->setSpacing( 5 );

    QWidget* spacer = new QWidget( this, "spacer" );
    spacer->setMinimumWidth( 5 );

    m_pixmapLabel = new QLabel( this, "pixmap" );
    m_pixmapLabel->setFixedWidth( 16 );
    m_textLabel = new QLabel( this, "text" );
    m_progressField = new QLabel( this, "progress" );

    QWidget *space = new QWidget( this, "spacer" );
    space->setMinimumWidth( 10 );

    m_layout->addWidget( spacer, 0, AlignLeft );
    m_layout->addWidget( m_pixmapLabel, 0, AlignLeft );
    m_layout->addWidget( m_textLabel, 5, AlignLeft );
    m_layout->addStretch(0);
    m_layout->addWidget( m_progressField, 0, AlignLeft );
    m_layout->addWidget( space, 0, AlignRight );
}

OverViewProgressEntry::~OverViewProgressEntry() {
}

void OverViewProgressEntry::setText( QString text ) {
    m_textLabel->setText( text );
    m_name = text;
}

void OverViewProgressEntry::setProgress( int status ) {
    // SyncStatus { SYNC_START=0, SYNC_PROGRESS=1,  SYNC_DONE=2,  SYNC_FAIL };

    if ( status == 0 )  {
         m_progressField->setPixmap(  KGlobal::iconLoader()->loadIcon("player_play", KIcon::Desktop, 16 ) );
    } else if ( status == 1 )  {
         m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon("reload", KIcon::Desktop, 16 ) );
    } else if ( status == 2 )  {
        m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon("ok", KIcon::Desktop, 16 ) );
    } else {
        m_progressField->setPixmap( KGlobal::iconLoader()->loadIcon("no", KIcon::Desktop, 16 ) );
    }
}

void OverViewProgressEntry::setPixmap( QPixmap pixmap ) {
    QImage test = pixmap.convertToImage();
    m_pixmapLabel->setPixmap( test.smoothScale( 16, 16, QImage::ScaleMin ) );
}

QString OverViewProgressEntry::name()  {
    return m_name;
}

#include "overviewprogressentry.moc"
