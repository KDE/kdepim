/*
    This file is part of KDE.

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

#include <tqdom.h>

#include "parser.h"

using namespace KWSDL;

static TQString sns( const TQString &str )
{
  int pos = str.find( ':' );
  if ( pos != -1 )
    return str.mid( pos + 1 );
  else
    return str;
}

Parser::Parser()
{
}

WSDL Parser::wsdl() const
{
  WSDL wsdl;

  wsdl.setBindings( mBindings );
  wsdl.setMessages( mMessages );
  wsdl.setPorts( mPorts );
  wsdl.setService( mService );
  wsdl.setTypes( mParser.types() );

  return wsdl;
}

void Parser::reset()
{
  mMessages.clear();
  mPorts.clear();
  mService = Service();
  mParser.clear();
}

void Parser::setSchemaBaseUrl( const TQString &url )
{
  mParser.setSchemaBaseUrl( url );
}

void Parser::parse( const TQDomElement &root )
{
  reset();

  TQDomNode node = root.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "types" )
        parseTypes( element );
      else if ( element.tagName() == "message" ) {
        Message message( element.attribute( "name" ) );
        parseMessage( element, message );
        mMessages.append( message );
      } else if ( element.tagName() == "portType" ) {
        Port port( sns( element.attribute( "name" ) ) );
        parsePortType( element, port );
        mPorts.append( port );
      } else if ( element.tagName() == "binding" ) {
        parseBinding( element );
      } else if ( element.tagName() == "service" ) {
        mService = Service( sns( element.attribute( "name" ) ) );
        parseService( element );
      }
    }

    node = node.nextSibling();
  }
}

void Parser::parseTypes( const TQDomElement &parent )
{
  TQDomNode node = parent.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "schema" )
        mParser.parseSchemaTag( element );
    }

    node = node.nextSibling();
  }
}

void Parser::parseMessage( const TQDomElement &parent, Message &message )
{
  TQDomNode node = parent.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "part" ) {

        // HACK(groupwise): is element valid here as attribute?
        TQString type = sns( element.attribute( "type" ) );
        if ( type.isEmpty() )
          type = sns( element.attribute( "element" ) );

        Message::Part part( sns( element.attribute( "name" ) ), type );
        message.addPart( part );
      }
    }

    node = node.nextSibling();
  }
}

void Parser::parsePortType( const TQDomElement &parent, Port &port )
{
  TQDomNode node = parent.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "operation" ) {
        TQString input, output;

        TQDomNode childNode = element.firstChild();
        while ( !childNode.isNull() ) {
          TQDomElement childElement = childNode.toElement();
          if ( !childElement.isNull() ) {
            if ( childElement.tagName() == "input" )
              input = sns( childElement.attribute( "message" ) );
            else if ( childElement.tagName() == "output" )
              output = sns( childElement.attribute( "message" ) );
          }

          childNode = childNode.nextSibling();
        }

        Port::Operation operation( sns( element.attribute( "name" ) ), input, output );
        port.addOperation( operation );
      }
    }

    node = node.nextSibling();
  }
}

void Parser::parseBinding( const TQDomElement &parent )
{
  Binding binding( sns( parent.attribute( "name" ) ), sns( parent.attribute( "type" ) ) );

  TQDomNode node = parent.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "binding" ) {
        binding.setStyle( element.attribute( "style" ) );
        binding.setTransport( element.attribute( "transport" ) );
      } else if ( element.tagName() == "operation" ) {
        Binding::Operation operation;
        operation.setName( element.attribute( "name" ) );

        TQDomNode opNode = element.firstChild();
        while ( !opNode.isNull() ) {
          TQDomElement opElement = opNode.toElement();
          if ( !opElement.isNull() ) {
            if ( opElement.tagName() == "operation" )
              operation.setAction( opElement.attribute( "soapAction" ) );
            else if ( opElement.tagName() == "input" ) {
              TQDomElement inputElement = opElement.firstChild().toElement();

              Binding::Operation::Item input;
              input.setUse( inputElement.attribute( "use" ) );
              input.setNameSpace( inputElement.attribute( "namespace" ) );
              input.setEncodingStyle( inputElement.attribute( "encodingStyle" ) );

              operation.setInput( input );
            } else if ( opElement.tagName() == "output" ) {
              TQDomElement outputElement = opElement.firstChild().toElement();

              Binding::Operation::Item output;
              output.setUse( outputElement.attribute( "use" ) );
              output.setNameSpace( outputElement.attribute( "namespace" ) );
              output.setEncodingStyle( outputElement.attribute( "encodingStyle" ) );

              operation.setOutput( output );
            }
          }

          opNode = opNode.nextSibling();
        }

        binding.addOperation( operation );
      }
    }

    node = node.nextSibling();
  }

  mBindings.append( binding );
}

void Parser::parseService( const TQDomElement &parent )
{
  TQDomNode node = parent.firstChild();
  while ( !node.isNull() ) {
    TQDomElement element = node.toElement();
    if ( !element.isNull() ) {
      if ( element.tagName() == "port" ) {
        Service::Port port;
        port.mName = sns( element.attribute( "name" ) );
        port.mBinding = sns( element.attribute( "binding" ) );
        TQDomNode childNode = element.firstChild();
        if ( !childNode.isNull() ) {
          TQDomElement childElement = childNode.toElement();
          port.mLocation = childElement.attribute( "location" );
        }
        mService.addPort( port );
      }
    }

    node = node.nextSibling();
  }
}
