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

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <schema/complextype.h>

#include "inputfieldfactory.h"

#include "complexbaseinputfield.h"

ComplexBaseInputField::ComplexBaseInputField( const TQString &name, const Schema::ComplexType *type )
  : ComplexInputField( name, type )
{
  Schema::Element::List elements = type->elements();
  Schema::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.begin(); elemIt != elements.end(); ++elemIt ) {
    bool isList = ((*elemIt).maxOccurs() == UNBOUNDED);
    InputField *field = InputFieldFactory::self()->createField( (*elemIt).name(), (*elemIt).typeName(), isList );
    if ( !field ) {
      qDebug( "ComplexBaseInputField: Unable to create field of type %s", type->baseTypeName().latin1() );
    } else {
      appendChild( field );
    }
  }

  Schema::Attribute::List attributes = type->attributes();
  Schema::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt ) {
    InputField *field = InputFieldFactory::self()->createField( (*attrIt).name(), (*attrIt).typeName() );
    if ( !field ) {
      qDebug( "ComplexBaseInputField: Unable to create field of type %s", type->baseTypeName().latin1() );
    } else {
      appendChild( field );
    }
  }
}

void ComplexBaseInputField::setXMLData( const TQDomElement &element )
{
  if ( mName != element.tagName() ) {
    qDebug( "ComplexBaseInputField: Wrong dom element passed: expected %s, got %s", mName.latin1(), element.tagName().latin1() );
    return;
  }

  // elements
  if ( mType->isArray() ) {
    InputField *field = childField( "item" );
    field->setXMLData( element );
  } else {
    TQDomNode node = element.firstChild();
    while ( !node.isNull() ) {
      TQDomElement child = node.toElement();
      if ( !child.isNull() ) {
        InputField *field = childField( child.tagName() );
        if ( !field ) {
          qDebug( "ComplexBaseInputField: Child field %s does not exists", child.tagName().latin1() );
        } else {
          field->setXMLData( child );
        }
      }

      node = node.nextSibling();
    }
  }

  // attributes
  TQDomNamedNodeMap nodes = element.attributes();
  for ( uint i = 0; i < nodes.count(); ++i ) {
    TQDomNode node = nodes.item( i );
    TQDomAttr attr = node.toAttr();

    InputField *field = childField( attr.name() );
    if ( !field ) {
      qDebug( "ComplexBaseInputField: Child field %s does not exists", attr.name().latin1() );
    } else {
      field->setData( attr.value() );
    }
  }
}

void ComplexBaseInputField::xmlData( TQDomDocument &document, TQDomElement &parent )
{
  TQDomElement element = document.createElement( mName );

  Schema::Element::List elements = mType->elements();
  Schema::Element::List::ConstIterator elemIt;
  for ( elemIt = elements.begin(); elemIt != elements.end(); ++elemIt ) {
    InputField *field = childField( (*elemIt).name() );
    if ( !field ) {
      qDebug( "ComplexBaseInputField: No child found" );
    } else {
      field->xmlData( document, element );
    }
  }

  Schema::Attribute::List attributes = mType->attributes();
  Schema::Attribute::List::ConstIterator attrIt;
  for ( attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt ) {
    InputField *field = childField( (*attrIt).name() );
    if ( !field ) {
      qDebug( "ComplexBaseInputField: No child found" );
    } else {
      element.setAttribute( field->name(), field->data() );
    }
  }

  parent.appendChild( element );
}

void ComplexBaseInputField::setData( const TQString& )
{
}

TQString ComplexBaseInputField::data() const
{
  return TQString();
}

TQWidget *ComplexBaseInputField::createWidget( TQWidget *parent )
{
  TQGroupBox *inputWidget = new TQGroupBox( mName, parent );
  inputWidget->setColumnLayout( 0, Qt::Horizontal );
  TQGridLayout *layout = new TQGridLayout( inputWidget->layout(), 2, 2, 6 );

  InputField::List::Iterator it;
  int row = 0;
  for ( it = mFields.begin(); it != mFields.end(); ++it, ++row ) {
    TQLabel *label = new TQLabel( (*it)->name(), inputWidget );
    layout->addWidget( label, row, 0 );
    layout->addWidget( (*it)->createWidget( inputWidget ), row, 1 );
  }

  return inputWidget;
}
