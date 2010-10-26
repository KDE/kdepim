// -*- mode: C++; c-file-style: "gnu" -*-
// kmfiltermgr.cpp

// my header
#include "filtermanager.h"
#include "mailkernel.h"
#include "mailfilter.h"
// other kmail headers
#include "filterlog.h"
using MailCommon::FilterLog;
#include "filterimporterexporter.h"
using MailCommon::FilterImporterExporter;
#include "messageproperty.h"
using MailCommon::MessageProperty;

#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmovejob.h>

// other KDE headers
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <kmime/kmime_message.h>

#include <progressmanager.h>
#include <broadcaststatus.h>

// other Qt headers

// other headers
#include <boost/bind.hpp>
#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <QTimer>


using namespace MailCommon;

//-----------------------------------------------------------------------------
FilterManager::FilterManager( bool popFilter )
  : bPopFilter( popFilter ),
    mShowLater( false ),
    mRequiresBody( false )
{
  if ( bPopFilter ) {
    kDebug() << "pPopFilter set";
  }

  tryToMonitorCollection();
  
  mChangeRecorder = new Akonadi::ChangeRecorder( this );
  mChangeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );
  mChangeRecorder->setChangeRecordingEnabled( false );
  mChangeRecorder->fetchCollection( true );
  connect( mChangeRecorder, SIGNAL(itemAdded(Akonadi::Item,Akonadi::Collection)),
           SLOT(itemAdded(Akonadi::Item,Akonadi::Collection)) );
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

  if ( bPopFilter ) {
    KConfigGroup group = config->group( "General" );
    mShowLater = group.readEntry( "popshowDLmsgs", false );
  }
  mFilters = FilterImporterExporter::readFiltersFromConfig( config, bPopFilter );
  endUpdate();
}

//-----------------------------------------------------------------------------
void FilterManager::writeConfig(bool withSync)
{
  KSharedConfig::Ptr config = KernelIf->config();

  // Now, write out the new stuff:
  FilterImporterExporter::writeFiltersToConfig( mFilters, config, bPopFilter );
  KConfigGroup group = config->group( "General" );
  if ( bPopFilter )
      group.writeEntry("popshowDLmsgs", mShowLater);

 if ( withSync ) group.sync();
}

int FilterManager::processPop( const Akonadi::Item & item ) const {
  for ( QList<MailFilter*>::const_iterator it = mFilters.begin();
        it != mFilters.end() ; ++it )
    if ( (*it)->pattern()->matches( item ) )
      return (*it)->action();

  return NoAction;
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

  if ( bPopFilter )
    return processPop( item );

  if ( set == NoSet ) {
    kDebug() << "FilterManager: process() called with not filter set selected";
    return 1;
  }
  bool stopIt = false;
  bool atLeastOneRuleMatched = false;

  if ( !beginFiltering( item ) )
    return 1;
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
        // filter matches
        atLeastOneRuleMatched = true;
        // execute actions:
        if ( (*it)->execActions(item, stopIt) == MailFilter::CriticalError ) {
          return 2;
        }
      }
    }
  }

  Akonadi::Collection targetFolder = MessageProperty::filterFolder( item );
  /* endFilter does a take() and addButKeepUID() to ensure the changed
   * message is on disk. This is unnessecary if nothing matched, so just
   * reset state and don't update the listview at all. */
  if ( atLeastOneRuleMatched ) {
    endFiltering( item );
  } else {
    MessageProperty::setFiltering( item, false );
  }
  if ( targetFolder.isValid() ) {
    new Akonadi::ItemMoveJob( item, targetFolder, this ); // TODO: check result
    return 0;
  }
  return 1;
}

bool FilterManager::isMatching( const Akonadi::Item& item, const MailFilter * filter )
{
  bool result = false;
  if ( FilterLog::instance()->isLogging() ) {
    QString logText( i18n( "<b>Evaluating filter rules:</b> " ) );
    logText.append( filter->pattern()->asString() );
    FilterLog::instance()->add( logText, FilterLog::patternDesc );
  }
  if ( filter->pattern()->matches( item ) ) {
    if ( FilterLog::instance()->isLogging() ) {
      FilterLog::instance()->add( i18n( "<b>Filter rules have matched.</b>" ),
                                  FilterLog::patternResult );
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
  FilterIf->openFilterDialog( bPopFilter, checkForEmptyFilterList );
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
  // We only filter the inboxes, not mail that arrives into other folders
  // TODO should probably also check SpecialMailCollections once there
  // is a method in there which directly checks for the attribute
  if (collection.name().toLower() == "inbox") {
    if ( mRequiresBody ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );
      job->fetchScope().fetchFullPayload( true );
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
  if ( job->error() )
    kDebug() << job->errorString();
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
