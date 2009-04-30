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

#include "customfieldswidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QDateEdit>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRegExpValidator>
#include <QtGui/QSpinBox>
#include <QtGui/QTimeEdit>
#include <QtGui/QVBoxLayout>

#include <kacceleratormanager.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "addresseeconfig.h"
#include "kabprefs.h"

AddFieldDialog::AddFieldDialog( QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Add Field" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );

  QLabel *label = new QLabel( i18nc( "@label:textbox Name of a custom field", "Name:" ), page );
  layout->addWidget( label, 0, 0 );

  mTitle = new KLineEdit( page );
  mTitle->setValidator( new QRegExpValidator( QRegExp( "[a-zA-Z\\d-]+" ), mTitle ) );
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
    mType->addItem( mTypeName[ i ] );

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
  return id.replace( ',', '_' ).replace( ' ', '_' );
}

QString AddFieldDialog::type() const
{
  return mTypeList[ mType->currentIndex() ];
}

bool AddFieldDialog::isGlobal() const
{
  return mGlobal->isChecked();
}

void AddFieldDialog::nameChanged( const QString &name )
{
  enableButton( Ok, !name.isEmpty() );
}

FieldWidget::FieldWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  mGlobalLayout = new QVBoxLayout();
  layout->addItem( mGlobalLayout );
  mGlobalLayout->setSpacing( KDialog::spacingHint() );
  mGlobalLayout->setAlignment( Qt::AlignTop );

  mSeparator = new QFrame( this );
  mSeparator->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  mSeparator->hide();
  layout->addWidget( mSeparator );

  mLocalLayout = new QVBoxLayout();
  layout->addItem( mLocalLayout );
  mLocalLayout->setSpacing( KDialog::spacingHint() );
  mLocalLayout->setAlignment( Qt::AlignTop );
}

