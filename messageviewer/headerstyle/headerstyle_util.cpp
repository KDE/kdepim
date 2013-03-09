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

#include "headerstyle_util.h"
#include "nodehelper.h"
#include "headerstyle.h"

#include <KLocale>
#include <KGlobal>

namespace MessageViewer {
namespace HeaderStyleUtil {
//
// Convenience functions:
//
QString directionOf( const QString & str ) {
  return str.isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr");
}

QString strToHtml( const QString & str, int flags ) {
  return LinkLocator::convertToHtml( str, flags );
}

// Prepare the date string (when printing always use the localized date)
QString dateString( KMime::Message *message, bool printing, bool shortDate ) {
  const KDateTime dateTime = message->date()->dateTime();
  if ( !dateTime.isValid() )
    return i18nc( "Unknown date", "Unknown" );
  if( printing ) {
    KLocale * locale = KGlobal::locale();
    return locale->formatDateTime( dateTime );
  } else {
    if ( shortDate )
      return MessageViewer::HeaderStyle::dateShortStr( dateTime );
    else
      return MessageViewer::HeaderStyle::dateStr( dateTime );
  }
}

QString subjectString( KMime::Message *message, int flags )
{
  QString subject;
  if ( message->subject(false) ) {
    subject = message->subject()->asUnicodeString();
    if ( subject.isEmpty() )
      subject = i18n("No Subject");
    else
      subject = strToHtml( subject, flags );
  } else {
    subject = i18n("No Subject");
  }
  return subject;
}

QString subjectDirectionString( KMime::Message *message )
{
  QString subjectDir;
  if ( message->subject(false) )
    subjectDir = directionOf( NodeHelper::cleanSubject( message ) );
  else
    subjectDir = directionOf( i18n("No Subject") );
  return subjectDir;
}
}
}
