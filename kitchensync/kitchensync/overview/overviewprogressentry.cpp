#include <kiconloader.h>
#include <kglobal.h>

#include <qimage.h>

#include "overviewprogressentry.h"

using namespace KSync;
using namespace KSync::OverView;

OverViewProgressEntry::OverViewProgressEntry( QWidget* parent, const char* name ) : QWidget( parent, name ) {

    m_layout = new QHBoxLayout( this, QBoxLayout::LeftToRight );
    m_layout->setSpacing( 5 );

    m_pixmapLabel = new QLabel( this, "pixmap" );
    m_pixmapLabel->setFixedWidth( 16 );
    m_textLabel = new QLabel( this, "text" );
    m_progressField = new QLabel( this, "progress" );

    m_layout->addWidget( m_pixmapLabel, 0, AlignLeft );
    m_layout->addWidget( m_textLabel, 5, AlignLeft );
    m_layout->addStretch(0);
    m_layout->addWidget( m_progressField, 0, AlignLeft );

}

OverViewProgressEntry::~OverViewProgressEntry() {
}

void OverViewProgressEntry::setText( QString text ) {
    m_textLabel->setText( text );
}

void OverViewProgressEntry::setProgress( QString text ) {
    m_progressField->setText( text );
}

void OverViewProgressEntry::setPixmap( QPixmap pixmap ) {
    QImage test = pixmap.convertToImage();
    m_pixmapLabel->setPixmap( test.smoothScale( 16, 16, QImage::ScaleMin ) );
}

#include "overviewprogressentry.moc"
