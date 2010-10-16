/******************************************************************************
 *
 *  Copyright 2009 Kevin Ottens <ervin@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "subjectutils_p.h"

#include "messagelist/core/settings.h"

#include <KDebug>

#include <QRegExp>
#include <QStringList>

QString MessageList::Core::SubjectUtils::stripOffPrefixes( const QString &subject )
{
  static QStringList defaultReplyPrefixes = QStringList() << QLatin1String( "Re\\s*:" )
                                                          << QLatin1String( "Re\\[\\d+\\]:" )
                                                          << QLatin1String( "Re\\d+:" );

  static QStringList defaultForwardPrefixes = QStringList() << QLatin1String( "Fwd:" )
                                                            << QLatin1String( "FW:" );

  QStringList replyPrefixes = Settings::self()->replyPrefixes();
  if ( replyPrefixes.isEmpty() )
    replyPrefixes = defaultReplyPrefixes;

  QStringList forwardPrefixes = Settings::self()->forwardPrefixes();
  if ( forwardPrefixes.isEmpty() )
    forwardPrefixes = defaultReplyPrefixes;

  const QStringList prefixRegExps = replyPrefixes + forwardPrefixes;

  // construct a big regexp that
  // 1. is anchored to the beginning of str (sans whitespace)
  // 2. matches at least one of the part regexps in prefixRegExps
  const QString bigRegExp = QString::fromLatin1( "^(?:\\s+|(?:%1))+\\s*" ).arg( prefixRegExps.join( QLatin1String( ")|(?:" ) ) );

  static QString regExpPattern;
  static QRegExp regExp;

  if ( regExpPattern != bigRegExp ) {
    // the prefixes have changed, so update the regexp
    regExpPattern = bigRegExp;
    regExp.setPattern( regExpPattern );
  }

  if ( !regExp.isValid() ) {
    kWarning() << "bigRegExp = \""
               << bigRegExp << "\"\n"
               << "prefix regexp is invalid!";
  } else {
    QString tmp = subject;
    if ( regExp.indexIn( tmp ) == 0 ) {
      return tmp.replace( 0, regExp.matchedLength(), QString() );
    }
  }

  return subject;
}

