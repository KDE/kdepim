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

#include <tqapplication.h>
#include <tqfile.h>

#include <schema/fileprovider.h>

#include "converter.h"
#include "creator.h"

#include "compiler.h"

using namespace KWSDL;

Compiler::Compiler()
  : TQObject( 0, "KWSDL::Compiler" )
{
}

void Compiler::setWSDLUrl( const TQString &wsdlUrl )
{
  mWSDLUrl = wsdlUrl;
  mWSDLBaseUrl = mWSDLUrl.left( mWSDLUrl.findRev( '/' ) );

  mParser.setSchemaBaseUrl( mWSDLBaseUrl );
}

void Compiler::setOutputDirectory( const TQString &outputDirectory )
{
  mOutputDirectory = outputDirectory;

  if ( !mOutputDirectory.endsWith( "/" ) )
    mOutputDirectory.append( "/" );
}

void Compiler::setNameSpace( const TQString &nameSpace )
{
  mNameSpace = nameSpace;
}

void Compiler::run()
{
  download();
}

void Compiler::download()
{
  Schema::FileProvider provider;

  TQString fileName;
  if ( provider.get( mWSDLUrl, fileName ) ) {
    TQFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
      qDebug( "Unable to download schema file %s", mWSDLUrl.latin1() );
      provider.cleanUp();
      return;
    }

    TQString errorMsg;
    int errorLine, errorCol;
    TQDomDocument doc;
    if ( !doc.setContent( &file, true, &errorMsg, &errorLine, &errorCol ) ) {
      qDebug( "%s at (%d,%d)", errorMsg.latin1(), errorLine, errorCol );
      return;
    }

    parse( doc.documentElement() );

    provider.cleanUp();
  }
}

void Compiler::parse( const TQDomElement &element )
{
  mParser.parse( element );
  create();
}

void Compiler::create()
{
  KWSDL::Converter converter;
  converter.setWSDL( mParser.wsdl() );

  converter.convert();

  KWSDL::Creator creator;
  creator.setOutputDirectory( mOutputDirectory );
  creator.setNameSpace( mNameSpace );
  creator.create( converter.classes() );

  qApp->quit();
}

#include "compiler.moc"
