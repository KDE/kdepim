/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmeprogresstokenmapper.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "qgpgmeprogresstokenmapper.h"

#include <klocale.h>

#include <qstring.h>

#include <assert.h>
#include <map>

struct Desc {
  int type; // 0 == fallback
  const char * display; // add %1 for useCur ^ useTot and %1 %2 for useCur == useTot == true
  bool useCur : 1;
  bool useTot : 1;
};

static const struct Desc pk_dsa[] = {
  { 0, I18N_NOOP("Generating DSA key..."), false, false }
};

static const struct Desc pk_elg[] = {
  { 0, I18N_NOOP("Generating ElGamal key..."), false, false }
};

static const struct Desc primegen[] = {
  // FIXME: add all type's?
  { 0, I18N_NOOP("Searching for a large prime number..."), false, false }
};

static const struct Desc need_entropy[] = {
  { 0, I18N_NOOP("Waiting for new entropy from random number generator (you might want to excercise the harddisks or move the mouse)..."), false, false }
};

static const struct Desc tick[] = {
  { 0, I18N_NOOP("Please wait..."), false, false }
};

static const struct Desc starting_agent[] = {
  { 0, I18N_NOOP("Starting gpg-agent (you should consider starting a global instance instead)..."), false, false }
};

static const struct {
  const char * token;
  const Desc * desc;
  unsigned int numDesc;
} tokens[] = {
#define make_token(x) { #x, x, sizeof(x) / sizeof(*x) }
  make_token(pk_dsa),
  make_token(pk_elg),
  make_token(primegen),
  make_token(need_entropy),
  make_token(tick),
  make_token(starting_agent)
#undef make_token
};



Kleo::QGpgMEProgressTokenMapper * Kleo::QGpgMEProgressTokenMapper::mSelf = 0;

const Kleo::QGpgMEProgressTokenMapper * Kleo::QGpgMEProgressTokenMapper::instance() {
  if ( !mSelf )
    (void) new QGpgMEProgressTokenMapper();
  return mSelf;
}

Kleo::QGpgMEProgressTokenMapper::QGpgMEProgressTokenMapper() {
  mSelf = this;
}

Kleo::QGpgMEProgressTokenMapper::~QGpgMEProgressTokenMapper() {
  mSelf = 0;
}

typedef std::map< QString, std::map<int,Desc> > Map;

static const Map & makeMap() { // return a reference to a static to avoid copying
  static Map map;
  for ( unsigned int i = 0 ; i < sizeof tokens / sizeof *tokens ; ++i ) {
    assert( tokens[i].token );
    const QString token = QString::fromLatin1( tokens[i].token ).lower();
    for ( unsigned int j = 0 ; j < tokens[i].numDesc ; ++j ) {
      const Desc & desc = tokens[i].desc[j];
      assert( desc.display );
      map[ token ][ desc.type ] = desc;
    }
  }
  return map;
}

QString Kleo::QGpgMEProgressTokenMapper::map( const char * tokenUtf8, int subtoken, int cur, int tot ) const {
  if ( !tokenUtf8 || !*tokenUtf8 )
    return QString::null;

  if ( qstrcmp( tokenUtf8, "file:" ) == 0 )
    return QString::null; // gpgme's job

  return map( QString::fromUtf8( tokenUtf8 ), subtoken, cur, tot );
}

QString Kleo::QGpgMEProgressTokenMapper::map( const QString & token, int subtoken, int cur, int tot ) const {
  if ( token.startsWith( "file:" ) )
    return QString::null; // gpgme's job

  static const Map & tokenMap = makeMap();

  const Map::const_iterator it1 = tokenMap.find( token.lower() );
  if ( it1 == tokenMap.end() )
    return token;
  std::map<int,Desc>::const_iterator it2 = it1->second.find( subtoken );
  if ( it2 == it1->second.end() )
    it2 = it1->second.find( 0 );
  if ( it2 == it1->second.end() )
    return token;
  const Desc & desc = it2->second;
  QString result = i18n( desc.display );
  if ( desc.useCur )
    result = result.arg( cur );
  if ( desc.useTot )
    result = result.arg( tot );
  return result;
}

