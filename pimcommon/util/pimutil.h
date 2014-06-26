/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/
#ifndef PIMCOMMON_MAILUTIL_H
#define PIMCOMMON_MAILUTIL_H

#include "pimcommon_export.h"

#include <KUrl>
class OrgKdeAkonadiImapSettingsInterface;
class QWidget;

#define IMAP_RESOURCE_IDENTIFIER QLatin1String("akonadi_imap_resource")
#define KOLAB_RESOURCE_IDENTIFIER QLatin1String("akonadi_kolab_resource")
#define POP3_RESOURCE_IDENTIFIER QLatin1String("akonadi_pop3_resource")
#define MBOX_RESOURCE_IDENTIFIER QLatin1String("akonadi_mbox_resource")
#define GMAIL_RESOURCE_IDENTIFIER QLatin1String("akonadi_gmail_resource")
namespace PimCommon {

/**
 * The Util namespace contains a collection of helper functions use in
 * various places.
 */
namespace Util {

PIMCOMMON_EXPORT OrgKdeAkonadiImapSettingsInterface *createImapSettingsInterface( const QString &ident );
PIMCOMMON_EXPORT void saveTextAs( const QString &text, const QString &filter, QWidget *parent, const KUrl &url = KUrl(), const QString &caption = QString());
PIMCOMMON_EXPORT bool saveToFile( const QString &filename, const QString &text );
PIMCOMMON_EXPORT QString loadToFile(const QString &filter, QWidget *parent, const KUrl &url = KUrl(), const QString &caption = QString());
PIMCOMMON_EXPORT bool isImapResource(const QString &identifier);
}

}

#endif
