// -*- mode: C++; c-file-style: "gnu" -*-
// kmfiltermgr.cpp

#include "filtermanager.h"

#include "filterimporterexporter.h"
#include "filterlog.h"
#include "mailfilter.h"
#include "mailkernel.h"
#include "messageproperty.h"

#include <akonadi/agentmanager.h>
#include <akonadi/changerecorder.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmovejob.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmime/kmime_message.h>
#include <libkdepim/progressmanager.h>
#include <libkdepim/broadcaststatus.h>

#include <QtCore/QTimer>

// other headers
#include <boost/bind.hpp>
#include <algorithm>
#include <assert.h>
#include <errno.h>

using namespace MailCommon;

//-----------------------------------------------------------------------------
FilterManager::FilterManager()
  : mShowLater( false ),
    mRequiresBody( false )
{
  tryToMonitorCollection();

  mChangeRecorder = new Akonadi::ChangeRecorder( this );
  mChangeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );
  mChangeRecorder->setChangeRecordingEnabled( false );
  mChangeRecorder->fetchCollection( true );
  mChangeRecorder->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );

  connect( mChangeRecorder, SIGNAL(itemAdded(Akonadi::Item,Akonadi::Collection)),
           SLOT(itemAdded(Akonadi::Item,Akonadi::Collection)) );

  tryToFilterInboxOnStartup();
}

void FilterManager::tryToFilterInboxOnStartup()
{
  if ( !Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Inbox ).isValid() ) {
    QTimer::singleShot( 0, this, SLOT( tryToFilterInboxOnStartup() ) );
    return;
  }

  //Fetch the collection on startup and apply filters on unread messages in the inbox.
  //This is done to filter inbox even if messages were downloaded while kmail/kmail-mobile
  //was not running. Once filtering goes into its own agent, this code can be removed,
  //as at that time the inbox is always monitored.
  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this );
  job->fetchScope().setContentMimeTypes( QStringList() << KMime::Message::mimeType() );
  job->fetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::Parent );
  connect( job, SIGNAL( collectionsReceived( const Akonadi::Collection::List& ) ),
           this, SLOT( slotInitialCollectionsFetched( const Akonadi::Collection::List& ) ) );
}

void FilterManager::tryToMonitorCollection()
{
  if ( KernelIf->folderCollectionMonitor() ) {
    connect( KernelIf->folderCollectionMonitor(), SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
           this, SLOT( slotFolderRemoved( const Akonadi::Collection & ) ) );
  } else {
    QTimer::singleShot( 0, this, SLOT( tryToMonitorCollection() ) );
  }
}

void FilterManager::slotInitialCollectionsFetched( const Akonadi::Collection::List& collections )
{
  Q_FOREACH( Akonadi::Collection collection, collections ) {
    if ( CommonKernel->folderIsInbox( collection ) ) {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection, this );
        job->fetchScope().fetchAllAttributes();
        connect( job, SIGNAL(itemsReceived(Akonadi::Item::List)), this, SLOT(slotInitialItemsFetched(Akonadi::Item::List)) );
    }
  }
}

void FilterManager::slotInitialItemsFetched(const Akonadi::Item::List& items)
{
  Akonadi::Item::List unreadItems;

  Q_FOREACH( Akonadi::Item item, items ) {
   Akonadi::MessageStatus status;
   status.setStatusFromFlags( item.flags() );
   if ( !status.isRead() )
     unreadItems << item;
  }

  applyFilters( unreadItems );
}


//-----------------------------------------------------------------------------
FilterManager::~FilterManager()
{
  writeConfig( false );
  clear();
}

void FilterManager::clear()
{
  qDeleteAll( mFilters );
  mFilters.clear();
}

//-----------------------------------------------------------------------------
void FilterManager::readConfig(void)
{
  beginUpdate();
  KSharedConfig::Ptr config = KernelIf->config();
  clear();

  mFilters = FilterImporterExporter::readFiltersFromConfig( config, false );
  endUpdate();
}

