/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqcheckbox.h>
#include <tqdatetimeedit.h>
#include <tqframe.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqvalidator.h>
#include <tqspinbox.h>

#include <kaccelmanager.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "addresseeconfig.h"
#include "kabprefs.h"

#include "customfieldswidget.h"


AddFieldDialog::AddFieldDialog( TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Add Field" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  TQWidget *page = plainPage();

  TQGridLayout *layout = new TQGridLayout( page, 3, 2, marginHint(), spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Title:" ), page );
  layout->addWidget( label, 0, 0 );

  mTitle = new KLineEdit( page );
  mTitle->setValidator( new TQRegExpValidator( TQRegExp( "([a-zA-Z]|\\d|-)+" ), mTitle ) );
  label->setBuddy( mTitle );
  layout->addWidget( mTitle, 0, 1 );

  label = new TQLabel( i18n( "Type:" ), page );
  layout->addWidget( label, 1, 0 );

  mType = new KComboBox( page );
  label->setBuddy( mType );
  layout->addWidget( mType, 1, 1 );

  mGlobal = new TQCheckBox( i18n( "Is available for all contacts" ), page );
  mGlobal->setChecked( true );
  layout->addMultiCellWidget( mGlobal, 2, 2, 0, 1 );

  connect( mTitle, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( nameChanged( const TQString& ) ) );

  KAcceleratorManager::manage( this );

  mTypeList.append( "text" );
  mTypeName.append( i18n( "Text" ) );
  mTypeList.append( "integer" );
  mTypeName.append( i18n( "Numeric Value" ) );
  mTypeList.append( "boolean" );
  mTypeName.append( i18n( "Boolean" ) );
  mTypeList.append( "date" );
  mTypeName.append( i18n( "Date" ) );
  mTypeList.append( "time" );
  mTypeName.append( i18n( "Time" ) );
  mTypeList.append( "datetime" );
  mTypeName.append( i18n( "Date & Time" ) );

  for ( uint i = 0; i < mTypeName.count(); ++i )
    mType->insertItem( mTypeName[ i ] );

  nameChanged( "" );

  mTitle->setFocus();
}

TQString AddFieldDialog::title() const
{
  return mTitle->text();
}

TQString AddFieldDialog::identifier() const
{
  TQString id = mTitle->text().lower();
  return id.replace( ",", "_" ).replace( " ", "_" );
}

TQString AddFieldDialog::type() const
{
  return mTypeList[ mType->currentItem() ];
}

bool AddFieldDialog::isGlobal() const
{
  return mGlobal->isChecked();
}

void AddFieldDialog::nameChanged( const TQString &name )
{
  enableButton( Ok, !name.isEmpty() );
}

FieldWidget::FieldWidget( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  TQVBoxLayout *layout = new TQVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mGlobalLayout = new TQVBoxLayout( layout, KDialog::spacingHint() );
  mGlobalLayout->setAlignment( Qt::AlignTop );

  mSeparator = new TQFrame( this );
  mSeparator->setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
  mSeparator->hide();
  layout->addWidget( mSeparator );

  mLocalLayout = new TQVBoxLayout( layout, KDialog::spacingHint() );
  mLocalLayout->setAlignment( Qt::AlignTop );
}

void FieldWidget::addField( const TQString &identifier, const TQString &title,
                            const TQString &type, bool isGlobal )
{
  FieldRecord record;

  record.mIdentifier = identifier;
  record.mTitle = title;
  record.mLabel = new TQLabel( title + ":", this );
  record.mGlobal = isGlobal;
  if ( type == "integer" ) {
    TQSpinBox *wdg = new TQSpinBox( 0, 1000, 1, this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( valueChanged( int ) ),
             this, TQT_SIGNAL( changed() ) );
  } else if ( type == "boolean" ) {
    TQCheckBox *wdg = new TQCheckBox( this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( toggled( bool ) ),
             this, TQT_SIGNAL( changed() ) );
  } else if ( type == "date" ) {
    QDateEdit *wdg = new QDateEdit( this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( valueChanged( const TQDate& ) ),
             this, TQT_SIGNAL( changed() ) );
  } else if ( type == "time" ) {
    QTimeEdit *wdg = new QTimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( valueChanged( const TQTime& ) ),
             this, TQT_SIGNAL( changed() ) );
  } else if ( type == "datetime" ) {
    QDateTimeEdit *wdg = new QDateTimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( valueChanged( const TQDateTime& ) ),
             this, TQT_SIGNAL( changed() ) );
  } else  if ( type == "text" ) {
    TQLineEdit *wdg = new TQLineEdit( this );
    record.mWidget = wdg;
    connect( wdg, TQT_SIGNAL( textChanged( const TQString& ) ),
             this, TQT_SIGNAL( changed() ) );
  }

  record.mLabel->show();
  record.mWidget->show();

  if ( isGlobal ) {
    record.mLayout = new TQHBoxLayout( mGlobalLayout );
    record.mLayout->addWidget( record.mLabel );
    record.mLayout->addWidget( record.mWidget, Qt::AlignLeft );
  } else {
    record.mLayout = new TQHBoxLayout( mLocalLayout );
    record.mLayout->addWidget( record.mLabel );
    record.mLayout->addWidget( record.mWidget, Qt::AlignLeft );
    mSeparator->show();
  }

  mFieldList.append( record );

  recalculateLayout();
}

