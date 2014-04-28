/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "urihandler.h"
#include <kmailinterface.h>
#include <korganizerinterface.h>
#include <Akonadi/Contact/contacteditordialog.h>

#include <kiconloader.h>
#include <krun.h>
#include <kapplication.h>
#include <kshell.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

#include <QObject>

bool UriHandler::process( const QString &uri, const Akonadi::Item& item )
{
    kDebug() << uri;

    if ( uri.startsWith( QLatin1String( "kmail:" ) ) ) {
        // make sure kmail is running or the part is shown
        KToolInvocation::startServiceByDesktopPath( QLatin1String("kmail") );

        // parse string, show
        int colon = uri.indexOf( QLatin1Char(':') );
        // extract 'number' from 'kmail:<number>/<id>'
        QString serialNumberStr = uri.mid( colon + 1 );
        serialNumberStr = serialNumberStr.left( serialNumberStr.indexOf( QLatin1Char('/') ) );

        org::kde::kmail::kmail kmail(
                    QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QDBusConnection::sessionBus() );
        kmail.showMail( serialNumberStr.toLongLong() );
        return true;
    } else if ( uri.startsWith( QLatin1String( "mailto:" ) ) ) {
        KToolInvocation::invokeMailer( uri.mid(7), QString() );
        return true;
    } else if ( uri.startsWith( QLatin1String( "uid:" ) ) ) {

        Akonadi::ContactEditorDialog *dlg = new Akonadi::ContactEditorDialog( Akonadi::ContactEditorDialog::EditMode, ( QWidget* )0 );
        if ( item.isValid() ) {
            dlg->setContact( item );
            dlg->show();
            return true;
        } else {
            kDebug()<<"Item is not valid.";
            return false;
        }
    } else if ( uri.startsWith( QLatin1String( "urn:x-ical" ) ) ) {
        // make sure korganizer is running or the part is shown
        KToolInvocation::startServiceByDesktopPath( QLatin1String("korganizer") );

        // we must work around KUrl breakage (it doesn't know about URNs)
        const QString uid = KUrl::fromPercentEncoding( uri.toLatin1() ).mid( 11 );
        OrgKdeKorganizerKorganizerInterface korganizerIface(
                    QLatin1String("org.kde.korganizer"), QLatin1String("/Korganizer"), QDBusConnection::sessionBus() );

        return korganizerIface.showIncidence( uid );
    } else if ( uri.startsWith( QLatin1String( "akonadi:" ) ) ) {
        const KUrl url( uri );
        const QString mimeType = url.queryItem( QLatin1String( "type" ) );
        if ( mimeType.toLower() == QLatin1String( "message/rfc822" ) ) {
            // make sure kmail is running or the part is shown
            KToolInvocation::startServiceByDesktopPath( QLatin1String("kmail") );

            org::kde::kmail::kmail kmail(
                        QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QDBusConnection::sessionBus() );
            kmail.viewMessage( uri );
            return true;
        }
    } else {  // no special URI, let KDE handle it
        new KRun( KUrl( uri ), 0 );
    }

    return false;
}