//-----------------------------------------------------------------------------
void FilterManager::writeConfig(bool withSync)
{
  KSharedConfig::Ptr config = KernelIf->config();

  // Now, write out the new stuff:
  FilterImporterExporter::writeFiltersToConfig( mFilters, config, false );
  KConfigGroup group = config->group( "General" );

  if ( withSync ) {
    group.sync();
  }
}

bool FilterManager::beginFiltering( const Akonadi::Item &item ) const
{
  //KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  //kDebug() << "filtering" << msg->subject()->asUnicodeString();

  if (MessageProperty::filtering( item ))
    return false;
  MessageProperty::setFiltering( item, true );
  if ( FilterLog::instance()->isLogging() ) {
    FilterLog::instance()->addSeparator();
  }
  return true;
}

void FilterManager::endFiltering( const Akonadi::Item &item ) const
{
  MessageProperty::setFiltering( item, false );
}

int FilterManager::process( const Akonadi::Item &item, const MailFilter * filter )
{
  bool stopIt = false;
  int result = 1;

  if ( !filter || !item.hasPayload<KMime::Message::Ptr>() )
    return 1;

  if ( isMatching( item, filter ) ) {
    // do the actual filtering stuff
    if ( !beginFiltering( item ) ) {
      return 1;
    }
    if ( filter->execActions( item, stopIt ) == MailFilter::CriticalError ) {
      return 2;
    }

    const Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );
    endFiltering( item );
    if ( targetFolder.isValid() ) {
      new Akonadi::ItemMoveJob( item, targetFolder, this ); // TODO: check result
      result = 0;
    }
  } else {
    result = 1;
  }
  return result;
}

int FilterManager::process( const Akonadi::Item &item, FilterSet set,
                            bool account, const QString& accountId ) {


  if ( set == NoSet ) {
    kDebug() << "FilterManager: process() called with not filter set selected";
    emit itemNotMoved( item );
    return 1;
  }
  bool stopIt = false;

  if ( !beginFiltering( item ) ) {
    emit itemNotMoved( item );
    return 1;
  }
  for ( QList<MailFilter*>::const_iterator it = mFilters.constBegin();
        !stopIt && it != mFilters.constEnd() ; ++it ) {

    if ( ( ( (set&Inbound) && (*it)->applyOnInbound() ) &&
         ( !account ||
             ( account && (*it)->applyOnAccount( accountId ) ) ) ) ||
         ( (set&Outbound)  && (*it)->applyOnOutbound() ) ||
         ( (set&BeforeOutbound)  && (*it)->applyBeforeOutbound() ) ||
         ( (set&Explicit) && (*it)->applyOnExplicit() ) ) {
        // filter is applicable

      if ( isMatching( item, (*it) ) ) {
        // execute actions:
        if ( (*it)->execActions(item, stopIt) == MailFilter::CriticalError ) {
          return 2;
        }
      }
    }
  }

  Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );

  endFiltering( item );

  if ( targetFolder.isValid() ) {
    new Akonadi::ItemMoveJob( item, targetFolder, this ); // TODO: check result
    return 0;
  }
  emit itemNotMoved( item );
  return 1;
}

bool FilterManager::isMatching( const Akonadi::Item& item, const MailFilter * filter )
{
  bool result = false;
  if ( FilterLog::instance()->isLogging() ) {
    QString logText( i18n( "<b>Evaluating filter rules:</b> " ) );
    logText.append( filter->pattern()->asString() );
    FilterLog::instance()->add( logText, FilterLog::PatternDescription );
  }
  if ( filter->pattern()->matches( item ) ) {
    if ( FilterLog::instance()->isLogging() ) {
      FilterLog::instance()->add( i18n( "<b>Filter rules have matched.</b>" ),
                                  FilterLog::PatternResult );
    }
    result = true;
  }
  return result;
}

