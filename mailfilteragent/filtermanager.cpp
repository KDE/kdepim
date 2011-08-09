// -*- mode: C++; c-file-style: "gnu" -*-
// kmfiltermgr.cpp

#include "filtermanager.h"

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
#include <kglobal.h>
#include <klocale.h>
#include <kmime/kmime_message.h>
#include <libkdepim/progressmanager.h>
#include <libkdepim/broadcaststatus.h>
#include <mailcommon/filterimporterexporter.h>
#include <mailcommon/filterlog.h>
#include <mailcommon/mailfilter.h>
#include <mailcommon/messageproperty.h>

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
        mRequiresBody( false ), mInboundFiltersExist( false )
    {
    }

    void itemsFetchJobForFilterDone( KJob *job );
    void moveJobResult( KJob* );
    void slotItemsFetchedForFilter( const Akonadi::Item::List &items );

    bool isMatching( const Akonadi::Item &item, const MailCommon::MailFilter *filter );
    bool beginFiltering( const Akonadi::Item &item ) const;
    void endFiltering( const Akonadi::Item &item ) const;
    bool atLeastOneFilterAppliesTo( const QString &accountId ) const;
    bool atLeastOneIncomingFilterAppliesTo( const QString &accountId ) const;
/*
    bool folderRemoved( const Akonadi::Collection &folder, const Akonadi::Collection &newFolder );
*/
    FilterManager *q;
    QList<MailCommon::MailFilter *> mFilters;
    bool mRequiresBody;
    bool mInboundFiltersExist;
};

void FilterManager::Private::slotItemsFetchedForFilter( const Akonadi::Item::List &items )
{
  KPIM::ProgressItem* progressItem = 0;
  if ( q->sender()->property( "progressItem" ).value< QObject*>() ) {
    progressItem = qobject_cast<KPIM::ProgressItem*>( q->sender()->property( "progressItem" ).value<QObject*>() );
  } else {
    kWarning() << "Got invalid progress item for slotItemsFetchedFromFilter! Something went wrong...";
  }

  const FilterManager::FilterSet filterSet = static_cast<FilterManager::FilterSet>(q->sender()->property( "filterSet" ).toInt());

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

    const int filterResult = q->process( item, filterSet );
    if ( filterResult == 2 ) {
      // something went horribly wrong (out of space?)
      //CommonKernel->emergencyExit( i18n( "Unable to process messages: " ) + QString::fromLocal8Bit( strerror( errno ) ) );
    }
  }
}

void FilterManager::Private::itemsFetchJobForFilterDone( KJob *job )
{
  if ( job->error() ) {
    kError() << "Error while fetching items. " << job->error() << job->errorString();
  }
  KPIM::BroadcastStatus::instance()->setStatusMsg( QString() );

  KPIM::ProgressItem *progressItem = qobject_cast<KPIM::ProgressItem*>( job->property( "progressItem" ).value<QObject*>() );
  progressItem->setComplete();
}

void FilterManager::Private::moveJobResult( KJob *job )
{
  if ( job->error() ) {
    const Akonadi::ItemMoveJob *movejob = qobject_cast<Akonadi::ItemMoveJob*>( job );
    if( movejob ) {
	  kError() << "Error while moving items. "<< job->error() << job->errorString()<<" to destinationCollection.id() :"<< movejob->destinationCollection().id();
          
    } else {
          kError() << "Error while moving items. " << job->error() << job->errorString();
    }
    // TODO: kmail should tell the user that this failed
  }
}

bool FilterManager::Private::isMatching( const Akonadi::Item &item, const MailCommon::MailFilter *filter )
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
  foreach ( const MailCommon::MailFilter *filter, mFilters ) {
    if ( filter->applyOnAccount( accountId ) ) {
      return true;
    }
  }

  return false;
}

bool FilterManager::Private::atLeastOneIncomingFilterAppliesTo( const QString &accountId ) const
{
  foreach ( const MailCommon::MailFilter *filter, mFilters ) {
    if ( filter->applyOnInbound() && filter->applyOnAccount( accountId ) ) {
      return true;
    }
  }

  return false;
}

/*
void FilterManager::Private::slotFolderRemoved( const Akonadi::Collection &folder )
{
  folderRemoved( folder, Akonadi::Collection() );
}

bool FilterManager::Private::folderRemoved( const Akonadi::Collection &folder, const Akonadi::Collection &newFolder )
{
  bool removed = false;

  foreach ( MailCommon::MailFilter *filter, mFilters ) {
    if ( filter->folderRemoved( folder, newFolder ) )
      removed = true;
  }

  return removed;
}
*/

