/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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
*/

#include "configadvancedoptionwidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QScrollArea>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>
#include <klineedit.h>

OptionWidget::OptionWidget( const QSync::PluginAdvancedOption &option, QWidget *parent )
  : QWidget( parent ),
    mBoolValue( 0 ), mDoubleValue( 0 ), mIntValue( 0 ), mEnumValue( 0 ), mStringValue( 0 ),
    mOption( option )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  const QSync::PluginAdvancedOption::OptionType type = option.type();

  const QStringList enumValues = option.enumValues();
  if ( !enumValues.isEmpty() ) {
    QLabel *label = new QLabel( option.displayName() + ":", this );
    layout->addWidget( label );

    mEnumValue = new KComboBox( this );
    layout->addWidget( mEnumValue );

    for ( int i = 0; i < enumValues.count(); ++i )
      mEnumValue->addItem( enumValues.at( i ) );

  } else if ( type == QSync::PluginAdvancedOption::BoolOption ) {
    mBoolValue = new QCheckBox( option.displayName(), this );
    layout->addWidget( mBoolValue );

  } else if ( type == QSync::PluginAdvancedOption::StringOption ||
              type == QSync::PluginAdvancedOption::CharOption ) {
    QLabel *label = new QLabel( option.displayName() + ":", this );
    layout->addWidget( label );

    mStringValue = new KLineEdit( this );
    layout->addWidget( mStringValue );

    if ( option.type() == QSync::PluginAdvancedOption::CharOption )
      mStringValue->setInputMask( "X" );

  } else if ( type == QSync::PluginAdvancedOption::DoubleOption ) {
    QLabel *label = new QLabel( option.displayName() + ":", this );
    layout->addWidget( label );

    mDoubleValue = new QDoubleSpinBox( this );
    layout->addWidget( mDoubleValue );

    mDoubleValue->setRange( option.minimumSize(), option.maximumSize() );
  } else {
    QLabel *label = new QLabel( option.displayName() + ":", this );
    layout->addWidget( label );

    mIntValue = new QSpinBox( this );
    layout->addWidget( mIntValue );

    mIntValue->setRange( option.minimumSize(), option.maximumSize() );
  }
}

OptionWidget::~OptionWidget()
{
}

void OptionWidget::load()
{
  const QString value = mOption.value();

  if ( mEnumValue ) {
    if ( !value.isEmpty() )
      mEnumValue->setCurrentIndex( mEnumValue->findText( value ) );
  } else if ( mBoolValue ) {
    mBoolValue->setChecked( value == "1" );
  } else if ( mStringValue ) {
    mStringValue->setText( value );
  } else if ( mDoubleValue ) {
    mDoubleValue->setValue( value.toDouble() );
  } else {
    mIntValue->setValue( value.toInt() );
  }
}

void OptionWidget::save()
{
  if ( mEnumValue ) {
    mOption.setValue( mEnumValue->currentText() );
  } else if ( mBoolValue ) {
    mOption.setValue( mBoolValue->isChecked() ? "1" : "0" );
  } else if ( mStringValue ) {
    mOption.setValue( mStringValue->text() );
  } else if ( mDoubleValue ) {
    mOption.setValue( QString::number( mDoubleValue->value() ) );
  } else {
    mOption.setValue( QString::number( mIntValue->value() ) );
  }
}


ConfigAdvancedOptionWidget::ConfigAdvancedOptionWidget( const QSync::PluginAdvancedOption::List &options, QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QWidget *base = new QWidget;
  QVBoxLayout *baseLayout = new QVBoxLayout( base );
  for ( int i = 0; i < options.count(); ++i ) {
    OptionWidget *wdg = new OptionWidget( options.at( i ), base );
    baseLayout->addWidget( wdg );

    mOptionWidgets.append( wdg );
  }

  QScrollArea *area = new QScrollArea( this );
  area->setWidget( base );
  layout->addWidget( area );
}

ConfigAdvancedOptionWidget::~ConfigAdvancedOptionWidget()
{
}

void ConfigAdvancedOptionWidget::load()
{
  for ( int i = 0; i < mOptionWidgets.count(); ++i )
    mOptionWidgets.at( i )->load();
}

void ConfigAdvancedOptionWidget::save()
{
  for ( int i = 0; i < mOptionWidgets.count(); ++i )
    mOptionWidgets.at( i )->save();
}
