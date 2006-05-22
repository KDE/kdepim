/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "account_input.h"

#include <kurlrequester.h>
#include <klineedit.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QValidator>

AccountInput::AccountInput( const QString& configName )
	: _configName( new QString( configName ) )
{
}

AccountInput::~AccountInput()
{
	delete _configName;
}

QString AccountInput::configName() const
{
	return *_configName;
}

TextInput::TextInput( QWidget *parent, const QString& title, Type type, const QString& defaul, const QString& configName )
	: AccountInput( configName )
{
	_left = new QLabel( title, parent );
	_right = new KLineEdit( "", parent );
	switch( type )
	{
	case text:
		break;
	case password:
		_right->setEchoMode( QLineEdit::Password );
		break;
	}
	setValue( defaul );
}

TextInput::TextInput( QWidget *parent, const QString& title, int min, int max, const QString& defaul, const QString& configName )
	: AccountInput( configName )
{
	_left = new QLabel( title, parent );
	_right = new KLineEdit( "", parent );
	_right->setValidator( new QIntValidator( min, max, _right ) );
	setValue( defaul );
}

TextInput::~TextInput()
{
	delete _left;
	delete _right;
}

QString TextInput::value() const
{
	return _right->text();
}

void TextInput::setValue( const QString& value )
{
	return _right->setText( value );
}

URLInput::URLInput( QWidget *parent, const QString& title, const QString& defaul, const QString& configName )
	: AccountInput( configName )
{
	_left = new QLabel( title, parent );
	_right = new KUrlRequester( parent );
	setValue( defaul );
}

URLInput::~URLInput()
{
	delete _left;
	delete _right;
}

QString URLInput::value() const
{
	return _right->url();
}

void URLInput::setValue( const QString& value )
{
	_right->setUrl( value );
}

ComboInput::ComboInput( QWidget *parent, const QString& title, const QMap<QString, QString>& list,
                        const QString& defaul, const QString& configName )
	: AccountInput( configName )
	, _list( new QMap< QString, QString >( list ) )
{
	_left = new QLabel( title, parent );
	_right = new QComboBox( parent );
	_right->setInsertPolicy( QComboBox::NoInsert );
	_right->insertItems( 0, QStringList( _list->values() ) );
	setValue( defaul );
}

ComboInput::~ComboInput()
{
	delete _left;
	delete _right;
}

QString ComboInput::value() const
{
	if( _right->currentIndex() >= 0 )
		return _list->keys()[ _right->currentIndex() ];
	else
		return "";
}

void ComboInput::setValue( const QString& value )
{
	if( _list->contains( value ) )
		_right->setCurrentIndex( _list->keys().indexOf( value ) );
	else
		_right->setCurrentIndex( -1 );
}

CheckboxInput::CheckboxInput( QWidget *parent, const QString& title, const QString& defaul, const QString& configName )
	: AccountInput( configName )
{
	_right = new QCheckBox( title, parent );
	setValue( defaul );
}

CheckboxInput::~CheckboxInput()
{
	delete _right;
}

QString CheckboxInput::value() const
{
	if( _right->isChecked() )
		return "true";
	else
		return "false";
}

void CheckboxInput::setValue( const QString& value )
{
	if( value == "true" )
		_right->setChecked( true );
	else if( value == "false" )
		_right->setChecked( false );
	//Elsewise: do nothing
}

