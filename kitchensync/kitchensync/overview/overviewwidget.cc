
#include "overviewwidget.h"

#include <qvariant.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qmovie.h>
#include <qptrlist.h>
#include <qvbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include <manipulatorpart.h>

using namespace KitchenSync;

OverviewWidget::OverviewWidget( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl ) {

    if ( !name )
        setName( "overviewWidget" );
    //setCaption( i18n( "KitchenSync - Overview" ) );

    deviceName = new QLabel( this, "deviceName" );
    deviceName->setGeometry( QRect( 20, 20, 200, 40 ) );
    deviceName->setText( i18n( "<h2>Device</h2>" ) );
    deviceName->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

    nameField = new QLabel ( this, "namField" );
    nameField->setGeometry( QRect( 20, 80, 200, 40 ) );
    nameField->setText( i18n("Name") );
    nameField->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

    deviceLogo = new QLabel( this, "deviceLogo" );
    deviceLogo->setGeometry( QRect( width()-140, 20, 120, 120 ) );
    //deviceLogo->setScaledContents( TRUE );

    Line = new QFrame( this, "Line" );
    Line->setGeometry( QRect( 0, 120, 611, 20 ) );
    Line->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0,  Line->sizePolicy().hasHeightForWidth() ) );
    Line->setProperty( "frameShape", (int)QFrame::HLine );
    Line->setFrameShadow( QFrame::Sunken );
    Line->setFrameShape( QFrame::HLine );


    //QLabel *aniIcon = new QLabel( this);
    //aniIcon->setMovie(KGlobal::iconLoader()->loadMovie("image", KIcon::NoGroup, 32));

    QLabel *filler = new QLabel(this);

    sv = new QScrollView( this );
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    sv->setGeometry (QRect( 20, 140 , this->width()-100 , this->height()-260 ) );
    sv->setFrameShape(QFrame::NoFrame);

    sv->addChild(filler);
    sv->show();
}


void OverviewWidget::showList(QPtrList<ManipulatorPart> list) {

    QVBox* progressLayout = new QVBox( sv->viewport() );

    ManipulatorPart* currentPart;
    for (currentPart = list.first(); currentPart != 0; currentPart = list.next()) {

        QPixmap *image = currentPart->pixmap();
        QString text = currentPart->name();

        NewProgress *test = new NewProgress(*image, text, progressLayout);

        sv->addChild(test);
    }
}

void OverviewWidget::setDeviceName(QString name) {
    deviceName->setText( i18n("<h2> %1 </h2>").arg(name) );
}

void OverviewWidget::setNameField(QString name) {
    nameField->setText( i18n("<b> %1 <b>").arg(name) );
}

void OverviewWidget::setLogo(QPixmap image0) {
    deviceLogo->setPixmap( image0 );
}


NewProgress::NewProgress( QPixmap &icon,
			  QString text,
			  QWidget* parent = 0,
			  const char* name = 0,
			  WFlags fl = 0) : QWidget(parent, name, fl) {

    progressItemPix = new QLabel( this, "progressItemPix" );
    progressItemPix->setGeometry( QRect ( 20, 0, 20, 20) );
    progressItemPix->setPixmap( icon );

    progressLabel = new QLabel( this, "progressLabel" );
    progressLabel->setGeometry( QRect( 50, 0, 440, 20 ) );
    progressLabel->setText( i18n( "<b> %1 <b>" ).arg(text) );
    progressLabel->setAlignment( int( QLabel::AlignTop | QLabel::AlignLeft ) );

    statusLabel = new QLabel ( this, "statusLabel" );
    statusLabel->setGeometry( QRect ( 450, 0, 20, 20) );
}

void NewProgress::setProgressItemPix(QPixmap image) {
    progressItemPix->setPixmap( image );
}

void NewProgress::setProgressLabel(QString text) {
    progressLabel->setText( i18n(text) );
}

void NewProgress::setStatusLabel(int status) {

    QPixmap workingIcon( " " );
    QPixmap doneIcon(" ");
    if (status=0) {
        progressItemPix->setPixmap( workingIcon );
    } else {
      progressItemPix->setPixmap( doneIcon );
    }
}

void NewProgress::timerEvent(QTimerEvent *) {
    progressItemPix->setMovie(KGlobal::iconLoader()->loadMovie("image", KIcon::NoGroup, 32));
}

NewProgress::~NewProgress() {
}

OverviewWidget::~OverviewWidget() {
}

