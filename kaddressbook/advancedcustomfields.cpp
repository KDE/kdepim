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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qspinbox.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <qwidgetfactory.h>

#include <kdatepicker.h>
#include <kdatetimewidget.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kstandarddirs.h>

#include "customfieldswidget.h"

#include "advancedcustomfields.h"


AdvancedCustomFields::AdvancedCustomFields( const QString &uiFile, KABC::AddressBook *ab,
                                            QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  initGUI( uiFile );
}

void AdvancedCustomFields::loadContact( KABC::Addressee *addr )
{
  QStringList customs = addr->customs();
    
  QString ns;
  if ( (mIdentifier.upper() == "KADDRESSBOOK") ||
        (QRegExp( "^Form\\d\\d?$").search(mIdentifier) >= 0 )
      )
    ns = "KADDRESSBOOK";
  else 
    ns = mIdentifier;
  
  // clear all custom page widgets 
  // we can't do this in the following loop, as it works on the 
  // custom fields of the vcard, which may not be set.
  QMap<QString, QWidget*>::Iterator widIt;
  for ( widIt = mWidgets.begin(); widIt != mWidgets.end(); ++widIt ) {
    QString value;
    if ( widIt.data()->inherits( "QLineEdit" ) ) {
      QLineEdit *wdg = static_cast<QLineEdit*>( widIt.data() );
      wdg->setText( QString::null );
    } else if ( widIt.data()->inherits( "QSpinBox" ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( widIt.data() );
      wdg->setValue( wdg->minValue() );
    } else if ( widIt.data()->inherits( "QCheckBox" ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( widIt.data() );
      wdg->setChecked( false );
    } else if ( widIt.data()->inherits( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( widIt.data() );
      wdg->setDateTime( QDateTime::currentDateTime() );
    } else if ( widIt.data()->inherits( "KDateTimeWidget" ) ) {
      KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( widIt.data() );
      wdg->setDateTime( QDateTime::currentDateTime() );
    } else if ( widIt.data()->inherits( "KDatePicker" ) ) {
      KDatePicker *wdg = static_cast<KDatePicker*>( widIt.data() );
      wdg->setDate( QDate::currentDate() );
    } else if ( widIt.data()->inherits( "QComboBox" ) ) {
      QComboBox *wdg = static_cast<QComboBox*>( widIt.data() );
      wdg->setCurrentItem( 0 );
    } else if ( widIt.data()->inherits( "QTextEdit" ) ) {
      QTextEdit *wdg = static_cast<QTextEdit*>( widIt.data() );
      wdg->setText( QString::null );
    }
  }

  QStringList::ConstIterator it;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );

    if ( app == ns ) {
      QMap<QString, QWidget*>::Iterator it = mWidgets.find( name );
      if ( it != mWidgets.end() ) {
        if ( it.data()->inherits( "QLineEdit" ) ) {
          QLineEdit *wdg = static_cast<QLineEdit*>( it.data() );
          wdg->setText( value );
        } else if ( it.data()->inherits( "QSpinBox" ) ) {
          QSpinBox *wdg = static_cast<QSpinBox*>( it.data() );
          wdg->setValue( value.toInt() );
        } else if ( it.data()->inherits( "QCheckBox" ) ) {
          QCheckBox *wdg = static_cast<QCheckBox*>( it.data() );
          wdg->setChecked( value == "true" || value == "1" );
        } else if ( it.data()->inherits( "QDateTimeEdit" ) ) {
          QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( it.data() );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        } else if ( it.data()->inherits( "KDateTimeWidget" ) ) {
          KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( it.data() );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        } else if ( it.data()->inherits( "KDatePicker" ) ) {
          KDatePicker *wdg = static_cast<KDatePicker*>( it.data() );
          wdg->setDate( QDate::fromString( value, Qt::ISODate ) );
        } else if ( it.data()->inherits( "QComboBox" ) ) {
          QComboBox *wdg = static_cast<QComboBox*>( it.data() );
          wdg->setCurrentText( value );
        } else if ( it.data()->inherits( "QTextEdit" ) ) {
          QTextEdit *wdg = static_cast<QTextEdit*>( it.data() );
          wdg->setText( value );
        }
      }
    }
  }
}

void AdvancedCustomFields::storeContact( KABC::Addressee *addr )
{
  QMap<QString, QWidget*>::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it ) {
    QString value;
    if ( it.data()->inherits( "QLineEdit" ) ) {
      QLineEdit *wdg = static_cast<QLineEdit*>( it.data() );
      value = wdg->text();
    } else if ( it.data()->inherits( "QSpinBox" ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( it.data() );
      value = QString::number( wdg->value() );
    } else if ( it.data()->inherits( "QCheckBox" ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( it.data() );
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
    } else if ( it.data()->inherits( "QComboBox" ) ) {
      QComboBox *wdg = static_cast<QComboBox*>( it.data() );
      value = wdg->currentText();
    } else if ( it.data()->inherits( "QTextEdit" ) ) {
      QTextEdit *wdg = static_cast<QTextEdit*>( it.data() );
      value = wdg->text();
   }

    QString ns;
    if ( (mIdentifier.upper() == "KADDRESSBOOK") ||
         (QRegExp( "^Form\\d\\d?$").search(mIdentifier) >= 0 )
       )
      ns = "KADDRESSBOOK";
    else 
      ns = mIdentifier;

    if ( value.isEmpty() )
      addr->removeCustom( ns.latin1(), it.key() );
    else
      addr->insertCustom( ns.latin1(), it.key(), value );
  }
}

void AdvancedCustomFields::setReadOnly( bool readOnly )
{
  QMap<QString, QWidget*>::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it )
    if ( mDisabledWidgets.find( it.data() ) == mDisabledWidgets.end() )
      it.data()->setEnabled( !readOnly );
}

void AdvancedCustomFields::initGUI( const QString &uiFile )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  QWidget *wdg = QWidgetFactory::create( uiFile, 0, this );
  if ( !wdg ) {
    kdError() << "No ui file found" << endl;
    return;
  }

  mTitle = wdg->caption();
  mIdentifier = wdg->name();

  layout->addWidget( wdg );

  QObjectList *list = wdg->queryList( "QWidget" );
  QObjectListIt it( *list );

  QStringList allowedTypes;
  allowedTypes << "QLineEdit"
               << "QTextEdit"
               << "QSpinBox"
               << "QCheckBox"
               << "QComboBox"
               << "QDateTimeEdit"
               << "KLineEdit"
               << "KDateTimeWidget"
               << "KDatePicker";

  while ( it.current() ) {
    if ( allowedTypes.contains( it.current()->className() ) ) {
      QString name = it.current()->name();
      if ( name.startsWith( "X_" ) ) {
        name = name.mid( 2 );

        QWidget *widget = static_cast<QWidget*>( it.current() );
        if ( !name.isEmpty() )
          mWidgets.insert( name, widget );

        if ( it.current()->inherits( "QLineEdit" ) )
          connect( it.current(), SIGNAL( textChanged( const QString& ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "QSpinBox" ) )
          connect( it.current(), SIGNAL( valueChanged( int ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "QCheckBox" ) )
          connect( it.current(), SIGNAL( toggled( bool ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "QComboBox" ) )
          connect( it.current(), SIGNAL( activated( const QString& ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "QDateTimeEdit" ) )
          connect( it.current(), SIGNAL( valueChanged( const QDateTime& ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "KDateTimeWidget" ) )
          connect( it.current(), SIGNAL( valueChanged( const QDateTime& ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "KDatePicker" ) )
          connect( it.current(), SIGNAL( dateChanged( QDate ) ),
                   this, SLOT( setModified() ) );
        else if ( it.current()->inherits( "QTextEdit" ) )
          connect( it.current(), SIGNAL( textChanged() ),
                   this, SLOT( setModified() ) );

        if ( !widget->isEnabled() )
          mDisabledWidgets.append( widget );
      }
    }

    ++it;
  }

  delete list;
}

QString AdvancedCustomFields::pageIdentifier() const
{
  return mIdentifier;
}

QString AdvancedCustomFields::pageTitle() const
{
  return mTitle;
}

#include "advancedcustomfields.moc"
