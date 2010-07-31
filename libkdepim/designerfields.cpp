/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdatetimeedit.h>
#include <tqlayout.h>
#include <tqobjectlist.h>
#include <tqspinbox.h>
#include <tqregexp.h>
#include <tqtextedit.h>
#include <tqwidgetfactory.h>

#include <kdatepicker.h>
#include <kdatetimewidget.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "designerfields.h"

using namespace KPIM;

DesignerFields::DesignerFields( const TQString &uiFile, TQWidget *parent,
  const char *name )
  : TQWidget( parent, name )
{
  initGUI( uiFile );
}

void DesignerFields::initGUI( const TQString &uiFile )
{
  TQVBoxLayout *layout = new TQVBoxLayout( this );

  TQWidget *wdg = TQWidgetFactory::create( uiFile, 0, this );
  if ( !wdg ) {
    kdError() << "No ui file found" << endl;
    return;
  }

  mTitle = wdg->caption();
  mIdentifier = wdg->name();

  layout->addWidget( wdg );

  TQObjectList *list = wdg->queryList( "TQWidget" );
  TQObjectListIt it( *list );

  TQStringList allowedTypes;
  allowedTypes << "TQLineEdit"
               << "TQTextEdit"
               << "TQSpinBox"
               << "TQCheckBox"
               << "TQComboBox"
               << "QDateTimeEdit"
               << "KLineEdit"
               << "KDateTimeWidget"
               << "KDatePicker";

  while ( it.current() ) {
    if ( allowedTypes.contains( it.current()->className() ) ) {
      TQString name = it.current()->name();
      if ( name.startsWith( "X_" ) ) {
        name = name.mid( 2 );

        TQWidget *widget = static_cast<TQWidget*>( it.current() );
        if ( !name.isEmpty() )
          mWidgets.insert( name, widget );

        if ( it.current()->inherits( "TQLineEdit" ) )
          connect( it.current(), TQT_SIGNAL( textChanged( const TQString& ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "TQSpinBox" ) )
          connect( it.current(), TQT_SIGNAL( valueChanged( int ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "TQCheckBox" ) )
          connect( it.current(), TQT_SIGNAL( toggled( bool ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "TQComboBox" ) )
          connect( it.current(), TQT_SIGNAL( activated( const TQString& ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "QDateTimeEdit" ) )
          connect( it.current(), TQT_SIGNAL( valueChanged( const TQDateTime& ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "KDateTimeWidget" ) )
          connect( it.current(), TQT_SIGNAL( valueChanged( const TQDateTime& ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "KDatePicker" ) )
          connect( it.current(), TQT_SIGNAL( dateChanged( TQDate ) ),
                   TQT_SIGNAL( modified() ) );
        else if ( it.current()->inherits( "TQTextEdit" ) )
          connect( it.current(), TQT_SIGNAL( textChanged() ),
                   TQT_SIGNAL( modified() ) );

        if ( !widget->isEnabled() )
          mDisabledWidgets.append( widget );
      }
    }

    ++it;
  }

  delete list;
}

TQString DesignerFields::identifier() const
{
  return mIdentifier;
}

TQString DesignerFields::title() const
{
  return mTitle;
}

void DesignerFields::load( DesignerFields::Storage *storage )
{
  TQStringList keys = storage->keys();
    
  // clear all custom page widgets 
  // we can't do this in the following loop, as it works on the 
  // custom fields of the vcard, which may not be set.
  TQMap<TQString, TQWidget *>::ConstIterator widIt;
  for ( widIt = mWidgets.begin(); widIt != mWidgets.end(); ++widIt ) {
    TQString value;
    if ( widIt.data()->inherits( "TQLineEdit" ) ) {
      TQLineEdit *wdg = static_cast<TQLineEdit*>( widIt.data() );
      wdg->setText( TQString::null );
    } else if ( widIt.data()->inherits( "TQSpinBox" ) ) {
      TQSpinBox *wdg = static_cast<TQSpinBox*>( widIt.data() );
      wdg->setValue( wdg->minValue() );
    } else if ( widIt.data()->inherits( "TQCheckBox" ) ) {
      TQCheckBox *wdg = static_cast<TQCheckBox*>( widIt.data() );
      wdg->setChecked( false );
    } else if ( widIt.data()->inherits( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( widIt.data() );
      wdg->setDateTime( TQDateTime::currentDateTime() );
    } else if ( widIt.data()->inherits( "KDateTimeWidget" ) ) {
      KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( widIt.data() );
      wdg->setDateTime( TQDateTime::currentDateTime() );
    } else if ( widIt.data()->inherits( "KDatePicker" ) ) {
      KDatePicker *wdg = static_cast<KDatePicker*>( widIt.data() );
      wdg->setDate( TQDate::currentDate() );
    } else if ( widIt.data()->inherits( "TQComboBox" ) ) {
      TQComboBox *wdg = static_cast<TQComboBox*>( widIt.data() );
      wdg->setCurrentItem( 0 );
    } else if ( widIt.data()->inherits( "TQTextEdit" ) ) {
      TQTextEdit *wdg = static_cast<TQTextEdit*>( widIt.data() );
      wdg->setText( TQString::null );
    }
  }

  TQStringList::ConstIterator it2;
  for ( it2 = keys.begin(); it2 != keys.end(); ++it2 ) {
    TQString value = storage->read( *it2 );

    TQMap<TQString, TQWidget *>::ConstIterator it = mWidgets.find( *it2 );
    if ( it != mWidgets.end() ) {
      if ( it.data()->inherits( "TQLineEdit" ) ) {
        TQLineEdit *wdg = static_cast<TQLineEdit*>( it.data() );
        wdg->setText( value );
      } else if ( it.data()->inherits( "TQSpinBox" ) ) {
        TQSpinBox *wdg = static_cast<TQSpinBox*>( it.data() );
        wdg->setValue( value.toInt() );
      } else if ( it.data()->inherits( "TQCheckBox" ) ) {
        TQCheckBox *wdg = static_cast<TQCheckBox*>( it.data() );
        wdg->setChecked( value == "true" || value == "1" );
      } else if ( it.data()->inherits( "QDateTimeEdit" ) ) {
        QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( it.data() );
        wdg->setDateTime( TQDateTime::fromString( value, Qt::ISODate ) );
      } else if ( it.data()->inherits( "KDateTimeWidget" ) ) {
        KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( it.data() );
        wdg->setDateTime( TQDateTime::fromString( value, Qt::ISODate ) );
      } else if ( it.data()->inherits( "KDatePicker" ) ) {
        KDatePicker *wdg = static_cast<KDatePicker*>( it.data() );
        wdg->setDate( TQDate::fromString( value, Qt::ISODate ) );
      } else if ( it.data()->inherits( "TQComboBox" ) ) {
        TQComboBox *wdg = static_cast<TQComboBox*>( it.data() );
        wdg->setCurrentText( value );
      } else if ( it.data()->inherits( "TQTextEdit" ) ) {
        TQTextEdit *wdg = static_cast<TQTextEdit*>( it.data() );
        wdg->setText( value );
      }
    }
  }
}

void DesignerFields::save( DesignerFields::Storage *storage )
{
  TQMap<TQString, TQWidget*>::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    TQString value;
    if ( it.data()->inherits( "TQLineEdit" ) ) {
      TQLineEdit *wdg = static_cast<TQLineEdit*>( it.data() );
      value = wdg->text();
    } else if ( it.data()->inherits( "TQSpinBox" ) ) {
      TQSpinBox *wdg = static_cast<TQSpinBox*>( it.data() );
      value = TQString::number( wdg->value() );
    } else if ( it.data()->inherits( "TQCheckBox" ) ) {
      TQCheckBox *wdg = static_cast<TQCheckBox*>( it.data() );
      value = ( wdg->isChecked() ? "true" : "false" );
    } else if ( it.data()->inherits( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( it.data() );
      value = wdg->dateTime().toString( Qt::ISODate );
    } else if ( it.data()->inherits( "KDateTimeWidget" ) ) {
      KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( it.data() );
      value = wdg->dateTime().toString( Qt::ISODate );
    } else if ( it.data()->inherits( "KDatePicker" ) ) {
      KDatePicker *wdg = static_cast<KDatePicker*>( it.data() );
      value = wdg->date().toString( Qt::ISODate );
    } else if ( it.data()->inherits( "TQComboBox" ) ) {
      TQComboBox *wdg = static_cast<TQComboBox*>( it.data() );
      value = wdg->currentText();
    } else if ( it.data()->inherits( "TQTextEdit" ) ) {
      TQTextEdit *wdg = static_cast<TQTextEdit*>( it.data() );
      value = wdg->text();
   }

   storage->write( it.key(), value );
  }
}

void DesignerFields::setReadOnly( bool readOnly )
{
  TQMap<TQString, TQWidget*>::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it )
    if ( mDisabledWidgets.find( it.data() ) == mDisabledWidgets.end() )
      it.data()->setEnabled( !readOnly );
}

#include "designerfields.moc"
