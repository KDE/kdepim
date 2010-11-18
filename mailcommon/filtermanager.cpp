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
#include <akonadi/kmime/messageparts.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmime/kmime_message.h>
#include <libkdepim/progressmanager.h>
#include <libkdepim/broadcaststatus.h>

#include <QtCore/QTimer>

// other headers
#include <algorithm>
#include <assert.h>
#include <boost/bind.hpp>
#include <errno.h>

using namespace MailCommon;

class FilterManager::Private
{
  public:
    Private( FilterManager *qq )
      : q( qq ),
        mRequiresBody( false )
    {
    }

    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void itemAddedFetchResult( KJob *job );
    void itemsFetchJobForFilterDone( KJob *job );
    void slotItemsFetchedForFilter( const Akonadi::Item::List &items );
    void slotInitialCollectionsFetched( const Akonadi::Collection::List &collections );
    void slotInitialItemsFetched( const Akonadi::Item::List &items );
    void slotFolderRemoved( const Akonadi::Collection &folder );
    void tryToMonitorCollection();
    void tryToFilterInboxOnStartup();

    bool isMatching( const Akonadi::Item &item, const MailFilter *filter );
    bool beginFiltering( const Akonadi::Item &item ) const;
    void endFiltering( const Akonadi::Item &item ) const;
    bool atLeastOneFilterAppliesTo( const QString &accountId ) const;
    bool atLeastOneIncomingFilterAppliesTo( const QString &accountId ) const;
    bool folderRemoved( const Akonadi::Collection &folder, const Akonadi::Collection &newFolder );

    FilterManager *q;
    QList<MailFilter *> mFilters;
    Akonadi::ChangeRecorder *mChangeRecorder;
    bool mRequiresBody;
};

void FilterManager::Private::tryToFilterInboxOnStartup()
{
  if ( !Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Inbox ).isValid() ) {
    QTimer::singleShot( 0, q, SLOT( tryToFilterInboxOnStartup() ) );
    return;
  }

  // Fetch the collection on startup and apply filters on unread messages in the inbox.
  // This is done to filter inbox even if messages were downloaded while kmail/kmail-mobile
  // was not running. Once filtering goes into its own agent, this code can be removed,
  // as at that time the inbox is always monitored.
  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, q );
  job->fetchScope().setContentMimeTypes( QStringList() << KMime::Message::mimeType() );
  job->fetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::Parent );
  q->connect( job, SIGNAL( collectionsReceived( const Akonadi::Collection::List& ) ),
              q, SLOT( slotInitialCollectionsFetched( const Akonadi::Collection::List& ) ) );
}

void FilterManager::Private::tryToMonitorCollection()
{
  if ( KernelIf->folderCollectionMonitor() ) {
    q->connect( KernelIf->folderCollectionMonitor(), SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
                q, SLOT( slotFolderRemoved( const Akonadi::Collection & ) ) );
  } else {
    QTimer::singleShot( 0, q, SLOT( tryToMonitorCollection() ) );
  }
}

void FilterManager::Private::slotInitialCollectionsFetched( const Akonadi::Collection::List &collections )
{
  foreach ( const Akonadi::Collection &collection, collections ) {
    if ( CommonKernel->folderIsInbox( collection ) ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection, q );
      job->fetchScope().fetchAllAttributes();
      q->connect( job, SIGNAL( itemsReceived( const Akonadi::Item::List& ) ),
                  q, SLOT( slotInitialItemsFetched( const Akonadi::Item::List& ) ) );
    }
  }
}

void FilterManager::Private::slotInitialItemsFetched( const Akonadi::Item::List &items )
{
  Akonadi::Item::List unreadItems;

  foreach ( const Akonadi::Item &item, items ) {
   Akonadi::MessageStatus status;
   status.setStatusFromFlags( item.flags() );
   if ( !status.isRead() )
     unreadItems << item;
  }

  q->applyFilters( unreadItems );
}

void FilterManager::Private::slotItemsFetchedForFilter( const Akonadi::Item::List &items )
{
  KPIM::ProgressItem* progressItem;
  if ( q->sender()->property( "progressItem" ).value< QObject*>() ) {
    progressItem = qobject_cast<KPIM::ProgressItem*>( q->sender()->property( "progressItem" ).value<QObject*>() );
  } else {
    progressItem = 0;
    kWarning() << "Got invalid progress item for slotItemsFetchedFromFilter! Something went wrong...";
  }

  foreach ( const Akonadi::Item &item, items ) {
    if ( progressItem ) {
      progressItem->incCompletedItems();

      const bool lessThanTenLeft = (progressItem->totalItems() - progressItem->completedItems() < 10);
      const bool aTenthItem = !(progressItem->completedItems() % 10);
      const bool lessThanTenAtAll = (progressItem->completedItems() <= 10);

      if ( lessThanTenLeft || aTenthItem || lessThanTenAtAll ) {
        progressItem->updateProgress();
        const QString statusMsg = i18n( "Filtering message %1 of %2", progressItem->completedItems(),
                                        progressItem->totalItems() );
        KPIM::BroadcastStatus::instance()->setStatusMsg( statusMsg );
      }
    }

    const int filterResult = q->process( item, FilterManager::Explicit );
    if ( filterResult == 2 ) {
      // something went horribly wrong (out of space?)
      CommonKernel->emergencyExit( i18n( "Unable to process messages: " ) + QString::fromLocal8Bit( strerror( errno ) ) );
    }
  }
}