void FieldWidget::removeField( const TQString &identifier )
{
  FieldRecordList::Iterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    if ( (*it).mIdentifier == identifier ) {
      delete (*it).mLabel;
      delete (*it).mWidget;
      delete (*it).mLayout;

      mFieldList.remove( it );
      recalculateLayout();

      bool hasLocal = false;
      for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
        hasLocal = hasLocal || !(*it).mGlobal;

      if ( !hasLocal )
        mSeparator->hide();

      return;
    }
  }
}

void FieldWidget::clearFields()
{
  FieldRecordList::ConstIterator fieldIt;
  for ( fieldIt = mFieldList.begin(); fieldIt != mFieldList.end(); ++fieldIt ) {
    if ( (*fieldIt).mWidget->isA( "TQLineEdit" ) ) {
      TQLineEdit *wdg = static_cast<TQLineEdit*>( (*fieldIt).mWidget );
      wdg->setText( TQString() );
    } else if ( (*fieldIt).mWidget->isA( "TQSpinBox" ) ) {
      TQSpinBox *wdg = static_cast<TQSpinBox*>( (*fieldIt).mWidget );
      wdg->setValue( 0 );
    } else if ( (*fieldIt).mWidget->isA( "TQCheckBox" ) ) {
      TQCheckBox *wdg = static_cast<TQCheckBox*>( (*fieldIt).mWidget );
      wdg->setChecked( true );
    } else if ( (*fieldIt).mWidget->isA( "QDateEdit" ) ) {
      QDateEdit *wdg = static_cast<QDateEdit*>( (*fieldIt).mWidget );
      wdg->setDate( TQDate::currentDate() );
    } else if ( (*fieldIt).mWidget->isA( "QTimeEdit" ) ) {
      QTimeEdit *wdg = static_cast<QTimeEdit*>( (*fieldIt).mWidget );
      wdg->setTime( TQTime::currentTime() );
    } else if ( (*fieldIt).mWidget->isA( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( (*fieldIt).mWidget );
      wdg->setDateTime( TQDateTime::currentDateTime() );
    }
  }
}

void FieldWidget::loadContact( KABC::Addressee *addr )
{
  const TQStringList customs = addr->customs();

  clearFields();

  TQStringList::ConstIterator it;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    TQString app, name, value;
    splitField( *it, app, name, value );
    if ( app != "KADDRESSBOOK" )
      continue;

    FieldRecordList::ConstIterator fieldIt;
    for ( fieldIt = mFieldList.begin(); fieldIt != mFieldList.end(); ++fieldIt ) {
      if ( (*fieldIt).mIdentifier == name ) {
        if ( (*fieldIt).mWidget->isA( "TQLineEdit" ) ) {
          TQLineEdit *wdg = static_cast<TQLineEdit*>( (*fieldIt).mWidget );
          wdg->setText( value );
        } else if ( (*fieldIt).mWidget->isA( "TQSpinBox" ) ) {
          TQSpinBox *wdg = static_cast<TQSpinBox*>( (*fieldIt).mWidget );
          wdg->setValue( value.toInt() );
        } else if ( (*fieldIt).mWidget->isA( "TQCheckBox" ) ) {
          TQCheckBox *wdg = static_cast<TQCheckBox*>( (*fieldIt).mWidget );
          wdg->setChecked( value == "true" || value == "1" );
        } else if ( (*fieldIt).mWidget->isA( "QDateEdit" ) ) {
          QDateEdit *wdg = static_cast<QDateEdit*>( (*fieldIt).mWidget );
          wdg->setDate( TQDate::fromString( value, Qt::ISODate ) );
        } else if ( (*fieldIt).mWidget->isA( "QTimeEdit" ) ) {
          QTimeEdit *wdg = static_cast<QTimeEdit*>( (*fieldIt).mWidget );
          wdg->setTime( TQTime::fromString( value, Qt::ISODate ) );
        } else if ( (*fieldIt).mWidget->isA( "QDateTimeEdit" ) ) {
          QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( (*fieldIt).mWidget );
          wdg->setDateTime( TQDateTime::fromString( value, Qt::ISODate ) );
        }
      }
    }
  }
}

