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
#include <Akonadi/Contact/ContactEditorDialog>

#include <krun.h>
#include <kshell.h>
#include "kdepimdbusinterface_debug.h"
#include <ktoolinvocation.h>
#include <QUrl>
#include <QObject>
#include <QDesktopServices>

bool UriHandler::process(const QString &uri, const Akonadi::Item &item)
{
    qCDebug(KDEPIMDBUSINTERFACE_LOG) << uri;

    if (uri.startsWith(QStringLiteral("kmail:"))) {
        // make sure kmail is running or the part is shown
        KToolInvocation::startServiceByDesktopPath(QStringLiteral("kmail"));

        // parse string, show
        int colon = uri.indexOf(QLatin1Char(':'));
        // extract 'number' from 'kmail:<number>/<id>'
        QString serialNumberStr = uri.mid(colon + 1);
        serialNumberStr = serialNumberStr.left(serialNumberStr.indexOf(QLatin1Char('/')));

        OrgKdeKmailKmailInterface kmailInterface(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QDBusConnection::sessionBus());
        kmailInterface.showMail(serialNumberStr.toLongLong());
        return true;
    } else if (uri.startsWith(QStringLiteral("mailto:"))) {
        QDesktopServices::openUrl(QUrl(uri));
        return true;
    } else if (uri.startsWith(QStringLiteral("uid:"))) {

        Akonadi::ContactEditorDialog *dlg = new Akonadi::ContactEditorDialog(Akonadi::ContactEditorDialog::EditMode, (QWidget *)0);
        if (item.isValid()) {
            dlg->setContact(item);
            dlg->show();
            return true;
        } else {
            qCDebug(KDEPIMDBUSINTERFACE_LOG) << "Item is not valid.";
            return false;
        }
    } else if (uri.startsWith(QStringLiteral("urn:x-ical"))) {
        // make sure korganizer is running or the part is shown
        KToolInvocation::startServiceByDesktopPath(QStringLiteral("korganizer"));

        // we must work around QUrl breakage (it doesn't know about URNs)
        const QString uid = QUrl::fromPercentEncoding(uri.toLatin1()).mid(11);
        OrgKdeKorganizerKorganizerInterface korganizerIface(
            QStringLiteral("org.kde.korganizer"), QStringLiteral("/Korganizer"), QDBusConnection::sessionBus());

        return korganizerIface.showIncidence(uid);
    } else if (uri.startsWith(QStringLiteral("akonadi:"))) {
        const QUrl url(uri);
        const QString mimeType = QUrlQuery(url).queryItemValue(QStringLiteral("type"));
        if (mimeType.toLower() == QLatin1String("message/rfc822")) {
            // make sure kmail is running or the part is shown
            KToolInvocation::startServiceByDesktopPath(QStringLiteral("kmail"));
            OrgKdeKmailKmailInterface kmailInterface(QStringLiteral("org.kde.kmail"), QStringLiteral("/KMail"), QDBusConnection::sessionBus());
            kmailInterface.viewMessage(uri);
            return true;
        }
    } else {  // no special URI, let KDE handle it
        new KRun(QUrl(uri), 0);
    }

    return false;
}

