#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './doc-conflictdialog.ui'
**
** Created: Son Dez 29 17:54:10 2002
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "doc-conflictdialog.h"

#include <qvariant.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a Form1 as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
Form1::Form1( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )

{
	if ( !name ) setName( "ResolutionDialog" );
	QGrideLayout Form1Layout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout"); 

	groupBox1 = new QGroupBox( this, "groupBox1" );
	groupBox1->setColumnLayout(0, Qt::Vertical );
//	groupBox1->layout()->setSpacing( 6 );
//	groupBox1->layout()->setMargin( 11 );
	groupBox1Layout = new QGridLayout( groupBox1->layout() );
	groupBox1Layout->setAlignment( Qt::AlignTop );

	textLabel2 = new QLabel( groupBox1, "textLabel2" );

	groupBox1Layout->addWidget( textLabel2, 0, 0 );

	fDBResolution_1 = new QComboBox( FALSE, groupBox1, "fDBResolution_1" );
//	fDBResolution_1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, fDBResolution_1->sizePolicy().hasHeightForWidth() ) );
	fDBResolution_1->setDuplicatesEnabled( FALSE );

	groupBox1Layout->addWidget( fDBResolution_1, 0, 1 );

	fDBResolution_2 = new QComboBox( FALSE, groupBox1, "fDBResolution_2" );
//	fDBResolution_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, fDBResolution_2->sizePolicy().hasHeightForWidth() ) );
	fDBResolution_2->setDuplicatesEnabled( FALSE );

	groupBox1Layout->addWidget( fDBResolution_2, 1, 1 );
	textLabel2_2 = new QLabel( groupBox1, "textLabel2_2" );

	groupBox1Layout->addWidget( textLabel2_2, 1, 0 );
	pushButton1_2 = new QPushButton( groupBox1, "pushButton1_2" );
	groupBox1Layout->addWidget( pushButton1_2, 1, 2 );

    pushButton1 = new QPushButton( groupBox1, "pushButton1" );

    groupBox1Layout->addWidget( pushButton1, 0, 2 );

    Form1Layout->addWidget( groupBox1, 2, 0 );
    QSpacerItem* spacer = new QSpacerItem( 31, 250, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Form1Layout->addItem( spacer, 3, 0 );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );

    Form1Layout->addWidget( textLabel1, 0, 0 );

    textLabel1_2 = new QLabel( this, "textLabel1_2" );
    textLabel1_2->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );

    Form1Layout->addWidget( textLabel1_2, 1, 0 );
    languageChange();
    resize( QSize(510, 467).expandedTo(minimumSizeHint()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Form1::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    groupBox1->setTitle( tr2i18n( "DOC Databases" ) );
    textLabel2->setText( tr2i18n( "Database name" ) );
    fDBResolution_1->clear();
    fDBResolution_1->insertItem( tr2i18n( "No Resolution" ) );
    fDBResolution_1->insertItem( tr2i18n( "PC overrides" ) );
    fDBResolution_1->insertItem( tr2i18n( "Handheld overrides" ) );
    fDBResolution_1->insertItem( tr2i18n( "Delete both databases" ) );
    fDBResolution_2->clear();
    fDBResolution_2->insertItem( tr2i18n( "No Resolution" ) );
    fDBResolution_2->insertItem( tr2i18n( "PC overrides" ) );
    fDBResolution_2->insertItem( tr2i18n( "Handheld overrides" ) );
    fDBResolution_2->insertItem( tr2i18n( "Delete both databases" ) );
    fDBResolution_2->setCurrentItem( 0 );
    textLabel2_2->setText( tr2i18n( "Database name" ) );
    pushButton1_2->setText( tr2i18n( "More info..." ) );
    pushButton1->setText( tr2i18n( "More info..." ) );
    textLabel1->setText( tr2i18n( "Here is a list of all text files and DOC databases the conduit found. The conduit tried to determine the correct sync direction, but for databases in bold red letters a conflict occured (i.e. the text was changed both on the desktop and on the handheld). For these databases please specify which version is the current one." ) );
    textLabel1_2->setText( tr2i18n( "You can also change the sync direction for databases without a conflict." ) );
}

#include "doc-conflictdialog.moc"