void FilterManager::Private::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  if ( CommonKernel->folderIsInbox( collection ) ) {
    if ( mRequiresBody ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, q );
      job->fetchScope().fetchFullPayload( true );
      job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
      job->setProperty( "resource", collection.resource() );
      q->connect( job, SIGNAL( result( KJob* ) ), SLOT( itemAddedFetchResult( KJob* ) ) );
    } else {
      // the monitor has fetched all headers for this message already
      q->process( item, Inbound, true, collection.resource() );
    }
  }
}

void FilterManager::Private::itemAddedFetchResult( KJob *job )
{
  if ( job->error() ) {
    kError() << job->error() << job->errorString();
    return;
  }

  const Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  Q_ASSERT( fetchJob );

  if ( fetchJob->items().count() == 1 ) {
    const Akonadi::Item item = fetchJob->items().first();
    q->process( item, Inbound, true, fetchJob->property( "resource" ).toString() );
  }
}

void FilterManager::Private::itemsFetchJobForFilterDone( KJob *job )
{
  if ( job->error() ) {
    kDebug() << job->errorString();
  }
  KPIM::BroadcastStatus::instance()->setStatusMsg( QString() );

  KPIM::ProgressItem *progressItem = qobject_cast<KPIM::ProgressItem*>( job->property( "progressItem" ).value<QObject*>() );
  progressItem->setComplete();
}

bool FilterManager::Private::isMatching( const Akonadi::Item &item, const MailFilter *filter )
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

bool FilterManager::Private::beginFiltering( const Akonadi::Item &item ) const
{
  if ( MessageProperty::filtering( item ) )
    return false;

  MessageProperty::setFiltering( item, true );
  if ( FilterLog::instance()->isLogging() ) {
    FilterLog::instance()->addSeparator();
  }

  return true;
}

void FilterManager::Private::endFiltering( const Akonadi::Item &item ) const
{
  MessageProperty::setFiltering( item, false );
}

bool FilterManager::Private::atLeastOneFilterAppliesTo( const QString &accountId ) const
{
  foreach ( const MailFilter *filter, mFilters ) {
    if ( filter->applyOnAccount( accountId ) ) {
      return true;
    }
  }

  return false;
}

bool FilterManager::Private::atLeastOneIncomingFilterAppliesTo( const QString &accountId ) const
{
  foreach ( const MailFilter *filter, mFilters ) {
    if ( filter->applyOnInbound() && filter->applyOnAccount( accountId ) ) {
      return true;
    }
  }

  return false;
}

void FilterManager::Private::slotFolderRemoved( const Akonadi::Collection &folder )
{
  folderRemoved( folder, Akonadi::Collection() );
}

bool FilterManager::Private::folderRemoved( const Akonadi::Collection &folder, const Akonadi::Collection &newFolder )
{
  bool removed = false;

  foreach ( MailFilter *filter, mFilters ) {
    if ( filter->folderRemoved( folder, newFolder ) )
      removed = true;
  }

  return removed;
}


FilterManager::FilterManager( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->tryToMonitorCollection();

  d->mChangeRecorder = new Akonadi::ChangeRecorder( this );
  d->mChangeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );
  d->mChangeRecorder->setChangeRecordingEnabled( false );
  d->mChangeRecorder->fetchCollection( true );
  d->mChangeRecorder->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  d->mChangeRecorder->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Header, true );

  connect( d->mChangeRecorder, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
           SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );

  d->tryToFilterInboxOnStartup();
}

FilterManager::~FilterManager()
{
  writeConfig( false );
  clear();

  delete d;
}

void FilterManager::clear()
{
  qDeleteAll( d->mFilters );
  d->mFilters.clear();
}

void FilterManager::readConfig()
{
  beginUpdate();
  KSharedConfig::Ptr config = KernelIf->config();
  clear();

  d->mFilters = FilterImporterExporter::readFiltersFromConfig( config );
  endUpdate();
}

void FilterManager::writeConfig( bool withSync ) const
{
  KSharedConfig::Ptr config = KernelIf->config();

  // Now, write out the new stuff:
  FilterImporterExporter::writeFiltersToConfig( d->mFilters, config );
  KConfigGroup group = config->group( "General" );

  if ( withSync ) {
    group.sync();
  }
}

