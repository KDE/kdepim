#include <qlistbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtoolbutton.h>

#include <klocale.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <kdebug.h>

#include "selectfieldswidget.h"

class FieldItem : public QListBoxText
{
  public:
    FieldItem( QListBox *parent, KABC::Field *field )
      : QListBoxText( parent, field->label() ), mField( field ) {}
    
    FieldItem( QListBox *parent, KABC::Field *field, int index )
      : QListBoxText( parent, field->label(), parent->item( index ) ),
        mField( field ) {}
    
    KABC::Field *field() { return mField; }
    
  private:
    KABC::Field *mField;
};


////////////////////////
// SelectFieldsWidget Methods

SelectFieldsWidget::SelectFieldsWidget( KABC::AddressBook *doc,
                                        const KABC::Field::List &oldFields,
			                QWidget *parent, const char *name )
  : QWidget(parent, name)
{
  initGUI( doc );
  setOldFields(oldFields);
}

SelectFieldsWidget::SelectFieldsWidget( KABC::AddressBook *doc, QWidget *parent,
                                        const char *name)
  : QWidget(parent, name)
{
  initGUI( doc );
}

void SelectFieldsWidget::setOldFields( const KABC::Field::List &oldFields )
{
  // TODO: Honor field category selection.
  KABC::Field::List allFields = mDoc->fields();

  KABC::Field::List::ConstIterator itOld;
  for( itOld = oldFields.begin(); itOld != oldFields.end(); ++itOld ) {
    new FieldItem( lbSelected, *itOld );
  }

  KABC::Field::List::ConstIterator itAll;
  for( itAll = allFields.begin(); itAll != allFields.end(); ++itAll ) {
    for( itOld = oldFields.begin(); itOld != oldFields.end(); ++itOld ) {
      if ( (*itAll)->equals( (*itOld) ) ) break;
    }
    if ( itOld == oldFields.end() ) {
      new FieldItem( lbUnSelected, *itAll );
    }
  }
}

void SelectFieldsWidget::initGUI( KABC::AddressBook *doc )
{
  mDoc = doc;

  setCaption( i18n("Select Fields to Display") );

  int spacing = KDialogBase::spacingHint();
  QGridLayout *gl = new QGridLayout(this , 6, 4, spacing);
  gl->setSpacing( spacing );

  cbUnselected = new QComboBox( false, this );
  QString tmp;

  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::All ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::Frequent ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::Address ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::Email ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::Personal ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::Organization ) );
  cbUnselected->insertItem( KABC::Field::categoryLabel( KABC::Field::CustomCategory ) );

  gl->addWidget( cbUnselected, 0, 0 );
  //  lUnselected->setAlignment( QLabel::AlignTop | QLabel::AlignLeft );
  QLabel *lSelected = new QLabel( i18n( "&Selected fields:" ), this );
  gl->addWidget( lSelected, 0, 2 );
  lSelected->setAlignment( QLabel::AlignBottom | QLabel::AlignLeft );

  lbUnSelected = new QListBox( this );
  lbUnSelected->setSelectionMode( QListBox::Extended );
  lbUnSelected->setMinimumHeight( 100 );
  gl->addWidget( lbUnSelected, 1, 0 );
  lbSelected = new QListBox( this );
  lbSelected->setSelectionMode( QListBox::Extended );
  lSelected->setBuddy( lbSelected );
  gl->addWidget( lbSelected, 1, 2 );

  QBoxLayout *vb1 = new QBoxLayout( QBoxLayout::TopToBottom, spacing );
  vb1->addStretch();
  //pbAdd = new QPushButton( i18n( "&Add >>" ), this );
  pbAdd = new QToolButton( this );
  pbAdd->setIconSet( SmallIconSet( "forward" ) );
  QObject::connect( pbAdd, SIGNAL( clicked() ), this, SLOT( select() ));
  vb1->addWidget( pbAdd );
  //pbRemove = new QPushButton( i18n( "<< &Remove" ), this );
  pbRemove = new QToolButton( this );
  pbRemove->setIconSet( SmallIconSet( "back" ) );
  QObject::connect( pbRemove, SIGNAL( clicked() ), this, SLOT( unselect() ));
  vb1->addWidget( pbRemove );
  vb1->addStretch();
  gl->addLayout( vb1, 1, 1 );

  // Buttons to set the order of selected fields
  QBoxLayout *vb2 = new QBoxLayout( QBoxLayout::TopToBottom, spacing );
  vb2->addStretch();
  pbUp = new QToolButton( this );
  pbUp->setIconSet( SmallIconSet( "up" ) );
  connect( pbUp, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
  vb2->addWidget( pbUp );
  pbDown = new QToolButton( this );
  pbDown->setIconSet( SmallIconSet( "down" ) );
  connect( pbDown, SIGNAL( clicked() ), this, SLOT( moveDown() ) );
  vb2->addWidget( pbDown );
  vb2->addStretch();
  gl->addLayout( vb2, 1, 3 );

  QBoxLayout *hb1 = new QBoxLayout( QBoxLayout::LeftToRight, spacing );
  QLabel *lCustomField = new QLabel( i18n( "&Custom field:" ), this );
  hb1->addWidget( lCustomField );
  leCustomField = new QLineEdit( this );
  lCustomField->setBuddy( leCustomField );
  hb1->addWidget( leCustomField );
  QObject::connect( leCustomField, SIGNAL( returnPressed() ),
		    this, SLOT( addCustom() ));
  QObject::connect( leCustomField, SIGNAL(textChanged ( const QString & )),
                    this, SLOT( textChanged(const QString &)));

  pbAddCustom = new QPushButton( i18n( "A&dd" ), this );
  QObject::connect( pbAddCustom, SIGNAL( clicked() ), this, SLOT( addCustom() ));
  hb1->addWidget( pbAddCustom );

  gl->addMultiCell( hb1, 2, 2, 0, 2, QGridLayout::AlignRight );

  // was here
  
  QSize lbSizeHint = lbUnSelected->sizeHint();
  lbSizeHint = lbSizeHint.expandedTo( lbSelected->sizeHint() );
  lbUnSelected->setMinimumSize( lbSizeHint );
  lbSelected->setMinimumSize( lbSizeHint );

  QObject::connect( cbUnselected, SIGNAL( activated(int) ),
		    this, SLOT( showFields(int) ));
  pbAddCustom->setEnabled(false);

  setButtonsEnabled();
  connect( lbUnSelected, SIGNAL( selectionChanged() ), this, SLOT( setButtonsEnabled() ) );
  connect( lbSelected, SIGNAL( selectionChanged() ), this, SLOT( setButtonsEnabled() ) );
  connect( lbSelected, SIGNAL( currentChanged( QListBoxItem * ) ),
                this, SLOT( setButtonsEnabled( QListBoxItem * ) ) );
  gl->activate();
}

