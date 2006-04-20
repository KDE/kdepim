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

#include <qcheckbox.h>
#include <q3datetimeedit.h>
#include <q3frame.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kacceleratormanager.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "addresseeconfig.h"
#include "kabprefs.h"

#include "customfieldswidget.h"


AddFieldDialog::AddFieldDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Add Field" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  QWidget *page = plainPage();

  QGridLayout *layout = new QGridLayout( page, 3, 2, marginHint(), spacingHint() );

  QLabel *label = new QLabel( i18n( "Title:" ), page );
  layout->addWidget( label, 0, 0 );

  mTitle = new KLineEdit( page );
  label->setBuddy( mTitle );
  layout->addWidget( mTitle, 0, 1 );

  label = new QLabel( i18n( "Type:" ), page );
  layout->addWidget( label, 1, 0 );

  mType = new KComboBox( page );
  label->setBuddy( mType );
  layout->addWidget( mType, 1, 1 );

  mGlobal = new QCheckBox( i18n( "Is available for all contacts" ), page );
  mGlobal->setChecked( true );
  layout->addWidget( mGlobal, 2, 0, 1, 2 );

  connect( mTitle, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( nameChanged( const QString& ) ) );

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

  for ( int i = 0; i < mTypeName.count(); ++i )
    mType->insertItem( mTypeName[ i ] );

  nameChanged( "" );

  mTitle->setFocus();
}

QString AddFieldDialog::title() const
{
  return mTitle->text();
}

QString AddFieldDialog::identifier() const
{
  QString id = mTitle->text().toLower();
  return id.replace( ",", "_" ).replace( " ", "_" );
}

QString AddFieldDialog::type() const
{
  return mTypeList[ mType->currentItem() ];
}

bool AddFieldDialog::isGlobal() const
{
  return mGlobal->isChecked();
}

void AddFieldDialog::nameChanged( const QString &name )
{
  enableButton( Ok, !name.isEmpty() );
}

FieldWidget::FieldWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mGlobalLayout = new QVBoxLayout( layout, KDialog::spacingHint() );
  mGlobalLayout->setAlignment( Qt::AlignTop );

  mSeparator = new QFrame( this );
  mSeparator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  mSeparator->hide();
  layout->addWidget( mSeparator );

  mLocalLayout = new QVBoxLayout( layout, KDialog::spacingHint() );
  mLocalLayout->setAlignment( Qt::AlignTop );
}