void FieldWidget::addField( const QString &identifier, const QString &title,
                            const QString &type, bool isGlobal )
{
  FieldRecord record;

  record.mIdentifier = identifier;
  record.mTitle = title;
  record.mLabel = new QLabel( title + ':', this );
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
    QDateEdit *wdg = new QDateEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( dateChanged( const QDate& ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "time" ) {
    QTimeEdit *wdg = new QTimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( timeChanged( const QTime& ) ),
             this, SIGNAL( changed() ) );
  } else if ( type == "datetime" ) {
    QDateTimeEdit *wdg = new QDateTimeEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( dateTimeChanged( const QDateTime& ) ),
             this, SIGNAL( changed() ) );
  } else  if ( type == "text" ) {
    KLineEdit *wdg = new KLineEdit( this );
    record.mWidget = wdg;
    connect( wdg, SIGNAL( textChanged( const QString& ) ),
             this, SIGNAL( changed() ) );
  }

  record.mLabel->show();
  record.mWidget->show();

  if ( isGlobal ) {
    record.mLayout = new QHBoxLayout();
    mGlobalLayout->addLayout( record.mLayout );
    record.mLayout->addWidget( record.mLabel );
    record.mLayout->addWidget( record.mWidget, Qt::AlignLeft );
  } else {
    record.mLayout = new QHBoxLayout();
    mLocalLayout->addLayout( record.mLayout );
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

      mFieldList.erase( it );
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
  for ( fieldIt = mFieldList.constBegin(); fieldIt != mFieldList.constEnd(); ++fieldIt ) {
    if ( qobject_cast<KLineEdit*>( (*fieldIt).mWidget ) ) {
      KLineEdit *wdg = static_cast<KLineEdit*>( (*fieldIt).mWidget );
      wdg->setText( QString() );
    } else if ( qobject_cast<QSpinBox*>( (*fieldIt).mWidget ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( (*fieldIt).mWidget );
      wdg->setValue( 0 );
    } else if ( qobject_cast<QCheckBox*>( (*fieldIt).mWidget ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( (*fieldIt).mWidget );
      wdg->setChecked( true );
    } else if ( qobject_cast<QDateEdit*>( (*fieldIt).mWidget ) ) {
      QDateEdit *wdg = static_cast<QDateEdit*>( (*fieldIt).mWidget );
      wdg->setDate( QDate::currentDate() );
    } else if ( qobject_cast<QTimeEdit*>( (*fieldIt).mWidget ) ) {
      QTimeEdit *wdg = static_cast<QTimeEdit*>( (*fieldIt).mWidget );
      wdg->setTime( QTime::currentTime() );
    } else if ( qobject_cast<QDateTimeEdit*>( (*fieldIt).mWidget ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( (*fieldIt).mWidget );
      wdg->setDateTime( QDateTime::currentDateTime() );
    }
  }
}

void FieldWidget::loadContact( KABC::Addressee *addr )
{
  const QStringList customs = addr->customs();

  clearFields();

  QStringList::ConstIterator it;
  for ( it = customs.constBegin(); it != customs.constEnd(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );
    if ( app != "KADDRESSBOOK" )
      continue;

    FieldRecordList::ConstIterator fieldIt;
    for ( fieldIt = mFieldList.constBegin(); fieldIt != mFieldList.constEnd(); ++fieldIt ) {
      if ( (*fieldIt).mIdentifier == name ) {
        if ( qobject_cast<KLineEdit*>( (*fieldIt).mWidget ) ) {
          KLineEdit *wdg = static_cast<KLineEdit*>( (*fieldIt).mWidget );
          wdg->setText( value );
        } else if ( qobject_cast<QSpinBox*>( (*fieldIt).mWidget ) ) {
          QSpinBox *wdg = static_cast<QSpinBox*>( (*fieldIt).mWidget );
          wdg->setValue( value.toInt() );
        } else if ( qobject_cast<QCheckBox*>( (*fieldIt).mWidget ) ) {
          QCheckBox *wdg = static_cast<QCheckBox*>( (*fieldIt).mWidget );
          wdg->setChecked( value == "true" || value == "1" );
        } else if ( qobject_cast<QDateEdit*>( (*fieldIt).mWidget ) ) {
          QDateEdit *wdg = static_cast<QDateEdit*>( (*fieldIt).mWidget );
          wdg->setDate( QDate::fromString( value, Qt::ISODate ) );
        } else if ( qobject_cast<QTimeEdit*>( (*fieldIt).mWidget ) ) {
          QTimeEdit *wdg = static_cast<QTimeEdit*>( (*fieldIt).mWidget );
          wdg->setTime( QTime::fromString( value, Qt::ISODate ) );
        } else if ( qobject_cast<QDateTimeEdit*>( (*fieldIt).mWidget ) ) {
          QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( (*fieldIt).mWidget );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        }
      }
    }
  }
}

void FieldWidget::setReadOnly( bool readOnly )
{
  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.begin(); it != mFieldList.end(); ++it ) {
    QString value;
    if ( QLineEdit *wdg = qobject_cast<QLineEdit*>( (*it).mWidget ) ) {
      wdg->setReadOnly( readOnly );
    } else if ( QSpinBox *wdg = qobject_cast<QSpinBox*>( (*it).mWidget ) ) {
      wdg->setEnabled( !readOnly );
    } else if ( QCheckBox *wdg = qobject_cast<QCheckBox*>( (*it).mWidget ) ) {
      wdg->setEnabled( !readOnly );
    } else if ( QDateEdit *wdg = qobject_cast<QDateEdit*>( (*it).mWidget ) ) {
      wdg->setEnabled( !readOnly );
    } else if ( QTimeEdit *wdg = qobject_cast<QTimeEdit*>( (*it).mWidget ) ) {
      wdg->setEnabled( !readOnly );
    } else if ( QDateTimeEdit *wdg = qobject_cast<QDateTimeEdit*>( (*it).mWidget ) ) {
      wdg->setEnabled( !readOnly );
    }
  }
}

void FieldWidget::storeContact( KABC::Addressee *addr )
{
  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.constBegin(); it != mFieldList.constEnd(); ++it ) {
    QString value;
    if ( qobject_cast<KLineEdit*>( (*it).mWidget ) ) {
      KLineEdit *wdg = static_cast<KLineEdit*>( (*it).mWidget );
      value = wdg->text();
    } else if ( qobject_cast<QSpinBox*>( (*it).mWidget ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( (*it).mWidget );
      value = QString::number( wdg->value() );
    } else if ( qobject_cast<QCheckBox*>( (*it).mWidget ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( (*it).mWidget );
      value = ( wdg->isChecked() ? "true" : "false" );
    } else if ( qobject_cast<QDateEdit*>( (*it).mWidget ) ) {
      QDateEdit *wdg = static_cast<QDateEdit*>( (*it).mWidget );
      value = wdg->date().toString( Qt::ISODate );
    } else if ( qobject_cast<QTimeEdit*>( (*it).mWidget )  ) {
      QTimeEdit *wdg = static_cast<QTimeEdit*>( (*it).mWidget );
      value = wdg->time().toString( Qt::ISODate );
    } else if ( qobject_cast<QDateTimeEdit*>( (*it).mWidget ) ) {
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

      it = mFieldList.erase( it );
      it--;
      recalculateLayout();
    }
  }
}

void FieldWidget::recalculateLayout()
{
  int maxWidth = 0;

  FieldRecordList::ConstIterator it;
  for ( it = mFieldList.constBegin(); it != mFieldList.constEnd(); ++it )
    maxWidth = qMax( maxWidth, (*it).mLabel->minimumSizeHint().width() );

  for ( it = mFieldList.constBegin(); it != mFieldList.constEnd(); ++it )
    (*it).mLabel->setMinimumWidth( maxWidth );
}

CustomFieldsWidget::CustomFieldsWidget( KABC::AddressBook *ab,
                                        QWidget *parent )
  : KAB::ContactEditorWidget( ab, parent )
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
  mFieldWidget->setReadOnly( readOnly );
}

void CustomFieldsWidget::addField()
{
  AddFieldDialog dlg( this );

  if ( dlg.exec() ) {
    FieldRecordList list = mFieldWidget->fields();

    FieldRecordList::ConstIterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it )
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
  for ( it = list.constBegin(); it != list.constEnd(); ++it )
    fields.append( (*it).mTitle );

  bool ok;
  QString title = KInputDialog::getItem( i18n( "Remove Field" ),
                                         i18n( "Select the field you want to remove:" ),
                                         fields, 0, false, &ok, this );

  if ( ok ) {
    for ( it = list.constBegin(); it != list.constEnd(); ++it )
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
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  mFieldWidget = new FieldWidget( this );
  layout->addWidget( mFieldWidget );
  layout->addStretch();

  QWidget *buttonGroup = new QWidget( this );
  QHBoxLayout *buttonGroupLayout = new QHBoxLayout;
  buttonGroupLayout->setMargin( 0 );
  buttonGroupLayout->setSpacing( KDialog::spacingHint() );

  mAddButton = new QPushButton( i18n( "Add Field..." ), buttonGroup );
  buttonGroupLayout->addStretch();
  buttonGroupLayout->addWidget( mAddButton );

  mRemoveButton = new QPushButton( i18n( "Remove Field..." ), buttonGroup );
  mRemoveButton->setEnabled( false );
  buttonGroupLayout->addWidget( mRemoveButton );

  buttonGroup->setLayout( buttonGroupLayout );
  layout->addWidget( buttonGroup );

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
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it).mGlobal == global ) {
      retval.append( (*it).mIdentifier );
      retval.append( (*it).mTitle );

      QString type = "text";
      if ( qobject_cast<QSpinBox*>( (*it).mWidget ) )  {
        type = "integer";
      } else if ( qobject_cast<QCheckBox*>( (*it).mWidget ) ) {
        type = "boolean";
      } else if ( qobject_cast<QDateEdit*>( (*it).mWidget ) ) {
        type = "date";
      } else if ( qobject_cast<QTimeEdit*>( (*it).mWidget ) ) {
        type = "time";
      } else if ( qobject_cast<QDateTimeEdit*>( (*it).mWidget ) ) {
        type = "datetime";
      } else if ( qobject_cast<KLineEdit*>( (*it).mWidget ) ) {
        type = "text";
      }

      retval.append( type );
    }
  }

  return retval;
}


void splitField( const QString &str, QString &app, QString &name, QString &value )
{
  int colon = str.indexOf( ':' );
  if ( colon != -1 ) {
    QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.indexOf( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}

#include "customfieldswidget.moc"
