#include "selectfields.h"
#include "attributes.h"

#include <qlistbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlabel.h>

#include <klocale.h>

////////////////////////
// SelectFields Methods

SelectFields::SelectFields( QStringList oldFields,
			    QWidget * parent, 
			    const char * name, 
			    bool modal )
  : QDialog( parent, name, modal )
{
  setCaption( "Select fields to display" );

  QGridLayout *gl = new QGridLayout( this, 6, 3, 10 );
  gl->setSpacing( 10 );

  cbUnselected = new QComboBox( false, this );
  QString tmp;

  for (int i = 0; 
       tmp = i18n(Attributes::instance()->fieldListName( i )), tmp != "";
       ++i )
    cbUnselected->insertItem( tmp );
  cbUnselected->setCurrentItem( 1 );
  
  gl->addWidget( cbUnselected, 0, 0 );
  //  lUnselected->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  QLabel *lSelected = new QLabel( i18n( "Selected fields" ), this );
  gl->addWidget( lSelected, 0, 2 );
  lSelected->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );

  lbUnSelected = new QListBox( this );
  lbUnSelected->setSelectionMode( QListBox::Extended );
  gl->addWidget( lbUnSelected, 1, 0 );
  lbSelected = new QListBox( this );
  lbSelected->setSelectionMode( QListBox::Extended );
  gl->addWidget( lbSelected, 1, 2 );
  
  QBoxLayout *vb1 = new QBoxLayout( QBoxLayout::TopToBottom, 10 );
  vb1->addStretch();
  QPushButton *pbAdd = new QPushButton( i18n( "&Add >>" ), this );
  QObject::connect( pbAdd, SIGNAL( clicked() ), this, SLOT( select() ));
  vb1->addWidget( pbAdd );
  QPushButton *pbRemove = new QPushButton( i18n( "<< &Remove" ), this );
  QObject::connect( pbRemove, SIGNAL( clicked() ), this, SLOT( unselect() ));
  vb1->addWidget( pbRemove );
  vb1->addStretch();
  gl->addLayout( vb1, 1, 1 );

  QBoxLayout *hb1 = new QBoxLayout( QBoxLayout::LeftToRight, 10 );
  QLabel *lCustomField = new QLabel( i18n( "Custom Field" ), this );
  hb1->addWidget( lCustomField );
  leCustomField = new QLineEdit( this );
  QObject::connect( leCustomField, SIGNAL( returnPressed() ), 
		    this, SLOT( addCustom() ));

  hb1->addWidget( leCustomField );
  gl->addMultiCell( hb1, 2, 2, 0, 2, QGridLayout::AlignRight );

  QFrame *bar1 = new QFrame( this, "horizontal divider 1" );
  bar1->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  bar1->setMinimumHeight( 10 );
  bar1->setMaximumHeight( 10 );
  gl->addMultiCellWidget( bar1, 3, 3, 0, 2 );

  QBoxLayout *hb2 = new QBoxLayout( QBoxLayout::LeftToRight, 10 );
  hb2->addStretch();
  QPushButton *pbOk = new QPushButton( i18n("&OK"), this );
  //  pbOk->setFocus();
  
  QObject::connect( pbOk, SIGNAL( clicked() ), this, SLOT( accept() ));
  hb2->addWidget( pbOk );
  QPushButton *pbCancel = new QPushButton( i18n("Cancel"), this );
  QObject::connect( pbCancel, SIGNAL( clicked() ), this, SLOT( reject() ));
  hb2->addWidget( pbCancel );
  gl->addMultiCell( hb2, 4, 4, 0, 2, QGridLayout::AlignRight );

  QStringList ignored;
  QStringList unSelectedNames;
  Attributes::instance()->nameFieldList( cbUnselected->currentItem(), 
					 &unSelectedNames,
					 &ignored );

  QStringList selectedNames;
  QStringList::Iterator it;
  for(it = oldFields.begin(); it != oldFields.end(); ++it) {
    QString name = Attributes::instance()->fieldToName( *it );
    selectedNames += name;
    QStringList::Iterator cit = unSelectedNames.find( name );
    if (cit != unSelectedNames.end())
      unSelectedNames.remove( cit ); 
  }
  selectedNames.sort();

  lbUnSelected->insertStringList( unSelectedNames );
  lbSelected->insertStringList( selectedNames  );
  QSize lbSizeHint = lbUnSelected->sizeHint();
  lbSizeHint = lbSizeHint.expandedTo( lbSelected->sizeHint() );
  lbUnSelected->setMinimumSize( lbSizeHint );
  lbSelected->setMinimumSize( lbSizeHint );

  QObject::connect( cbUnselected, SIGNAL( activated(int) ),
		    this, SLOT( showFields(int) ));

  gl->activate();
}

void SelectFields::showFields( int index )
{
  QStringList ignored;
  QStringList unSelectedNames;
  Attributes::instance()->nameFieldList( index,
					 &unSelectedNames,
					 &ignored );

  for(uint i = 0; i < lbSelected->count(); ++i) {
    QString name = lbSelected->text( i );
    QStringList::Iterator cit = unSelectedNames.find( name );
    if (cit != unSelectedNames.end())
      unSelectedNames.remove( cit ); 
  }
  lbUnSelected->clear();
  lbUnSelected->insertStringList( unSelectedNames );
}

void SelectFields::select()
{
  for(uint i = 0; i < lbUnSelected->count(); ++i)
    if (lbUnSelected->isSelected( lbUnSelected->item( i ))) {
      QString item = lbUnSelected->item( i )->text();
      QString lItem = item.lower();
      uint j;
      for(j = 0; j < lbSelected->count(); ++j) {
	if (lbSelected->text( j ).lower() > lItem)
	  break;
      }
      lbSelected->insertItem( item, j );
      lbUnSelected->removeItem( i );
      --i;
    }
}

void SelectFields::unselect()
{
  for(uint i = 0; i < lbSelected->count(); ++i)
    if (lbSelected->isSelected( lbSelected->item( i ))) {
      QString item = lbSelected->item( i )->text();
      QString lItem = item.lower();
      uint j = 0;
      for(j = 0; j < lbUnSelected->count(); ++j) {
	if (lbUnSelected->text( j ).lower() > lItem)
	  break;
      }
      lbUnSelected->insertItem( item, j );
      lbSelected->removeItem( i );
      --i;
    }
}

QStringList SelectFields::chosenFields()
{
  QStringList result;
  uint i;
  for(i = 0; i < lbSelected->count(); ++i) {
    QString field = Attributes::instance()->nameToField( lbSelected->text( i ) );
    if (field != "")
      result += field;
    else
      result += "X-CUSTOM-" + lbSelected->text( i );
  }
  return result;
}

void SelectFields::addCustom()
{
  QString item = leCustomField->text();
  if (item == "")
    return;
  QString lItem = item.lower();
    uint i = 0;
  for(i = 0; i < lbSelected->count(); ++i) {
    if (lbSelected->text( i ).lower() > lItem)
      break;
  }
  lbSelected->insertItem( item, i );
  leCustomField->clear();
}