void FieldWidget::addField( const QString &identifier, const QString &title,
                            const QString &type, bool isGlobal )
{
  FieldRecord record;

  record.mIdentifier = identifier;
  record.mTitle = title;
  record.mLabel = new QLabel( title + ":", this );
  record.mGlobal = isGlobal;
  if ( type == "integer" ) {
    QSpinBox *wdg = new QSpinBox( this );
    wdg->setRange( 0, 1000 );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( valueChanged( int ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "boolean" ) {
    QCheckBox *wdg = new QCheckBox( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( toggled( bool ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "date" ) {
    Q3DateEdit *wdg = new Q3DateEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( valueChanged( const QDate& ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "time" ) {
    Q3TimeEdit *wdg = new Q3TimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( valueChanged( const QTime& ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "datetime" ) {
    Q3DateTimeEdit *wdg = new Q3DateTimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( valueChanged( const QDateTime& ) ),
             this, SIGNAL( changed() ) );
  } else  if ( type == "text" ) {
    QLineEdit *wdg = new QLineEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( textChanged( const QString& ) ),
             this, SIGNAL( changed() ) );
  }

  record.mLabel->show();
  record.mWidget->show();

  if ( isGlobal ) {
    record.mLayout = new QHBoxLayout( mGlobalLayout );
    record.mLayout->addWidget( record.mLabel );
    record.mLayout->addWidget( record.mWidget, Qt::AlignLeft );
  } else {
    record.mLayout = new QHBoxLayout( mLocalLayout );
    record.mLayout->addWidget( record.mLabel );
    record.mLayout->addWidget( record.mWidget, Qt::AlignLeft );
    mSeparator->show();
  }

  mFieldList.append( record );

  recalculateLayout();
}

void FieldWidget::removeField( const QString &identifier )
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
    if ( (*fieldIt).mWidget->isA( "QLineEdit" ) ) {
      QLineEdit *wdg = static_cast<QLineEdit*>( (*fieldIt).mWidget );
      wdg->setText( QString() );
    } else if ( (*fieldIt).mWidget->isA( "QSpinBox" ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( (*fieldIt).mWidget );
      wdg->setValue( 0 );
    } else if ( (*fieldIt).mWidget->isA( "QCheckBox" ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( (*fieldIt).mWidget );
      wdg->setChecked( true );
    } else if ( (*fieldIt).mWidget->isA( "QDateEdit" ) ) {
      Q3DateEdit *wdg = static_cast<Q3DateEdit*>( (*fieldIt).mWidget );
      wdg->setDate( QDate::currentDate() );
    } else if ( (*fieldIt).mWidget->isA( "QTimeEdit" ) ) {
      Q3TimeEdit *wdg = static_cast<Q3TimeEdit*>( (*fieldIt).mWidget );
      wdg->setTime( QTime::currentTime() );
    } else if ( (*fieldIt).mWidget->isA( "QDateTimeEdit" ) ) {
      Q3DateTimeEdit *wdg = static_cast<Q3DateTimeEdit*>( (*fieldIt).mWidget );
      wdg->setDateTime( QDateTime::currentDateTime() );
    }
  }
}

void FieldWidget::loadContact( KABC::Addressee *addr )
{
  const QStringList customs = addr->customs();

  clearFields();

  QStringList::ConstIterator it;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );
    if ( app != "KADDRESSBOOK" )
      continue;

    FieldRecordList::ConstIterator fieldIt;
    for ( fieldIt = mFieldList.begin(); fieldIt != mFieldList.end(); ++fieldIt ) {
      if ( (*fieldIt).mIdentifier == name ) {
        if ( (*fieldIt).mWidget->isA( "QLineEdit" ) ) {
          QLineEdit *wdg = static_cast<QLineEdit*>( (*fieldIt).mWidget );
          wdg->setText( value );
        } else if ( (*fieldIt).mWidget->isA( "QSpinBox" ) ) {
          QSpinBox *wdg = static_cast<QSpinBox*>( (*fieldIt).mWidget );
          wdg->setValue( value.toInt() );
        } else if ( (*fieldIt).mWidget->isA( "QCheckBox" ) ) {
          QCheckBox *wdg = static_cast<QCheckBox*>( (*fieldIt).mWidget );
          wdg->setChecked( value == "true" || value == "1" );
        } else if ( (*fieldIt).mWidget->isA( "QDateEdit" ) ) {
          Q3DateEdit *wdg = static_cast<Q3DateEdit*>( (*fieldIt).mWidget );
          wdg->setDate( QDate::fromString( value, Qt::ISODate ) );
        } else if ( (*fieldIt).mWidget->isA( "QTimeEdit" ) ) {
          Q3TimeEdit *wdg = static_cast<Q3TimeEdit*>( (*fieldIt).mWidget );
          wdg->setTime( QTime::fromString( value, Qt::ISODate ) );
        } else if ( (*fieldIt).mWidget->isA( "QDateTimeEdit" ) ) {
          Q3DateTimeEdit *wdg = static_cast<Q3DateTimeEdit*>( (*fieldIt).mWidget );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        }
      }
    }
  }
}

void FieldWidget::storeContact( KABC::Addressee *addr )
{
  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    QString value;
    if ( (*it).mWidget->isA( "QLineEdit" ) ) {
      QLineEdit *wdg = static_cast<QLineEdit*>( (*it).mWidget );
      value = wdg->text();
    } else if ( (*it).mWidget->isA( "QSpinBox" ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( (*it).mWidget );
      value = QString::number( wdg->value() );
    } else if ( (*it).mWidget->isA( "QCheckBox" ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( (*it).mWidget );
      value = ( wdg->isChecked() ? "true" : "false" );
    } else if ( (*it).mWidget->isA( "QDateEdit" ) ) {
      Q3DateEdit *wdg = static_cast<Q3DateEdit*>( (*it).mWidget );
      value = wdg->date().toString( Qt::ISODate );
    } else if ( (*it).mWidget->isA( "QTimeEdit" ) ) {
      Q3TimeEdit *wdg = static_cast<Q3TimeEdit*>( (*it).mWidget );
      value = wdg->time().toString( Qt::ISODate );
    } else if ( (*it).mWidget->isA( "QDateTimeEdit" ) ) {
      Q3DateTimeEdit *wdg = static_cast<Q3DateTimeEdit*>( (*it).mWidget );
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
    maxWidth = qMax( maxWidth, (*it).mLabel->minimumSizeHint().width() );

  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it )
    (*it).mLabel->setMinimumWidth( maxWidth );
}

CustomFieldsWidget::CustomFieldsWidget( KABC::AddressBook *ab,
                                        QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  initGUI();

  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addField() ) );
  connect( mRemoveButton, SIGNAL( clicked() ), this, SLOT( removeField() ) );

  connect( mFieldWidget, SIGNAL( changed() ), this, SLOT( setModified() ) );
}

void CustomFieldsWidget::loadContact( KABC::Addressee *addr )
{
  mAddressee = *addr;

  mFieldWidget->removeLocalFields();

  AddresseeConfig addrConfig( mAddressee );
  QStringList fields = addrConfig.customFields();

  if ( !fields.isEmpty() ) {
    for ( int i = 0; i < fields.count(); i += 3 ) {
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

  QStringList fields;

  FieldRecordList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    fields.append( (*it).mTitle );

  bool ok;
  QString title = KInputDialog::getItem( i18n( "Remove Field" ),
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
  QGridLayout *layout = new QGridLayout( this, 2, 3, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mFieldWidget = new FieldWidget( this );
  layout->addWidget( mFieldWidget, 0, 0, 1, 3 );

  mAddButton = new QPushButton( i18n( "Add Field..." ), this );
  layout->addWidget( mAddButton, 1, 1, Qt::AlignRight );

  mRemoveButton = new QPushButton( i18n( "Remove Field..." ), this );
  mRemoveButton->setEnabled( false );
  layout->addWidget( mRemoveButton, 1, 2, Qt::AlignRight );

  // load global fields
  QStringList globalFields = KABPrefs::instance()->globalCustomFields();

  if ( globalFields.isEmpty() )
    return;

  for ( int i = 0; i < globalFields.count(); i += 3 ) {
    mFieldWidget->addField( globalFields[ i ], globalFields[ i + 1 ],
                            globalFields[ i + 2 ] , true );
    mRemoveButton->setEnabled( true );
  }
}

QStringList CustomFieldsWidget::marshallFields( bool global ) const
{
  QStringList retval;

  const FieldRecordList list = mFieldWidget->fields();
  FieldRecordList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it).mGlobal == global ) {
      retval.append( (*it).mIdentifier );
      retval.append( (*it).mTitle );

      QString type = "text";
      if ( (*it).mWidget->isA( "QSpinBox" ) ) {
        type = "integer";
      } else if ( (*it).mWidget->isA( "QCheckBox" ) ) {
        type = "boolean";
      } else if ( (*it).mWidget->isA( "QDateEdit" ) ) {
        type = "date";
      } else if ( (*it).mWidget->isA( "QTimeEdit" ) ) {
        type = "time";
      } else if ( (*it).mWidget->isA( "QDateTimeEdit" ) ) {
        type = "datetime";
      } else if ( (*it).mWidget->isA( "QLineEdit" ) ) {
        type = "text";
      }

      retval.append( type );
    }
  }

  return retval;
}


void splitField( const QString &str, QString &app, QString &name, QString &value )
{
  int colon = str.find( ':' );
  if ( colon != -1 ) {
    QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.find( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}

#include "customfieldswidget.moc"