void FieldWidget::setReadOnly( bool readOnly )
{
  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    TQString value;
    if ( (*it).mWidget->isA( "TQLineEdit" ) ) {
      TQLineEdit *wdg = static_cast<TQLineEdit*>( (*it).mWidget );
      wdg->setReadOnly(readOnly);
    } else if ( (*it).mWidget->isA( "TQSpinBox" ) ) {
      TQSpinBox *wdg = static_cast<TQSpinBox*>( (*it).mWidget );
      wdg->setEnabled( !readOnly );
    } else if ( (*it).mWidget->isA( "TQCheckBox" ) ) {
      TQCheckBox *wdg = static_cast<TQCheckBox*>( (*it).mWidget );
      wdg->setEnabled( !readOnly );
    } else if ( (*it).mWidget->isA( "TQDateEdit" ) ) {
      TQDateEdit *wdg = static_cast<TQDateEdit*>( (*it).mWidget );
      wdg->setEnabled( !readOnly );
    } else if ( (*it).mWidget->isA( "TQTimeEdit" ) ) {
      TQTimeEdit *wdg = static_cast<TQTimeEdit*>( (*it).mWidget );
      wdg->setEnabled( !readOnly );
    } else if ( (*it).mWidget->isA( "TQDateTimeEdit" ) ) {
      TQDateTimeEdit *wdg = static_cast<TQDateTimeEdit*>( (*it).mWidget );
      wdg->setEnabled( !readOnly );
    }
  }
}

void FieldWidget::storeContact( KABC::Addressee *addr )
{
  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    TQString value;
    if ( (*it).mWidget->isA( "TQLineEdit" ) ) {
      TQLineEdit *wdg = static_cast<TQLineEdit*>( (*it).mWidget );
      value = wdg->text();
    } else if ( (*it).mWidget->isA( "TQSpinBox" ) ) {
      TQSpinBox *wdg = static_cast<TQSpinBox*>( (*it).mWidget );
      value = TQString::number( wdg->value() );
    } else if ( (*it).mWidget->isA( "TQCheckBox" ) ) {
      TQCheckBox *wdg = static_cast<TQCheckBox*>( (*it).mWidget );
      value = ( wdg->isChecked() ? "true" : "false" );
    } else if ( (*it).mWidget->isA( "QDateEdit" ) ) {
      QDateEdit *wdg = static_cast<QDateEdit*>( (*it).mWidget );
      value = wdg->date().toString( Qt::ISODate );
    } else if ( (*it).mWidget->isA( "QTimeEdit" ) ) {
      QTimeEdit *wdg = static_cast<QTimeEdit*>( (*it).mWidget );
      value = wdg->time().toString( Qt::ISODate );
    } else if ( (*it).mWidget->isA( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( (*it).mWidget );
      value = wdg->dateTime().toString( Qt::ISODate );
    }

    if ( value.isEmpty() )
      addr->removeCustom( "KADDRESSBOOK", (*it).mIdentifier );
    else
      addr->insertCustom( "KADDRESSBOOK", (*it).mIdentifier, value );
  }
}

void FieldWidget::removeLocalFields()
{
  FieldRecordList::Iterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    if ( !(*it).mGlobal ) {
      delete (*it).mLabel;
      delete (*it).mWidget;
      delete (*it).mLayout;

      it = mFieldList.remove( it );
      it--;
      recalculateLayout();
    }
  }
}

void FieldWidget::recalculateLayout()
{
  int maxWidth = 0;

  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
    maxWidth = QMAX( maxWidth, (*it).mLabel->minimumSizeHint().width() );

  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
    (*it).mLabel->setMinimumWidth( maxWidth );
}

CustomFieldsWidget::CustomFieldsWidget( KABC::AddressBook *ab,
                                        TQWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  initGUI();

  connect( mAddButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( addField() ) );
  connect( mRemoveButton, TQT_SIGNAL( clicked() ), this, TQT_SLOT( removeField() ) );

  connect( mFieldWidget, TQT_SIGNAL( changed() ), this, TQT_SLOT( setModified() ) );
}

void CustomFieldsWidget::loadContact( KABC::Addressee *addr )
{
  mAddressee = *addr;

  mFieldWidget->removeLocalFields();

  AddresseeConfig addrConfig( mAddressee );
  TQStringList fields = addrConfig.customFields();

  if ( !fields.isEmpty() ) {
    for ( uint i = 0; i < fields.count(); i += 3 ) {
      mFieldWidget->addField( fields[ i ], fields[ i + 1 ],
                              fields[ i + 2 ] , false );
      mRemoveButton->setEnabled( true );
    }
  }

  mFieldWidget->loadContact( addr );
}

