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
#include <qdatetimeedit.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qspinbox.h>
#include <qwidgetfactory.h>

#include <kdatepicker.h>
#include <kdatetimewidget.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kstandarddirs.h>

#include "advancedcustomfields.h"

static void splitField( const QString&, QString&, QString&, QString& );


AdvancedCustomFields::AdvancedCustomFields( const QString &uiFile, KABC::AddressBook *ab,
                                            QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  initGUI( uiFile );
}

void AdvancedCustomFields::loadContact( KABC::Addressee *addr )
{
  QStringList customs = addr->customs();

  QStringList::ConstIterator it;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );

    if ( app == "KADDRESSBOOK" ) {
      QMap<QString, QWidget*>::Iterator it = mWidgets.find( name );
      if ( it != mWidgets.end() ) {
        if ( it.data()->isA( "QLineEdit" ) || it.data()->isA( "KLineEdit" ) ) {
          QLineEdit *wdg = static_cast<QLineEdit*>( it.data() );
          wdg->setText( value );
        } else if ( it.data()->isA( "QSpinBox" ) ) {
          QSpinBox *wdg = static_cast<QSpinBox*>( it.data() );
          wdg->setValue( value.toInt() );
        } else if ( it.data()->isA( "QCheckBox" ) ) {
          QCheckBox *wdg = static_cast<QCheckBox*>( it.data() );
          wdg->setChecked( value == "true" || value == "1" );
        } else if ( it.data()->isA( "QDateTimeEdit" ) ) {
          QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( it.data() );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        } else if ( it.data()->isA( "KDateTimeWidget" ) ) {
          KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( it.data() );
          wdg->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
        } else if ( it.data()->isA( "KDatePicker" ) ) {
          KDatePicker *wdg = static_cast<KDatePicker*>( it.data() );
          wdg->setDate( QDate::fromString( value, Qt::ISODate ) );
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
    if ( it.data()->isA( "QLineEdit" ) || it.data()->isA( "KLineEdit" ) ) {
      QLineEdit *wdg = static_cast<QLineEdit*>( it.data() );
      value = wdg->text();
    } else if ( it.data()->isA( "QSpinBox" ) ) {
      QSpinBox *wdg = static_cast<QSpinBox*>( it.data() );
      value = QString::number( wdg->value() );
    } else if ( it.data()->isA( "QCheckBox" ) ) {
      QCheckBox *wdg = static_cast<QCheckBox*>( it.data() );
      value = ( wdg->isChecked() ? "true" : "false" );
    } else if ( it.data()->isA( "QDateTimeEdit" ) ) {
      QDateTimeEdit *wdg = static_cast<QDateTimeEdit*>( it.data() );
      value = wdg->dateTime().toString( Qt::ISODate );
    } else if ( it.data()->isA( "KDateTimeWidget" ) ) {
      KDateTimeWidget *wdg = static_cast<KDateTimeWidget*>( it.data() );
      value = wdg->dateTime().toString( Qt::ISODate );
    } else if ( it.data()->isA( "KDatePicker" ) ) {
      KDatePicker *wdg = static_cast<KDatePicker*>( it.data() );
      value = wdg->date().toString( Qt::ISODate );
    }

    addr->insertCustom( "KADDRESSBOOK", it.key(), value );
  }
}

void AdvancedCustomFields::setReadOnly( bool readOnly )
{
  QMap<QString, QWidget*>::Iterator it;
  for ( it = mWidgets.begin(); it != mWidgets.end(); ++it )
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
               << "KLineEdit"
               << "QSpinBox"
               << "QCheckBox"
               << "QDateTimeEdit"
               << "KDateTimeWidget"
               << "KDatePicker";

  while ( it.current() ) {
    if ( allowedTypes.contains( it.current()->className() ) ) {
      QString name = it.current()->name();
      if ( name.startsWith( "X_" ) ) {
        name = name.mid( 2 );
        if ( !name.isEmpty() )
          mWidgets.insert( name, static_cast<QWidget*>( it.current() ) );

        if ( it.current()->isA( "QLineEdit" ) || it.current()->isA( "KLineEdit" ) )
          connect( it.current(), SIGNAL( textChanged( const QString& ) ),
                   this, SIGNAL( changed() ) );
        else if ( it.current()->isA( "QSpinBox" ) )
          connect( it.current(), SIGNAL( valueChanged( int ) ),
                   this, SIGNAL( changed() ) );
        else if ( it.current()->isA( "QCheckBox" ) )
          connect( it.current(), SIGNAL( toggled( bool ) ),
                   this, SIGNAL( changed() ) );
        else if ( it.current()->isA( "QDateTimeEdit" ) )
          connect( it.current(), SIGNAL( valueChanged( const QDateTime& ) ),
                   this, SIGNAL( changed() ) );
        else if ( it.current()->isA( "KDateTimeWidget" ) )
          connect( it.current(), SIGNAL( valueChanged( const QDateTime& ) ),
                   this, SIGNAL( changed() ) );
        else if ( it.current()->isA( "KDatePicker" ) )
          connect( it.current(), SIGNAL( dateChanged( QDate ) ),
                   this, SIGNAL( changed() ) );
      }
    }

    ++it;
  }
}

QString AdvancedCustomFields::pageIdentifier() const
{
  return mIdentifier;
}

QString AdvancedCustomFields::pageTitle() const
{
  return mTitle;
}

static void splitField( const QString &str, QString &app, QString &name, QString &value )
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

#include "advancedcustomfields.moc"
