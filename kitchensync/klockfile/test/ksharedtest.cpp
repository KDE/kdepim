#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './ksharedtest.ui'
**
** Created: Wed Feb 6 23:10:18 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "ksharedtest.h"

#include <qvariant.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include "ksharedtest.ui.h"

/* 
 *  Constructs a Form1 which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
Form1::Form1( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "Form1" );
    resize( 445, 165 ); 
    setCaption( tr2i18n( "Form1" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );

    ToolButton1 = new QToolButton( centralWidget(), "ToolButton1" );
    ToolButton1->setGeometry( QRect( 390, 20, 20, 24 ) ); 
    ToolButton1->setText( tr2i18n( "..." ) );

    LineEdit1 = new QLineEdit( centralWidget(), "LineEdit1" );
    LineEdit1->setGeometry( QRect( 40, 20, 331, 25 ) ); 

    PushButton3 = new QPushButton( centralWidget(), "PushButton3" );
    PushButton3->setGeometry( QRect( 220, 60, 150, 29 ) ); 
    PushButton3->setText( tr2i18n( "Remove Read Lock" ) );

    PushButton2 = new QPushButton( centralWidget(), "PushButton2" );
    PushButton2->setGeometry( QRect( 40, 100, 150, 29 ) );
    PushButton2->setText( tr2i18n( "Write Lock" ) );

    PushButton4 = new QPushButton( centralWidget(), "PushButton4" );
    PushButton4->setGeometry( QRect( 220, 100, 150, 29 ) );
    PushButton4->setText( tr2i18n( "Remove Write Lock" ) );

    PushButton1 = new QPushButton( centralWidget(), "PushButton1" );
    PushButton1->setGeometry( QRect( 40, 60, 150, 29 ) );
    PushButton1->setText( tr2i18n( "Read Lock" ) );

    // actions
    Action = new QAction( this, "Action" );
    Action->setText( tr2i18n( "Action" ) );


    // toolbars


    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( slotReadLock() ) );
    connect( PushButton3, SIGNAL( clicked() ), this, SLOT( slotReadUnlock() ) );
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( slotWriteLock() ) );
    connect( PushButton4, SIGNAL( clicked() ), this, SLOT( slotWriteUnlock() ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "ksharedtest.moc"
