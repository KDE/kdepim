
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
#include <qvbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kicontheme.h>

using namespace KitchenSync;

OverviewWidget::OverviewWidget( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl ) {

  QPixmap image0( "../kitchensync.png"  );
  if ( !name )
    setName( "overviewWidget" );
  resize( 731, 593 ); 
  setCaption( i18n( "Overview" ) );
  
  deviceName = new QLabel( this, "deviceName" );
  deviceName->setGeometry( QRect( 20, 20, 280, 40 ) ); 
  deviceName->setText( i18n( "<h2>Device</h2>" ) );
  deviceName->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );
  
  nameField = new QLabel ( this, "namField" );
  nameField->setGeometry( QRect( 20, 80, 280, 40 ) );
  nameField->setText( i18n("Name") );
  nameField->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

  deviceLogo = new QLabel( this, "deviceLogo" );
  deviceLogo->setGeometry( QRect( width()-180, 20, 120, 120 ) ); 
  deviceLogo->setPixmap( image0 );
  //deviceLogo->setScaledContents( TRUE );

  Line = new QFrame( this, "Line" );
  Line->setGeometry( QRect( 0, 240, 811, 20 ) ); 
  Line->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0,  Line->sizePolicy().hasHeightForWidth() ) );
  Line->setProperty( "frameShape", (int)QFrame::HLine );
  Line->setFrameShadow( QFrame::Sunken );
  Line->setFrameShape( QFrame::HLine );

  
  //progressWindow->setMovie(KGlobal::iconLoader()->loadAnimated("working", KIcon::Standard, 15));;
  
  sv = new QScrollView( this );
  sv->setResizePolicy(QScrollView::AutoOneFit);
  sv->setHScrollBarMode( QScrollView::AlwaysOff );
  sv->setGeometry (QRect( 20, 280 , this->width()-20 , this->height()-280 ) );
  sv->setFrameShape(QFrame::NoFrame);
  
  sv->show();
  showProgressPart();
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


void OverviewWidget::showProgressPart() {
  QPixmap image0( "../kitchensync.png"  );

  QVBox* progressLayout = new QVBox( sv->viewport() );

  progressLayout->setBackgroundColor(Qt::red );
  NewProgress *test = new NewProgress(image0, "test",0, progressLayout);

  sv->addChild(progressLayout);
}

NewProgress::NewProgress( QPixmap &icon, 
			  QString text, 
			  bool progress,
			  QWidget* parent = 0,
			  const char* name = 0,
			  WFlags fl = 0) : QWidget(parent, name, fl) {
  
  progressItemPix = new QLabel( this, "progressItemPix" );
  progressItemPix->setGeometry( QRect ( 20, 0, 20, 20) );
  
  progressLabel = new QLabel( this, "progressLabel" );
  progressLabel->setGeometry( QRect( 20, 0, 440, 20 ) ); 
  progressLabel->setText( i18n( "Progress" ) );
  progressLabel->setAlignment( int( QLabel::AlignTop | QLabel::AlignLeft ) );
  
}

OverviewWidget::~OverviewWidget() {
}

