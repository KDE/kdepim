/*
    Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mailfilteragent.h"

#include "mailcommon/dbusoperators.h"
#include "dummykernel.h"
#include "filterlogdialog.h"
#include "filtermanager.h"
#include "mailfilteragentadaptor.h"
#include "pop3resourceattribute.h"

#include <akonadi/changerecorder.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/dbusconnectionpool.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>
#include <akonadi/session.h>
#include <mailcommon/kernel/mailkernel.h>
#include <KLocalizedString>
#include <KMime/Message>
#include <KNotification>
#include <KWindowSystem>
#include <Akonadi/AgentManager>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/AttributeFactory>

#include <QtCore/QVector>
#include <QtCore/QTimer>

static bool isFilterableCollection( const Akonadi::Collection &collection )
{
    return MailCommon::Kernel::folderIsInbox( collection );

    //TODO: check got filter attribute here
}

MailFilterAgent::MailFilterAgent( const QString &id )
    : Akonadi::AgentBase( id ),
      m_filterLogDialog( 0 )
{
    KGlobal::locale()->insertCatalog(QLatin1String("libmailcommon"));
    Akonadi::AttributeFactory::registerAttribute<Pop3ResourceAttribute>();
    DummyKernel *kernel = new DummyKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    m_filterManager = new FilterManager( this );

    connect(m_filterManager, SIGNAL(percent(int)), this, SLOT(emitProgress(int)));
    connect(m_filterManager, SIGNAL(progressMessage(QString)), this, SLOT(emitProgressMessage(QString)));

    Akonadi::Monitor *collectionMonitor = new Akonadi::Monitor( this );
    collectionMonitor->fetchCollection( true );
    collectionMonitor->ignoreSession( Akonadi::Session::defaultSession() );
    collectionMonitor->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
    collectionMonitor->setMimeTypeMonitored( KMime::Message::mimeType() );

    connect( collectionMonitor, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)),
             this, SLOT(mailCollectionAdded(Akonadi::Collection,Akonadi::Collection)) );
    connect( collectionMonitor, SIGNAL(collectionChanged(Akonadi::Collection)),
             this, SLOT(mailCollectionChanged(Akonadi::Collection)) );

    connect( collectionMonitor, SIGNAL(collectionRemoved(Akonadi::Collection)),
             this, SLOT(mailCollectionRemoved(Akonadi::Collection)) );

    QTimer::singleShot( 0, this, SLOT(initializeCollections()) );

    qDBusRegisterMetaType<QList<qint64> >();

    new MailFilterAgentAdaptor( this );

    Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/MailFilterAgent" ), this, QDBusConnection::ExportAdaptors );
    Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.MailFilterAgent" ) );
    //Enabled or not filterlogdialog
    KSharedConfig::Ptr config = KGlobal::config();
    if ( config->hasGroup( "FilterLog" ) ) {
        KConfigGroup group( config, "FilterLog" );
        if ( group.hasKey( "Enabled" ) ) {
            if ( group.readEntry( "Enabled", false ) ) {
                m_filterLogDialog = new FilterLogDialog( 0 );
                const QPixmap pixmap = KIcon( QLatin1String("view-filter") ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );
                KNotification *notify = new KNotification( QLatin1String("mailfilterlogenabled") );
                notify->setComponentData( componentData() );
                notify->setPixmap( pixmap );
                notify->setText( i18nc("Notification when the filter log was enabled", "Mail Filter Log Enabled" ) );
                notify->sendEvent();
            }
        }
    }

    changeRecorder()->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    changeRecorder()->itemFetchScope().setCacheOnly(true);
    changeRecorder()->fetchCollection( true );
    changeRecorder()->setChangeRecordingEnabled( false );

    mProgressCounter = 0;
    mProgressTimer = new QTimer( this );
    connect(mProgressTimer, SIGNAL(timeout()), this, SLOT(emitProgress()));
}

MailFilterAgent::~MailFilterAgent()
{
    delete m_filterLogDialog;
}

void MailFilterAgent::initializeCollections()
{
    m_filterManager->readConfig();

    Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this );
    job->fetchScope().setContentMimeTypes( QStringList() << KMime::Message::mimeType() );
    connect( job, SIGNAL(result(KJob*)), this, SLOT(initialCollectionFetchingDone(KJob*)) );
}

void MailFilterAgent::initialCollectionFetchingDone( KJob *job )
{
    if ( job->error() ) {
        qWarning() << job->errorString();
        return; //TODO: proper error handling
    }

    Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob*>( job );

    foreach ( const Akonadi::Collection &collection, fetchJob->collections() ) {
        if ( isFilterableCollection( collection ) )
            changeRecorder()->setCollectionMonitored( collection, true );
    }
    emit status(AgentBase::Idle, i18n("Ready") );
    emit percent(100);
    QTimer::singleShot( 2000, this, SLOT(clearMessage()) );
}

void MailFilterAgent::clearMessage()
{
    emit status(AgentBase::Idle, QString() );
}

void MailFilterAgent::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
    /* The monitor mimetype filter would override the collection filter, therefor we have to check
   * for the mimetype of the item here.
   */
    if ( item.mimeType() != KMime::Message::mimeType() ) {
        kDebug() << "MailFilterAgent::itemAdded called for a non-message item!";
        return;
    }

    MailCommon::SearchRule::RequiredPart requiredPart = m_filterManager->requiredPart(collection.resource());

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(item);
    connect( job, SIGNAL(itemsReceived(Akonadi::Item::List)),
             this, SLOT(itemsReceiviedForFiltering(Akonadi::Item::List)) );
    if (requiredPart == MailCommon::SearchRule::CompleteMessage) {
        job->fetchScope().fetchFullPayload();
    } else if (requiredPart == MailCommon::SearchRule::Header) {
        job->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Header, true );
    } else {
        job->fetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope, true );
    }
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    job->fetchScope().fetchAttribute<Pop3ResourceAttribute>();
    job->setProperty( "resource", collection.resource() );

    //TODO: Error handling?
}