int FilterManager::process( const Akonadi::Item &item, const MailFilter *filter )
{
  bool stopIt = false;
  int result = 1;

  if ( !filter || !item.hasPayload<KMime::Message::Ptr>() )
    return 1;

  if ( d->isMatching( item, filter ) ) {
    // do the actual filtering stuff
    if ( !d->beginFiltering( item ) ) {
      return 1;
    }

    if ( filter->execActions( item, stopIt ) == MailFilter::CriticalError ) {
      return 2;
    }

    const Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );
    d->endFiltering( item );
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
                            bool account, const QString &accountId )
{
  if ( set == NoSet ) {
    kDebug() << "FilterManager: process() called with not filter set selected";
    emit itemNotMoved( item );
    return 1;
  }

  bool stopIt = false;

  if ( !d->beginFiltering( item ) ) {
    emit itemNotMoved( item );
    return 1;
  }

  for ( QList<MailFilter*>::const_iterator it = d->mFilters.constBegin();
        !stopIt && it != d->mFilters.constEnd() ; ++it ) {

    const bool inboundOk = ((set & Inbound) && (*it)->applyOnInbound());
    const bool outboundOk = ((set & Outbound) && (*it)->applyOnOutbound());
    const bool beforeOutboundOk = ((set & BeforeOutbound) && (*it)->applyBeforeOutbound());
    const bool explicitOk = ((set & Explicit) && (*it)->applyOnExplicit());
    const bool accountOk = (!account || (account && (*it)->applyOnAccount( accountId )));

    if ( (inboundOk && accountOk) || outboundOk || beforeOutboundOk || explicitOk ) {
        // filter is applicable

      if ( d->isMatching( item, *it ) ) {
        // execute actions:
        if ( (*it)->execActions( item, stopIt ) == MailFilter::CriticalError ) {
          return 2;
        }
      }
    }
  }

  Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );

  d->endFiltering( item );

  if ( targetFolder.isValid() ) {
    new Akonadi::ItemMoveJob( item, targetFolder, this ); // TODO: check result
    return 0;
  }

  emit itemNotMoved( item );
  return 1;
}

void FilterManager::openDialog( bool checkForEmptyFilterList )
{
  FilterIf->openFilterDialog( checkForEmptyFilterList );
}

void FilterManager::createFilter( const QByteArray &field, const QString &value )
{
  openDialog( false );
  FilterIf->createFilter( field, value );
}

QString FilterManager::createUniqueName( const QString &name ) const
{
  QString uniqueName = name;

  int counter = 0;
  bool found = true;

  while ( found ) {
    found = false;
    foreach ( const MailFilter *filter, d->mFilters ) {
      if ( !filter->name().compare( uniqueName ) ) {
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

void FilterManager::appendFilters( const QList<MailFilter*> &filters,
                                   bool replaceIfNameExists )
{
  beginUpdate();
  if ( replaceIfNameExists ) {
    foreach ( const MailFilter *newFilter, filters ) {
      for ( int i = 0; i < d->mFilters.count(); ++i ) {
        MailFilter *filter = d->mFilters[ i ];
        if ( newFilter->name() == filter->name() ) {
          d->mFilters.removeAll( filter );
          i = 0;
        }
      }
    }
  }

  d->mFilters += filters;

  writeConfig( true );
  endUpdate();
}

void FilterManager::setFilters( const QList<MailFilter*> &filters )
{
  beginUpdate();
  clear();
  d->mFilters = filters;
  writeConfig( true );
  endUpdate();
}

QList<MailFilter*> FilterManager::filters() const
{
  return d->mFilters;
}

void FilterManager::removeFilter( MailFilter *filter )
{
  beginUpdate();
  d->mFilters.removeAll( filter );
  writeConfig( true );
  endUpdate();
}

void FilterManager::beginUpdate()
{
}

void FilterManager::endUpdate()
{
  // check if at least one filter requires the message body
  d->mRequiresBody = std::find_if( d->mFilters.constBegin(), d->mFilters.constEnd(),
                                   boost::bind( &MailFilter::requiresBody, _1 ) ) != d->mFilters.constEnd();

  emit filterListUpdated();
}

#ifndef NDEBUG
void FilterManager::dump() const
{
  foreach ( const MailFilter *filter, d->mFilters ) {
    kDebug() << filter->asString();
  }
}
#endif

void FilterManager::applyFilters( const QList<Akonadi::Item> &selectedMessages )
{
  const int msgCountToFilter = selectedMessages.size();

  KPIM::ProgressItem *progressItem = KPIM::ProgressManager::createProgressItem(
      "filter" + KPIM::ProgressManager::getUniqueID(),
      i18n( "Filtering messages" )
    );

  progressItem->setTotalItems( msgCountToFilter );

  Akonadi::ItemFetchJob *itemFetchJob = new Akonadi::ItemFetchJob( selectedMessages, this );
  if ( d->mRequiresBody )
    itemFetchJob->fetchScope().fetchFullPayload( true );
  else
    itemFetchJob->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Header, true );
  itemFetchJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  itemFetchJob->setProperty( "progressItem", QVariant::fromValue( static_cast<QObject*>( progressItem ) ) );

  connect( itemFetchJob, SIGNAL( itemsReceived( const Akonadi::Item::List& ) ),
           this, SLOT( slotItemsFetchedForFilter( const Akonadi::Item::List& ) ) );
  connect( itemFetchJob, SIGNAL( result( KJob* ) ),
           SLOT( itemsFetchJobForFilterDone( KJob* ) ) );
}

#include "filtermanager.moc"
