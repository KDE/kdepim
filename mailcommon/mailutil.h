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
#ifndef MAILCOMMON_UTIL_H
#define MAILCOMMON_UTIL_H

#include "mailcommon_export.h"

#include <akonadi/agentinstance.h>

class KUrl;
class KJob;

class QString;

class OrgKdeAkonadiImapSettingsInterface;

namespace Akonadi {
  class Collection;
  class Item;
}


#define IMAP_RESOURCE_IDENTIFIER "akonadi_imap_resource"
#define POP3_RESOURCE_IDENTIFIER "akonadi_pop3_resource"
namespace MailCommon
{

  class Kernel;

    /**
     * The Util namespace contains a collection of helper functions use in
     * various places.
     */
namespace Util {

    MAILCOMMON_EXPORT OrgKdeAkonadiImapSettingsInterface *createImapSettingsInterface( const QString &ident );

    MAILCOMMON_EXPORT bool isVirtualCollection(const Akonadi::Collection & col);

    MAILCOMMON_EXPORT QString fullCollectionPath( const Akonadi::Collection& collection );

    MAILCOMMON_EXPORT void showJobErrorMessage( KJob *job );

    MAILCOMMON_EXPORT Akonadi::AgentInstance::List agentInstances();

    MAILCOMMON_EXPORT bool createTodoFromMail( const Akonadi::Item &mailItem );

    /**
     * Returns the identity of the folder that contains the given
     *  Akonadi::Item.
     */
    MAILCOMMON_EXPORT uint folderIdentity( const Akonadi::Item& item );

}
}

#endif
