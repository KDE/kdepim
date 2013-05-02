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

#include "defaultcompletion.h"

QStringList DefaultCompletion::defaultCompetion()
{
    //TODO add to highlighter
    QStringList lst;
    lst <<QLatin1String( "<div>" )
        <<QLatin1String( "subjecti18n" )
        <<QLatin1String( "subject" )
        <<QLatin1String( "replyToi18n" )
        <<QLatin1String( "replyTo" )
        <<QLatin1String( "replyToStr" )
        <<QLatin1String( "toi18n" )
        <<QLatin1String( "to" )
        <<QLatin1String( "toStr" )
        <<QLatin1String( "cci18n" )
        <<QLatin1String( "cc" )
        <<QLatin1String( "ccStr" )
        <<QLatin1String( "bcci18n" )
        <<QLatin1String( "bcc" )
        <<QLatin1String( "bccStr" )
        <<QLatin1String( "fromi18n" )
        <<QLatin1String( "from" )
        <<QLatin1String( "fromStr" )
        <<QLatin1String( "spamHTML" )
        <<QLatin1String( "spamstatusi18n" )
        <<QLatin1String( "datei18n")
        <<QLatin1String( "dateshort" )
        <<QLatin1String( "date" )
        <<QLatin1String( "useragent" )
        <<QLatin1String( "x-mailer" )
        <<QLatin1String( "resentfrom" )
        <<QLatin1String( "resentfromi18n" )
        <<QLatin1String( "organization" )
        <<QLatin1String( "vcardname" )
        <<QLatin1String( "activecolordark" )
        <<QLatin1String( "fontcolor" )
        <<QLatin1String( "linkcolor" )
        <<QLatin1String( "photowidth" )
        <<QLatin1String( "photoheight" )
        <<QLatin1String( "applicationDir" )
        <<QLatin1String( "subjectDir" )
        <<QLatin1String( "photourl" );
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst <<QLatin1String( "showlink" )
        <<QLatin1String( "nameonly" )
        <<QLatin1String( "Safe" );
    return lst;
}