void MailFilterAgent::itemsReceiviedForFiltering (const Akonadi::Item::List& items)
{
    if (items.isEmpty()) {
        kDebug() << "MailFilterAgent::itemsReceiviedForFiltering items is empty!";
        return;
    }

    Akonadi::Item item = items.first();
    /*
   * happens when item no longer exists etc, and queue compression didn't happen yet
   */
    if ( !item.hasPayload() ) {
        kDebug() << "MailFilterAgent::itemsReceiviedForFiltering item has no payload!";
        return;
    }

    Akonadi::MessageStatus status;
    status.setStatusFromFlags( item.flags() );
    if ( status.isRead() || status.isSpam() || status.isIgnored() ) {
        return;
    }

    QString resource = sender()->property("resource").toString();
    const Pop3ResourceAttribute *pop3ResourceAttribute = item.attribute<Pop3ResourceAttribute>();
    if ( pop3ResourceAttribute ) {
        resource = pop3ResourceAttribute->pop3AccountName();
    }

    emitProgressMessage(i18n("Filtering in %1",Akonadi::AgentManager::self()->instance(resource).name()) );
    m_filterManager->process( item, m_filterManager->requiredPart(resource), FilterManager::Inbound, true, resource );

    emitProgress( ++mProgressCounter );

    mProgressTimer->start(1000);
}

void MailFilterAgent::mailCollectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection& )
{
    if ( isFilterableCollection( collection ) )
        changeRecorder()->setCollectionMonitored( collection, true );
}

void MailFilterAgent::mailCollectionChanged( const Akonadi::Collection &collection )
{
    changeRecorder()->setCollectionMonitored( collection, isFilterableCollection( collection ) );
}

void MailFilterAgent::mailCollectionRemoved( const Akonadi::Collection& collection )
{
    changeRecorder()->setCollectionMonitored( collection, false );
    m_filterManager->mailCollectionRemoved(collection);
}

QString MailFilterAgent::createUniqueName( const QString &nameTemplate )
{
    return m_filterManager->createUniqueName( nameTemplate );
}

void MailFilterAgent::filterItems( const QList< qint64 >& itemIds, int filterSet )
{
    QList<Akonadi::Item> items;
    foreach ( qint64 id, itemIds ) {
        items << Akonadi::Item( id );
    }

    m_filterManager->applyFilters( items, static_cast<FilterManager::FilterSet>(filterSet) );
}

void MailFilterAgent::applySpecificFilters( const QList< qint64 >& itemIds, int requires, const QStringList& listFilters )
{
    QList<Akonadi::Item> items;
    foreach ( qint64 id, itemIds ) {
        items << Akonadi::Item( id );
    }

    m_filterManager->applySpecificFilters( items, static_cast<MailCommon::SearchRule::RequiredPart>(requires),listFilters );
}


void MailFilterAgent::filterItem( qint64 item, int filterSet, const QString& resourceId )
{
    m_filterManager->filter( Akonadi::Item( item ), static_cast<FilterManager::FilterSet>( filterSet ), resourceId );
}

void MailFilterAgent::filter(qint64 item, const QString& filterIdentifier, const QString& resourceId)
{
    m_filterManager->filter( Akonadi::Item( item ), filterIdentifier, resourceId );
}

void MailFilterAgent::reload()
{
    Akonadi::Collection::List collections = changeRecorder()->collectionsMonitored();
    foreach( const Akonadi::Collection &collection, collections) {
        changeRecorder()->setCollectionMonitored( collection, false );
    }
    initializeCollections();
}

void MailFilterAgent::showFilterLogDialog(qlonglong windowId)
{
    if ( !m_filterLogDialog ) {
        m_filterLogDialog = new FilterLogDialog( 0 );
    }
#ifndef Q_WS_WIN
    KWindowSystem::setMainWindow(m_filterLogDialog,windowId);
#else
    KWindowSystem::setMainWindow(m_filterLogDialog,(HWND)windowId);
#endif
    m_filterLogDialog->show();
    m_filterLogDialog->raise();
    m_filterLogDialog->activateWindow();
    m_filterLogDialog->setModal(false);
}

void MailFilterAgent::emitProgress(int p)
{
    if ( p == 0 ) {
        mProgressTimer->stop();
        emit status(AgentBase::Idle, QString() );
    }
    mProgressCounter = p;
    emit percent(p);
}

void MailFilterAgent::emitProgressMessage (const QString& message)
{
    emit status(AgentBase::Running, message);
}

QString MailFilterAgent::printCollectionMonitored()
{
    QString printDebugCollection;
    Akonadi::Collection::List collections = changeRecorder()->collectionsMonitored();
    if (collections.isEmpty()) {
        printDebugCollection = QLatin1String("No collection is monitored!");
    } else {
        foreach( const Akonadi::Collection &collection, collections) {
            if (!printDebugCollection.isEmpty()) {
                printDebugCollection += QLatin1Char('\n');
            }
            printDebugCollection += QString::fromLatin1("Collection name: %1\n").arg(collection.name());
            printDebugCollection += QString::fromLatin1("Collection id: %1\n").arg(collection.id());
        }
    }
    return printDebugCollection;
}

AKONADI_AGENT_MAIN( MailFilterAgent )