bool FilterManager::atLeastOneFilterAppliesTo( const QString& accountID ) const
{
  QList<MailFilter*>::const_iterator it = mFilters.constBegin();
  for ( ; it != mFilters.constEnd() ; ++it ) {
    if ( (*it)->applyOnAccount( accountID ) ) {
      return true;
    }
  }
  return false;
}

bool FilterManager::atLeastOneIncomingFilterAppliesTo( const QString& accountID ) const
{
  QList<MailFilter*>::const_iterator it = mFilters.constBegin();
  for ( ; it != mFilters.constEnd() ; ++it ) {
    if ( (*it)->applyOnInbound() && (*it)->applyOnAccount( accountID ) ) {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
void FilterManager::openDialog( QWidget *, bool checkForEmptyFilterList )
{
  FilterIf->openFilterDialog( false, checkForEmptyFilterList );
}


//-----------------------------------------------------------------------------
void FilterManager::createFilter( const QByteArray & field, const QString & value )
{
  openDialog( 0, false );
  FilterIf->createFilter( field, value );
}


//-----------------------------------------------------------------------------
const QString FilterManager::createUniqueName( const QString & name )
{
  QString uniqueName = name;
  int counter = 0;
  bool found = true;

  while ( found ) {
    found = false;
    for ( QList<MailFilter*>::const_iterator it = mFilters.constBegin();
          it != mFilters.constEnd(); ++it ) {
      if ( !( (*it)->name().compare( uniqueName ) ) ) {
        found = true;
        ++counter;
        uniqueName = name;
        uniqueName += QString( " (" ) + QString::number( counter )
                    + QString( ")" );
        break;
      }
    }
  }
  return uniqueName;
}


//-----------------------------------------------------------------------------
void FilterManager::appendFilters( const QList<MailFilter*> &filters,
                                 bool replaceIfNameExists )
{
  beginUpdate();
  if ( replaceIfNameExists ) {
    QList<MailFilter*>::const_iterator it1 = filters.constBegin();
    for ( ; it1 != filters.constEnd() ; ++it1 ) {
      for ( int i = 0; i < mFilters.count(); i++ ) {
        MailFilter *filter = mFilters[i];
        if ( (*it1)->name() == filter->name() ) {
          mFilters.removeAll( filter );
          i = 0;
        }
      }
    }
  }
  mFilters += filters;
  writeConfig( true );
  endUpdate();
}

void FilterManager::setFilters( const QList<MailFilter*> &filters )
{
  beginUpdate();
  clear();
  mFilters = filters;
  writeConfig( true );
  endUpdate();
}

void FilterManager::removeFilter( MailFilter* filter )
{
  beginUpdate();
  mFilters.removeAll( filter );
  writeConfig( true );
  endUpdate();
}

void FilterManager::slotFolderRemoved( const Akonadi::Collection & aFolder )
{
  folderRemoved( aFolder, Akonadi::Collection() );
}

//-----------------------------------------------------------------------------
bool FilterManager::folderRemoved(const Akonadi::Collection & aFolder, const Akonadi::Collection & aNewFolder)
{
  bool rem = false;
  QList<MailFilter*>::const_iterator it = mFilters.constBegin();
  for ( ; it != mFilters.constEnd() ; ++it )
    if ( (*it)->folderRemoved(aFolder, aNewFolder) )
      rem = true;

  return rem;
}


//-----------------------------------------------------------------------------
#ifndef NDEBUG
void FilterManager::dump(void) const
{
  QList<MailFilter*>::const_iterator it = mFilters.constBegin();
  for ( ; it != mFilters.constEnd() ; ++it ) {
    kDebug() << (*it)->asString();
  }
}
#endif

//-----------------------------------------------------------------------------
void FilterManager::endUpdate(void)
{
  mRequiresBody = std::find_if( mFilters.constBegin(), mFilters.constEnd(),
      boost::bind( &MailFilter::requiresBody, _1 ) ) != mFilters.constEnd();

  emit filterListUpdated();
}

void FilterManager::itemAdded(const Akonadi::Item& item, const Akonadi::Collection &collection)
{
  if ( CommonKernel->folderIsInbox( collection ) ) {
    if ( mRequiresBody ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );
      job->fetchScope().fetchFullPayload( true );
      job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
      job->setProperty( "resource", collection.resource() );
      connect( job, SIGNAL( result( KJob* ) ), SLOT( itemAddedFetchResult( KJob* ) ) );
    } else {
      process( item, Inbound, true, collection.resource() );
    }
  }
}

void FilterManager::itemAddedFetchResult( KJob *job )
{
  if ( job->error() ) {
    kError() << job->error() << job->errorString();
    return;
  }

  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  Q_ASSERT( fetchJob );

  if ( fetchJob->items().count() == 1 ) {
    const Akonadi::Item item = fetchJob->items().first();
    process( item, Inbound, true, fetchJob->property( "resource" ).toString() );
  }
}

void FilterManager::applyFilters(const QList< Akonadi::Item >& selectedMessages)
{
  const int msgCountToFilter = selectedMessages.size();

  KPIM::ProgressItem *progressItem = KPIM::ProgressManager::createProgressItem (
      "filter" + KPIM::ProgressManager::getUniqueID(),
      i18n( "Filtering messages" )
    );

  progressItem->setTotalItems( msgCountToFilter );
  Akonadi::ItemFetchJob *itemFetchJob = new Akonadi::ItemFetchJob( selectedMessages, this );
  itemFetchJob->fetchScope().fetchFullPayload( true );
  itemFetchJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  itemFetchJob->setProperty( "progressItem", QVariant::fromValue( static_cast<QObject*>( progressItem ) ) );
  connect( itemFetchJob, SIGNAL( itemsReceived( Akonadi::Item::List ) ), SLOT( slotItemsFetchedForFilter( Akonadi::Item::List ) ) );
  connect( itemFetchJob, SIGNAL( result(KJob *) ), SLOT( itemsFetchJobForFilterDone( KJob* ) ) );
}


void FilterManager::itemsFetchJobForFilterDone( KJob *job )
{
  if ( job->error() ) {
    kDebug() << job->errorString();
  }
  KPIM::BroadcastStatus::instance()->setStatusMsg( QString() );

  KPIM::ProgressItem *progressItem = qobject_cast<KPIM::ProgressItem*>( job->property( "progressItem" ).value<QObject*>() );
  progressItem->setComplete();
}

void FilterManager::slotItemsFetchedForFilter( const Akonadi::Item::List &items )
{

  KPIM::ProgressItem* progressItem;
  if( sender()->property( "progressItem" ).value< QObject*>() ) {
    progressItem = qobject_cast<KPIM::ProgressItem*>( sender()->property( "progressItem" ).value<QObject*>() );
  } else {
    progressItem = 0;
    kWarning() << "Got invalid progress item for slotItemsFetchedFromFilter! Something went wrong...";
  }

  foreach ( const Akonadi::Item &item, items ) {
    if( progressItem ) {
      progressItem->incCompletedItems();
      if ( progressItem->totalItems() - progressItem->completedItems() < 10 ||
        !( progressItem->completedItems() % 10 ) ||
          progressItem->completedItems() <= 10 )
      {
        progressItem->updateProgress();
        const QString statusMsg = i18n( "Filtering message %1 of %2", progressItem->completedItems(),
                                        progressItem->totalItems() );
        KPIM::BroadcastStatus::instance()->setStatusMsg( statusMsg );
      }
    }
    int filterResult = process(item, FilterManager::Explicit);
    if (filterResult == 2)
    {
      // something went horribly wrong (out of space?)
      CommonKernel->emergencyExit( i18n("Unable to process messages: " ) + QString::fromLocal8Bit(strerror(errno)));
    }
  }
}

#include "filtermanager.moc"