FilterManager::FilterManager( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
//  d->tryToMonitorCollection();

//  connect( this, SIGNAL(filterListUpdated()), SLOT(tryToFilterInboxOnStartup()) );
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
  //tokoe KSharedConfig::Ptr config = KernelIf->config();
  KSharedConfig::Ptr config = KGlobal::config();
  clear();

  d->mFilters = FilterImporterExporter::readFiltersFromConfig( config );
  endUpdate();
}

void FilterManager::writeConfig( bool withSync ) const
{
  //tokoe KSharedConfig::Ptr config = KernelIf->config();
  KSharedConfig::Ptr config = KGlobal::config();

  // Now, write out the new stuff:
  FilterImporterExporter::writeFiltersToConfig( d->mFilters, config );
  KConfigGroup group = config->group( "General" );

  if ( withSync ) {
    group.sync();
  }
}

int FilterManager::process( const Akonadi::Item &item, const MailCommon::MailFilter *filter )
{
  bool stopIt = false;
  int result = 1;

  if ( !filter || !item.hasPayload<KMime::Message::Ptr>() ) {
    kError() << "Filter is null or item doesn't have correct payload.";
    return 1;
  }

  if ( d->isMatching( item, filter ) ) {
    // do the actual filtering stuff
    if ( !d->beginFiltering( item ) ) {
      return 1;
    }

    if ( filter->execActions( item, stopIt ) == MailCommon::MailFilter::CriticalError ) {
      return 2;
    }

    const Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );
    d->endFiltering( item );
    if ( targetFolder.isValid() ) {
      Akonadi::ItemMoveJob *moveJob = new Akonadi::ItemMoveJob( item, targetFolder, this );
      connect( moveJob, SIGNAL(result(KJob*)), SLOT(moveJobResult(KJob*)) );
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

  for ( QList<MailCommon::MailFilter*>::const_iterator it = d->mFilters.constBegin();
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
        if ( (*it)->execActions( item, stopIt ) == MailCommon::MailFilter::CriticalError ) {
          return 2;
        }
      }
    }
  }

  Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );

  d->endFiltering( item );

  if ( targetFolder.isValid() ) {
    Akonadi::ItemMoveJob *moveJob = new Akonadi::ItemMoveJob( item, targetFolder, this );
    connect( moveJob, SIGNAL(result(KJob*)), SLOT(moveJobResult(KJob*)) );
    return 0;
  }

  emit itemNotMoved( item );
  return 1;
}

QString FilterManager::createUniqueName( const QString &name ) const
{
  QString uniqueName = name;

  int counter = 0;
  bool found = true;

  while ( found ) {
    found = false;
    foreach ( const MailCommon::MailFilter *filter, d->mFilters ) {
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

void FilterManager::appendFilters( const QList<MailCommon::MailFilter*> &filters,
                                   bool replaceIfNameExists )
{
  beginUpdate();
  if ( replaceIfNameExists ) {
    foreach ( const MailCommon::MailFilter *newFilter, filters ) {
      const int numberOfFilters = d->mFilters.count();
      for ( int i = 0; i < numberOfFilters; ++i ) {
        MailCommon::MailFilter *filter = d->mFilters.at( i );
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

void FilterManager::setFilters( const QList<MailCommon::MailFilter*> &filters )
{
  beginUpdate();
  clear();
  d->mFilters = filters;
  writeConfig( true );
  endUpdate();
}

QList<MailCommon::MailFilter*> FilterManager::filters() const
{
  return d->mFilters;
}

void FilterManager::removeFilter( MailCommon::MailFilter *filter )
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
                                   boost::bind( &MailCommon::MailFilter::requiresBody, _1 ) ) != d->mFilters.constEnd();
  // check if at least one filter is to be applied on inbound mail
  d->mInboundFiltersExist = std::find_if( d->mFilters.constBegin(), d->mFilters.constEnd(),
                                          boost::bind( &MailCommon::MailFilter::applyOnInbound, _1 ) ) != d->mFilters.constEnd();

  emit filterListUpdated();
}

bool FilterManager::requiresFullMailBody() const
{
  return d->mRequiresBody;
}

#ifndef NDEBUG
void FilterManager::dump() const
{
  foreach ( const MailCommon::MailFilter *filter, d->mFilters ) {
    kDebug() << filter->asString();
  }
}
#endif

void FilterManager::applyFilters( const QList<Akonadi::Item> &selectedMessages, FilterSet filterSet )
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
  itemFetchJob->setProperty( "filterSet", QVariant::fromValue( static_cast<int>( filterSet ) ) );

  connect( itemFetchJob, SIGNAL(itemsReceived(Akonadi::Item::List)),
           this, SLOT(slotItemsFetchedForFilter(Akonadi::Item::List)) );
  connect( itemFetchJob, SIGNAL(result(KJob*)),
           SLOT(itemsFetchJobForFilterDone(KJob*)) );
}

#include "filtermanager.moc"
