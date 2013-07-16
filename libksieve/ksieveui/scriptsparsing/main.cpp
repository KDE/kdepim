/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <QApplication>
#include "xmlprintingscriptbuilder.h"
//#include <config-libksieve.h> // SIZEOF_UNSIGNED_LONG
#include <ksieve/parser.h>
using KSieve::Parser;

#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>

#include <QDebug>

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  QString script = QLatin1String( "require \"reject\";"
          "require \"fileinto\";"
          "require \"envelope\";"
          "if size :under 1 {"
          "   fileinto \"INBOX\";"
                                  "}"
                                  );
  const QByteArray scriptUTF8 = script.trimmed().toUtf8();
  qDebug() << "scriptUtf8 = \"" + scriptUTF8 +"\"";

  KSieve::Parser parser( scriptUTF8.begin(),
                         scriptUTF8.begin() + scriptUTF8.length() );
  XMLPrintingScriptBuilder psb;
  parser.setScriptBuilder( &psb );
  if ( parser.parse() )
    qDebug() << "ok";
  else
    qDebug() << "bad";

  return 0;
}