void SelectFieldsWidget::textChanged(const QString &_text)
{
    pbAddCustom->setEnabled(!_text.isEmpty());
}

void SelectFieldsWidget::showFields( int index )
{
  lbUnSelected->clear();

  int category;
  if ( index == 0 ) category = KABC::Field::All;
  else category = 1 << ( index - 1 );

  KABC::Field::List allFields = mDoc->fields( category );

  KABC::Field::List::ConstIterator itAll;
  for( itAll = allFields.begin(); itAll != allFields.end(); ++itAll ) {
    QListBoxItem *item = lbSelected->firstItem();
    while( item ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( item );
      if ( (*itAll)->equals( fieldItem->field()) ) break;
      item = item->next();
    }
    if ( !item ) {
      new FieldItem( lbUnSelected, *itAll );
    }
  }
}

void SelectFieldsWidget::select()
{
  // insert selected items in the unselected list to the selected list,
  // directoy under the current item if selected, or at the bottonm if
  // nothing is selected in the selected list
  int where = lbSelected->currentItem();
  if ( where > -1 && lbSelected->item( where )->isSelected() )
    where++;
  else
    where = lbSelected->count();

  for(uint i = 0; i < lbUnSelected->count(); ++i)
    if (lbUnSelected->isSelected( lbUnSelected->item( i ))) {
      FieldItem *fieldItem = static_cast<FieldItem *>( lbUnSelected->item( i ) );
      new FieldItem( lbSelected, fieldItem->field(), where );
      lbUnSelected->removeItem( i );
      where++;
      --i;
    }
}

void SelectFieldsWidget::unselect()
{
  for(uint i = 0; i < lbSelected->count(); ++i)
    if (lbSelected->isSelected( lbSelected->item( i ))) {
      FieldItem *fieldItem = static_cast<FieldItem *>( lbSelected->item( i ) );
      QString item = lbSelected->item( i )->text();
      QString lItem = item.lower();
      uint j = 0;
      for(j = 0; j < lbUnSelected->count(); ++j) {
	if (lbUnSelected->text( j ).lower() > lItem)
	  break;
      }
      new FieldItem( lbUnSelected, fieldItem->field(), j );
      lbSelected->removeItem( i );
      --i;
    }
}

KABC::Field::List SelectFieldsWidget::chosenFields()
{
  KABC::Field::List result;
  uint i;
  for(i = 0; i < lbSelected->count(); ++i) {
    FieldItem *fieldItem = static_cast<FieldItem *>( lbSelected->item( i ) );
    result.append( fieldItem->field() );
  }
  return result;
}

void SelectFieldsWidget::addCustom()
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
#ifdef TODO_add_custom_field
  lbSelected->insertItem( item, i );
#endif
  leCustomField->clear();
}

void SelectFieldsWidget::setButtonsEnabled()
{
  bool b = false;
  // add button: enabled if any items are selected in the unselected list
  for(uint i = 0; i < lbUnSelected->count(); ++i)
    if ( lbUnSelected->item( i )->isSelected() ) {
      b = true;
      break;
    }
  pbAdd->setEnabled( b );

  int j = lbSelected->currentItem();
  b = ( j > -1 && lbSelected->isSelected( j ) );
  // up button: enabled if there is a current item > 0 and that is selected
  pbUp->setEnabled( ( j > 0 && b ) );
  // down button: enabled if there is a current item < count - 2 and that is selected
  pbDown->setEnabled( ( j > -1 && j < (int)lbSelected->count()-1 && b ) );

  // remove button: enabled if any items are selected in the selected list
  b = false;
  for(uint i = 0; i < lbSelected->count(); ++i)
    if ( lbSelected->item( i )->isSelected() ) {
      b = true;
      break;
    }
  pbRemove->setEnabled( b );
}

void SelectFieldsWidget::moveUp()
{
  int i = lbSelected->currentItem();
  if ( i > 0 ) {
    QListBoxItem *item = lbSelected->item( i );
    lbSelected->takeItem( item );
    lbSelected->insertItem( item, i-1 );
    lbSelected->setCurrentItem( item );
    lbSelected->setSelected( i-1, true );
  }
}

void SelectFieldsWidget::moveDown()
{
  int i = lbSelected->currentItem();
  if ( i > -1 && i < (int)lbSelected->count() - 1 ) {
    QListBoxItem *item = lbSelected->item( i );
    lbSelected->takeItem( item );
    lbSelected->insertItem( item, i+1 );
    lbSelected->setCurrentItem( item );
    lbSelected->setSelected( i+1, true );
  }
}

#include "selectfieldswidget.moc"
