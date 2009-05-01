/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "locale.h"

#include "knglobals.h"
#include "kngroup.h"
#include "settings.h"


#include <KCharsets>
#include <KGlobal>
#include <KLocale>

#include <QTextCodec>



using namespace KNode::Utilities;


QString Locale::toMimeCharset( const QString &charset )
{
  QString c = charset;

  // First, get the user preferred encoding
  if ( c.isEmpty() ) {
    c = KGlobal::locale()->encoding();
    if ( c.isEmpty() ) {
      // To be really sure...
      c = "UTF-8";
    }
  }

  // Second, convert to something sensible
  bool ok;
  QTextCodec *codec = KGlobal::charsets()->codecForName( c, ok );
  if ( ok && !codec->name().isEmpty() ) {
    c = codec->name();
  } else {
    c = "UTF-8";
  }

  // special logic for japanese users:
  // "euc-jp" is default encoding for them, but in the news
  // "iso-2022-jp" is used (#36638)
  if ( c.toUpper() == "EUC-JP" ) {
    c = "ISO-2022-JP";
  }

  // Uppercase is preferred in MIME headers
  c = c.toUpper();

  return c;
}



QByteArray Locale::defaultCharset()
{
  return toMimeCharset( knGlobals.settings()->charset() ).toLatin1();
}

QByteArray Locale::defaultCharset( KNGroup *g )
{
  if ( g && g->useCharset() ) {
    return toMimeCharset( g->defaultCharset() ).toLatin1();
  } else {
    return defaultCharset();
  }
}