void CustomFieldsWidget::storeContact( KABC::Addressee *addr )
{
  mFieldWidget->storeContact( addr );
}

void CustomFieldsWidget::setReadOnly( bool readOnly )
{
  mAddButton->setEnabled( !readOnly );
  mRemoveButton->setEnabled( !readOnly && !mFieldWidget->fields().isEmpty() );
  mFieldWidget->setReadOnly( readOnly );
}

void CustomFieldsWidget::addField()
{
  AddFieldDialog dlg( this );

  if ( dlg.exec() ) {
    FieldRecordList list = mFieldWidget->fields();

    FieldRecordList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      if ( (*it).mIdentifier == dlg.identifier() ) {
        KMessageBox::sorry( this, i18n( "A field with the same name already exists, please choose another one." ) );
        return;
      }

    mFieldWidget->addField( dlg.identifier(), dlg.title(),
                            dlg.type(), dlg.isGlobal() );

    if ( dlg.isGlobal() ) {
      KABPrefs::instance()->setGlobalCustomFields( marshallFields( true ) );
    } else {
      AddresseeConfig addrConfig( mAddressee );
      addrConfig.setCustomFields( marshallFields( false ) );
    }

    mRemoveButton->setEnabled( true );
  }
}

void CustomFieldsWidget::removeField()
{
  const FieldRecordList list = mFieldWidget->fields();

  TQStringList fields;

  FieldRecordList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    fields.append( (*it).mTitle );

  bool ok;
  TQString title = KInputDialog::getItem( i18n( "Remove Field" ),
                                         i18n( "Select the field you want to remove:" ),
                                         fields, 0, false, &ok, this );

  if ( ok ) {
    for ( it = list.begin(); it != list.end(); ++it )
      if ( (*it).mTitle == title ) {
        mFieldWidget->removeField( (*it).mIdentifier );

        if ( list.count() == 1 )
          mRemoveButton->setEnabled( false );

        if ( (*it).mGlobal ) {
          KABPrefs::instance()->setGlobalCustomFields( marshallFields( true ) );
        } else {
          AddresseeConfig addrConfig( mAddressee );
          addrConfig.setCustomFields( marshallFields( false ) );
        }

        return;
      }
  }
}

void CustomFieldsWidget::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mFieldWidget = new FieldWidget( this );
  layout->addMultiCellWidget( mFieldWidget, 0, 0, 0, 2 );

  mAddButton = new TQPushButton( i18n( "Add Field..." ), this );
  layout->addWidget( mAddButton, 1, 1, Qt::AlignRight );

  mRemoveButton = new TQPushButton( i18n( "Remove Field..." ), this );
  mRemoveButton->setEnabled( false );
  layout->addWidget( mRemoveButton, 1, 2, Qt::AlignRight );

  // load global fields
  TQStringList globalFields = KABPrefs::instance()->globalCustomFields();

  if ( globalFields.isEmpty() )
    return;

  for ( uint i = 0; i < globalFields.count(); i += 3 ) {
    mFieldWidget->addField( globalFields[ i ], globalFields[ i + 1 ],
                            globalFields[ i + 2 ] , true );
    mRemoveButton->setEnabled( true );
  }
}

TQStringList CustomFieldsWidget::marshallFields( bool global ) const
{
  TQStringList retval;

  const FieldRecordList list = mFieldWidget->fields();
  FieldRecordList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it).mGlobal == global ) {
      retval.append( (*it).mIdentifier );
      retval.append( (*it).mTitle );

      TQString type = "text";
      if ( (*it).mWidget->isA( "TQSpinBox" ) ) {
        type = "integer";
      } else if ( (*it).mWidget->isA( "TQCheckBox" ) ) {
        type = "boolean";
      } else if ( (*it).mWidget->isA( "QDateEdit" ) ) {
        type = "date";
      } else if ( (*it).mWidget->isA( "QTimeEdit" ) ) {
        type = "time";
      } else if ( (*it).mWidget->isA( "QDateTimeEdit" ) ) {
        type = "datetime";
      } else if ( (*it).mWidget->isA( "TQLineEdit" ) ) {
        type = "text";
      }

      retval.append( type );
    }
  }

  return retval;
}


void splitField( const TQString &str, TQString &app, TQString &name, TQString &value )
{
  int colon = str.find( ':' );
  if ( colon != -1 ) {
    TQString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.find( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}

#include "customfieldswidget.moc"
