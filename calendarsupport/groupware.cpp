/*
  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

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

#include "groupware.h"
#include "kcalprefs.h"
#include "mailclient.h"

#include <akonadi/calendar/calendarsettings.h>
#include <KCalUtils/IncidenceFormatter>
#include <KCalUtils/Stringify>

#include <KLocale>
#include <KMessageBox>

using namespace CalendarSupport;

Groupware *Groupware::mInstance = 0;

GroupwareUiDelegate::~GroupwareUiDelegate()
{
}

struct Invitation {
  QString type;
  QString iCal;
  QString receiver;
};

class Groupware::Private
{
  public:
    Private() {}
};

Groupware *Groupware::create( GroupwareUiDelegate *delegate )
{
  if ( !mInstance ) {
    mInstance = new Groupware( delegate );
  }
  return mInstance;
}

Groupware *Groupware::instance()
{
  // Doesn't create, that is the task of create()
  Q_ASSERT( mInstance );
  return mInstance;
}

Groupware::Groupware( GroupwareUiDelegate *delegate )
  : QObject( 0 ), mDelegate( delegate ), mDoNotNotify( false ), d( new Private )
{
  setObjectName( QLatin1String( "kmgroupware_instance" ) );
}

Groupware::~Groupware()
{
  delete d;
}

class KOInvitationFormatterHelper : public KCalUtils::InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id )
    {
      return QLatin1String( "kmail:groupware_request_" ) + id;
    }
};

#include "groupware.moc"
