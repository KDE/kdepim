/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "pluckerpart.h"
#include "kspluckerconfigwidget.h"
#include "pluckerconfig.h"
#include "pluckerfilehandle.h"
#include "pluckerprocesshandler.h"

#include <unknownsyncee.h>

#include <profile.h>
#include <core.h>
#include <konnectorview.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <ktempdir.h>

#include <kparts/genericfactory.h>

#include <qapplication.h>
#include <qdir.h>
#include <qeventloop.h>
#include <qlayout.h>
#include <qtextedit.h>

typedef KParts::GenericFactory<KSPlucker::PluckerPart> PluckerPartFactory;
K_EXPORT_COMPONENT_FACTORY( libksync_pluckerpart, PluckerPartFactory )


namespace KSPlucker {

PluckerPart::PluckerPart( QWidget* parent, const char* name,
                          QObject* , const char* ,
                          const QStringList& )
  : DCOPObject("PluckerInterface"),
    KSync::ActionPart( parent, name ),
    m_widget( 0 ), m_config( 0 ), m_edit( 0 ), m_view( 0 ),
    m_temp( 0 )
{
  m_pixmap = KGlobal::iconLoader()->loadIcon( "knode",
                                              KIcon::Desktop, 48 );

  connectDoneSync(SLOT(slotCleanUp()));
  connectProfileChanged(SLOT(slotProfileChanged(const Profile&)));
}

PluckerPart::~PluckerPart()
{
  PluckerConfig *conf = PluckerConfig::self();
  conf->load( core()->currentProfile().uid() );
  kdDebug() << "Selected Konnectors are " << m_view->selectedKonnectorsList() << endl;
  conf->setKonnectorIds( m_view->selectedKonnectorsList() );
  conf->save( core()->currentProfile().uid() );
}

KAboutData *PluckerPart::createAboutData()
{
  return new KAboutData( "Plucker Part",
                         I18N_NOOP("Plucker Converter"),
                         "0.0" );

}

QString PluckerPart::type()const
{
  return QString::fromLatin1( "PluckerPart" );
}

QString PluckerPart::title()const
{
  return i18n( "Plucker" );
}

QString PluckerPart::description()const
{
  return i18n( "Pluck Websites and News-Feeds" );
}

bool PluckerPart::hasGui()const
{
  return true;
}

QPixmap *PluckerPart::pixmap()
{
  return &m_pixmap;
}

QString PluckerPart::iconName()const
{
  return QString::fromLatin1("knode");
}

QWidget *PluckerPart::widget()
{
  /*
   * Create a KonnectorView so the user can choose which
   * Konnectors should get a Plucker Upload
   * And then we present the log of the JPlucker Output
   */
  if ( !m_widget ) {
    m_widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(m_widget);

    m_view = new KSync::KonnectorView( m_widget , "pluckers view" );
    lay->addWidget( m_view );

    m_edit = new QTextEdit( m_widget );
    m_edit->setReadOnly( true );
    lay->addWidget( m_edit );
  }

  return m_widget;
}

bool PluckerPart::needsKonnectorWrite()const
{
  return true;
}

/*
 * Start the Action. We will start the
 * conversion and then do uploading
 * via KSync::UnknownSyncee
 */
void PluckerPart::executeAction()
{
  /*
   * Load and Safe Configuration
   */
  PluckerConfig *conf = PluckerConfig::self();
  conf->load( core()->currentProfile().uid() );
  QStringList lst = conf->pluckerFiles();
  conf->setKonnectorIds( m_view->selectedKonnectorsList() );
  conf->save( core()->currentProfile().uid() );


  m_temp = new KTempDir();
  PluckerProcessHandler *handler = new PluckerProcessHandler( PluckerProcessHandler::Convert,
                                                false, lst, m_temp->name(), this );

  connect(handler, SIGNAL(sigProgress(const QString&)),
          m_edit, SLOT(append(const QString&)));
  connect(handler, SIGNAL(sigFinished(PluckerProcessHandler*)),
          this, SLOT(slotFinished(PluckerProcessHandler*)));

  handler->run();
  /*
   * some hack due the synchronus of syncing!
   * this will be set to true in slotFinished.
   * We will allow SocketNotifier so that
   * Repainting is allowed and Konnectors can communicate
   *
   * #FIXME make it async soon
   */
  m_done = false;

  while ( !m_done )
    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

  /*
   * Now let us create the Syncees
   */
  QDir *dir = m_temp->qDir();
  lst = dir->entryList();

  lst.remove( "." ); lst.remove( ".." );
  delete dir;

  KSync::Konnector::List list = m_view->selectedKonnectors();


  for (KSync::Konnector::List::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
    KSync::UnknownSyncee *sync = new KSync::UnknownSyncee();
    for(QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
      sync->addEntry( new KSync::UnknownSyncEntry(m_temp->name()+"/"+*it,
                                                  QString::null, sync));
    (*it2)->appendSyncee( sync );
  }


}

bool PluckerPart::configIsVisible()const
{
  return true;
}

QWidget* PluckerPart::configWidget()
{
  KSPlucker::PluckerConfig::self()->load( core()->currentProfile().uid() );

  m_config = new KSPluckerConfigWidget();
  m_config->readConfig();


  return m_config;
}

void PluckerPart::addPluckerUrl( KURL url)
{
  KSPlucker::PluckerFileHandle::addFile( url, core()->currentProfile().uid() );
}

void PluckerPart::addPluckerFeed( KURL url )
{
  KSPlucker::PluckerFileHandle::addFile( url, core()->currentProfile().uid(),
                                         false );
}

void PluckerPart::slotConfigOk()
{
  /*
   * Safe the Config Data to the current profile
   */
  if ( m_config ) {
    m_config->slotConfigOk();
    PluckerConfig::self()->save( core()->currentProfile().uid() );
  }

  m_config = 0;
}

/*
 * Clean Up the intermediate pluckers files
 * and remove the dir
 */
void PluckerPart::slotCleanUp()
{
  kdDebug() << "Cleaning up " << endl << endl << endl;
  if ( m_temp )
    m_temp->unlink();

  delete m_temp;
  m_temp = 0l;
}

void PluckerPart::slotFinished( PluckerProcessHandler* handle )
{
  handle->deleteLater();
  m_done = true;
}

void PluckerPart::slotProfileChanged(const KSync::Profile& )
{
  /*
   * Apply settings to the KonnectorView
   */
  PluckerConfig* conf = PluckerConfig::self();
  conf->load( core()->currentProfile().uid() );
  m_view->setSelectedKonnectors( conf->konnectorIds() );
}

}

#include "pluckerpart.moc"
