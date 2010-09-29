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


#include "mailutil.h"
#include "imapsettings.h"
#include "mailkernel.h"

#include "messagecore/stringutil.h"
#include "messagecomposer/messagehelper.h"



#include <kmime/kmime_message.h>
#include <kpimutils/email.h>
#include <kimap/loginjob.h>
#include <mailtransport/transport.h>
#include <Akonadi/AgentManager>
#include <Akonadi/EntityTreeModel>
#include <akonadi/entitymimetypefiltermodel.h>

#include <KStandardDirs>
#include <kascii.h>
#include <KCharsets>
#include <KJob>
#include <kio/jobuidelegate.h>


#include <stdlib.h>

#include "foldercollection.h"
#include <KDBusServiceStarter>


OrgKdeAkonadiImapSettingsInterface *MailCommon::Util::createImapSettingsInterface( const QString &ident )
{
  return new OrgKdeAkonadiImapSettingsInterface("org.freedesktop.Akonadi.Resource." + ident, "/Settings", QDBusConnection::sessionBus() );
}


bool MailCommon::Util::isVirtualCollection(const Akonadi::Collection & collection)
{
  return ( collection.resource() == QLatin1String( "akonadi_nepomuktag_resource" ) || collection.resource() == QLatin1String( "akonadi_search_resource" ) );

}

QString MailCommon::Util::fullCollectionPath( const Akonadi::Collection& collection )
{
  QString fullPath;
  QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection( KernelIf->collectionModel(), collection );
  if ( !idx.isValid() )
    return fullPath;
  fullPath = idx.data().toString();
  idx = idx.parent();
  while ( idx != QModelIndex() ) {
    fullPath = idx.data().toString() + '/' + fullPath;
    idx = idx.parent();
  }
  return fullPath;
}

void MailCommon::Util::showJobErrorMessage( KJob *job )
{
  if ( job->error() ) {
    if ( static_cast<KIO::Job*>( job )->ui() )
      static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
    else
      kDebug()<<" job->errorString() :"<<job->errorString();
  }
}

Akonadi::AgentInstance::List MailCommon::Util::agentInstances()
{
  Akonadi::AgentInstance::List relevantInstances;
  foreach ( const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances() ) {
    if ( instance.type().mimeTypes().contains( KMime::Message::mimeType() ) &&
         instance.type().capabilities().contains( "Resource" ) &&
         !instance.type().capabilities().contains( "Virtual" ) ) {
      relevantInstances << instance;
    }
  }
  return relevantInstances;
}

void MailCommon::Util::ensureKorganizerRunning( bool switchTo )
{
  QString error;
  QString dbusService;
  int result = KDBusServiceStarter::self()->findServiceFor( "DBUS/Organizer", QString(),
                                                            &error, &dbusService );
  if ( result == 0 ) {
    // OK, so korganizer (or kontact) is running. Now ensure the object we want is loaded.
    QDBusInterface iface( "org.kde.korganizer", "/MainApplication",
                          "org.kde.KUniqueApplication" );
    if ( iface.isValid() ) {
      if ( switchTo ) {
        iface.call( "newInstance" ); // activate korganizer window
      }
      QDBusInterface pimIface( "org.kde.korganizer", "/korganizer_PimApplication",
                                "org.kde.KUniqueApplication" );
      QDBusReply<bool> r = pimIface.call( "load" );
      if ( !r.isValid() || !r.value() ) {
        kWarning() << "Loading korganizer failed: " << pimIface.lastError().message();
      }
    } else {
      kWarning() << "Couldn't obtain korganizer D-Bus interface" << iface.lastError().message();
    }

    // We don't do anything with it, we just need it to be running so that it handles
    // the incoming directory.
  } else {
    kWarning() << "Couldn't start DBUS/Organizer:" << dbusService << error;
  }
}
