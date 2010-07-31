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

#include <tqlineedit.h>
#include <tqvalidator.h>

#include <schema/simpletype.h>

#include "stringinputfield.h"

StringInputField::StringInputField( const TQString &name, const TQString &typeName, const Schema::SimpleType *type )
  : SimpleInputField( name, type ),
    mTypeName( typeName )
{
}

void StringInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "StringInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  setData( element.text() );
}

void StringInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );
  element.setAttribute( "xsi:type", "xsd:" + mTypeName );
  TQDomText text = document.createTextNode( data() );
  element.appendChild( text );

  parent.appendChild( element );
}

void StringInputField::setData( const TQString &data )
{
  mValue = data;
}

TQString StringInputField::data() const
{
  return mValue;
}

TQWidget *StringInputField::createWidget( TQWidget *parent )
{
  mInputWidget = new TQLineEdit( parent );

  if ( mType ) {
    if ( mType->facetType() & Schema::SimpleType::LENGTH ) // TODO: using TQValidator here?
      mInputWidget->setMaxLength( mType->facetLength() );

    if ( mType->facetType() & Schema::SimpleType::MINLEN ) {
      // TODO: using TQValidator here?
      // mInputWidget->setMaxLength( type->facetMinimumLength() );
    }

    if ( mType->facetType() & Schema::SimpleType::MAXLEN )
      mInputWidget->setMaxLength( mType->facetMaximumLength() );

    if ( mType->facetType() & Schema::SimpleType::PATTERN )
      mInputWidget->setValidator( new TQRegExpValidator( mType->facetPattern(), mInputWidget ) );
  }

  mInputWidget->setText( mValue );

  connect( mInputWidget, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( inputChanged( const TQString& ) ) );

  return mInputWidget;
}

void StringInputField::inputChanged( const TQString &text )
{
  mValue = text;

  emit modified();
}

#include "stringinputfield.moc"
