#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './DisplaySettingsForm.ui'
**
** Created: Tue May 8 00:46:21 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "DisplaySettingsForm.h"

#include <kcolorbtn.h>
#include <kcolorbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a DisplaySettingsForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
DisplaySettingsForm::DisplaySettingsForm( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "DisplaySettingsForm" );
    resize( 600, 484 ); 
    setCaption( i18n( "Form1" ) );
    DisplaySettingsFormLayout = new QVBoxLayout( this ); 
    DisplaySettingsFormLayout->setSpacing( 6 );
    DisplaySettingsFormLayout->setMargin( 11 );

    ButtonGroup1 = new QButtonGroup( this, "ButtonGroup1" );
    ButtonGroup1->setTitle( i18n( "Quoted text colors" ) );
    ButtonGroup1->setColumnLayout(0, Qt::Vertical );
    ButtonGroup1->layout()->setSpacing( 0 );
    ButtonGroup1->layout()->setMargin( 0 );
    ButtonGroup1Layout = new QGridLayout( ButtonGroup1->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( 6 );
    ButtonGroup1Layout->setMargin( 11 );

    kcb_depth2 = new KColorButton( ButtonGroup1, "kcb_depth2" );
    kcb_depth2->setText( QString::null );
    kcb_depth2->setColor( QColor( 0, 85, 0 ) );

    ButtonGroup1Layout->addWidget( kcb_depth2, 1, 2 );

    TextLabel2 = new QLabel( ButtonGroup1, "TextLabel2" );
    TextLabel2->setText( i18n( "Depth &2" ) );

    ButtonGroup1Layout->addWidget( TextLabel2, 1, 0 );

    TextLabel1 = new QLabel( ButtonGroup1, "TextLabel1" );
    TextLabel1->setText( i18n( "Depth &1" ) );

    ButtonGroup1Layout->addWidget( TextLabel1, 0, 0 );

    kcb_depth1 = new KColorButton( ButtonGroup1, "kcb_depth1" );
    kcb_depth1->setText( QString::null );
    kcb_depth1->setColor( QColor( 0, 85, 127 ) );

    ButtonGroup1Layout->addWidget( kcb_depth1, 0, 2 );

    l_depth1 = new QLabel( ButtonGroup1, "l_depth1" );
    l_depth1->setText( i18n( "> Example" ) );
    l_depth1->setFrameShape( QLabel::Box );
    l_depth1->setFrameShadow( QLabel::Sunken );

    ButtonGroup1Layout->addWidget( l_depth1, 0, 1 );

    l_depth2 = new QLabel( ButtonGroup1, "l_depth2" );
    l_depth2->setText( i18n( "> > Example" ) );
    l_depth2->setFrameShape( QLabel::Box );
    l_depth2->setFrameShadow( QLabel::Sunken );

    ButtonGroup1Layout->addWidget( l_depth2, 1, 1 );
    DisplaySettingsFormLayout->addWidget( ButtonGroup1 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    DisplaySettingsFormLayout->addItem( spacer );

    // tab order
    setTabOrder( kcb_depth1, kcb_depth2 );

    // buddies
    TextLabel2->setBuddy( kcb_depth2 );
    TextLabel1->setBuddy( kcb_depth1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
DisplaySettingsForm::~DisplaySettingsForm()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "DisplaySettingsForm.moc"
