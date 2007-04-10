/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "parsertest.h"

#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefattach.h>

#include "assert.h"

void ParserTest::testSingleAttachment()
{
  KTNEFParser parser;
  assert( parser.openFile( KDESRCDIR "/one-file.tnef" ) == true );

  KTNEFMessage *msg = parser.message();
  assert( msg != 0 );

  QPtrList<KTNEFAttach> atts = msg->attachmentList();
  assert( atts.count() == 1 );

  KTNEFAttach* att = atts.first();
  assert( att != 0 );
  assert( att->size() == 244 );
  assert( att->name() == QString( "AUTHORS" ) );
}

void ParserTest::testTwoAttachments()
{
  KTNEFParser parser;
  assert( parser.openFile( KDESRCDIR "/two-files.tnef" ) == true );

  KTNEFMessage *msg = parser.message();
  assert( msg != 0 );

  QPtrList<KTNEFAttach> atts = msg->attachmentList();
  assert( atts.count() == 2 );

  KTNEFAttach* att = atts.at( 0 );
  assert( att != 0 );
  assert( att->size() == 244 );
  assert( att->name() == QString( "AUTHORS" ) );

  att = atts.at( 1 );
  assert( att != 0 );
  assert( att->size() == 893 );
  assert( att->name() == QString( "README" ) );
}

void ParserTest::testMAPIAttachments()
{
  KTNEFParser parser;
  assert( parser.openFile( KDESRCDIR "/mapi_attach_data_obj.tnef" ) == true );

  KTNEFMessage *msg = parser.message();
  assert( msg != 0 );

  QPtrList<KTNEFAttach> atts = msg->attachmentList();
  assert( atts.count() == 3 );

  KTNEFAttach* att = atts.at( 0 );
  assert( att != 0 );
  assert( att->size() == 61952 );
  assert( att->name() == QString( "VIA_Nytt_1402.doc" ) );

  att = atts.at( 1 );
  assert( att != 0 );
  assert( att->size() == 213688 );
  assert( att->name() == QString( "VIA_Nytt_1402.pdf" ) );

  att = atts.at( 2 );
  assert( att != 0 );
  assert( att->size() == 68920 );
  assert( att->name() == QString( "VIA_Nytt_14021.htm" ) );
}

#include <kinstance.h>

int main( int argc, char** argv )
{
  KInstance inst( "ktnef-parsertest" );
  ParserTest test;
  test.testSingleAttachment();
  test.testTwoAttachments();
  test.testMAPIAttachments();
  return 0;
}
