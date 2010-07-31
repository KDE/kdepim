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

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvalidator.h>

AccountInput::AccountInput( const TQString& configName )
	: _configName( new TQString( configName ) )
{
}

AccountInput::~AccountInput()
{
	delete _configName;
}

TQString AccountInput::configName() const
{
	return *_configName;
}

TextInput::TextInput( TQWidget *parent, const TQString& title, Type type, const TQString& defaul, const TQString& configName )
	: AccountInput( configName )
{
	_left = new TQLabel( title, parent, "label" );
	_right = new KLineEdit( "", parent, "edit" );
	switch( type )
	{
	case text:
		break;
	case password:
		_right->setEchoMode( TQLineEdit::Password );
		break;
	}
	setValue( defaul );
}

TextInput::TextInput( TQWidget *parent, const TQString& title, int min, int max, const TQString& defaul, const TQString& configName )
	: AccountInput( configName )
{
	_left = new TQLabel( title, parent, "label" );
	_right = new KLineEdit( "", parent, "edit" );
	_right->setValidator( new TQIntValidator( min, max, _right, "validator" ) );
	setValue( defaul );
}

TextInput::~TextInput()
{
	delete _left;
	delete _right;
}

TQString TextInput::value() const
{
	return _right->text();
}

void TextInput::setValue( const TQString& value )
{
	return _right->setText( value );
}

URLInput::URLInput( TQWidget *parent, const TQString& title, const TQString& defaul, const TQString& configName )
	: AccountInput( configName )
{
	_left = new TQLabel( title, parent, "label" );
	_right = new KURLRequester( "", parent, "kurledit" );
	setValue( defaul );
}

URLInput::~URLInput()
{
	delete _left;
	delete _right;
}

TQString URLInput::value() const
{
	return _right->url();
}

void URLInput::setValue( const TQString& value )
{
	_right->setURL( value );
}

ComboInput::ComboInput( TQWidget *parent, const TQString& title, const TQMap<TQString, TQString>& list,
                        const TQString& defaul, const TQString& configName )
	: AccountInput( configName )
	, _list( new TQMap< TQString, TQString >( list ) )
{
	_left = new TQLabel( title, parent, "label" );
	_right = new TQComboBox( false, parent, "combo" );
	_right->insertStringList( TQStringList( _list->values() ) );
	setValue( defaul );
}

ComboInput::~ComboInput()
{
	delete _left;
	delete _right;
}

TQString ComboInput::value() const
{
	if( _right->currentItem() >= 0 )
		return _list->keys()[ _right->currentItem() ];
	else
		return "";
}

void ComboInput::setValue( const TQString& value )
{
	if( _list->contains( value ) )
		_right->setCurrentItem( _list->keys().findIndex( value ) );
	else
		_right->setCurrentItem( -1 );
}

CheckboxInput::CheckboxInput( TQWidget *parent, const TQString& title, const TQString& defaul, const TQString& configName )
	: AccountInput( configName )
{
	_right = new TQCheckBox( title, parent, "checkbox" );
	setValue( defaul );
}

CheckboxInput::~CheckboxInput()
{
	delete _right;
}

TQString CheckboxInput::value() const
{
	if( _right->isChecked() )
		return "true";
	else
		return "false";
}

void CheckboxInput::setValue( const TQString& value )
{
	if( value == "true" )
		_right->setChecked( true );
	else if( value == "false" )
		_right->setChecked( false );
	//Elsewise: do nothing
}

