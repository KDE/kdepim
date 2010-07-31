/*
    This file is part of Kung.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <kcombobox.h>

#include <schema/simpletype.h>

#include "enuminputfield.h"

EnumInputField::EnumInputField( const TQString &name, const Schema::SimpleType *type )
  : SimpleInputField( name, type )
{
  mEnums = type->facetEnums();

  mValue = mEnums.first();
}

void EnumInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "EnumInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  setData( element.text() );
}

void EnumInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );
  element.setAttribute( "xsi:type", "xsd:string" );
  TQDomText text = document.createTextNode( data() );
  element.appendChild( text );

  parent.appendChild( element );
}

void EnumInputField::setData( const TQString &data )
{
  mValue = data;
}

TQString EnumInputField::data() const
{
  return mValue;
}

TQWidget *EnumInputField::createWidget( TQWidget *parent )
{
  mInputWidget = new KComboBox( parent );

  mInputWidget->insertStringList( mEnums );
  mInputWidget->setCurrentItem( mEnums.findIndex( mValue ) );

  connect( mInputWidget, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( inputChanged( int ) ) );

  return mInputWidget;
}

void EnumInputField::inputChanged( int value )
{
  mValue = mEnums[ value ];

  emit modified();
}

#include "enuminputfield.moc"
