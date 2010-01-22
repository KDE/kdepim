/***************************************************************************
                          filters.cxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// Local Includes
#include "filters.hxx"
#include "kmailcvt.h"

// KDEPIM Includes
#include <messagecore/messagestatus.h>
#include <KPIMUtils/KFileIO>

// Akonadi Includes
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/CollectionCreateJob>

// KDE Includes
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>

// Qt Includes
#include <QFile>

//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

bool FilterInfo::s_terminateASAP = false;

FilterInfo::FilterInfo( KImportPageDlg* dlg, QWidget* parent , bool _removeDupMsg)
  : m_dlg( dlg ),
    m_parent( parent )
{
  removeDupMsg = _removeDupMsg;
  s_terminateASAP = false;
}

FilterInfo::~FilterInfo()
{
}

void FilterInfo::setStatusMsg( const QString& status )
{
  m_dlg->_textStatus->setText( status );
}

void FilterInfo::setFrom( const QString& from )
{
  m_dlg->_from->setText( from );
}

void FilterInfo::setTo( const QString& to )
{
  m_dlg->_to->setText( to );
}

void FilterInfo::setCurrent( const QString& current )
{
  m_dlg->_current->setText( current );
  kapp->processEvents();
}

void  FilterInfo::setCurrent( int percent )
{
  m_dlg->_done_current->setValue( percent );
  kapp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  FilterInfo::setOverall( int percent )
{
  m_dlg->_done_overall->setValue( percent );
}

void FilterInfo::addLog( const QString& log )
{
  m_dlg->_log->addItem( log );
  m_dlg->_log->setCurrentItem( m_dlg->_log->item(m_dlg->_log->count() - 1 ));
  kapp->processEvents();
}

void FilterInfo::clear()
{
  m_dlg->_log->clear();
  setCurrent();
  setOverall();
  setCurrent( QString() );
  setFrom( QString() );
  setTo( QString() );
}

void FilterInfo::alert( const QString& message )
{
  KMessageBox::information( m_parent, message );
}

void FilterInfo::terminateASAP()
{
  s_terminateASAP = true;
}

bool FilterInfo::shouldTerminate()
{
  return s_terminateASAP;
}

Akonadi::Collection FilterInfo::getRootCollection() const
{
  return m_rootCollection;
}

void FilterInfo::setRootCollection( const Akonadi::Collection &collection )
{
  m_rootCollection = collection;
}


//////////////////////////////////////////////////////////////////////////////////
//
// The generic filter class
//
//////////////////////////////////////////////////////////////////////////////////


Filter::Filter( const QString& name, const QString& author,
                const QString& info )
  : m_name( name ),
    m_author( author ),
    m_info( info )
{
    //public
    count_duplicates = 0;
}

bool Filter::addAkonadiMessage( FilterInfo* info, const Akonadi::Collection &collection,
                                const KMime::Message::Ptr message )
{
  Akonadi::Item item;
  KPIM::MessageStatus status;

  item.setMimeType( "message/rfc822" );

  KMime::Headers::Base *statusHeaders = message->headerByType( "X-Status" );
  if( statusHeaders ) {
    if( !statusHeaders->isEmpty() ) {
      status.setStatusFromStr( statusHeaders->asUnicodeString() );
      item.setFlags( status.getStatusFlags() );
    }
  }
  item.setPayload<KMime::Message::Ptr>( message );
  Akonadi::ItemCreateJob* job = new Akonadi::ItemCreateJob( item, collection );
  if( !job->exec() ) {
    info->alert( i18n( "<b>Error:<\b> Could not add message to the collection %1. Reason: %2",
		       collection.name(), job->errorString() ) );
    return false;
  }
  return true;
}

Akonadi::Collection Filter::parseFolderString(FilterInfo* info, const QString& folderParseString)
{
  // Return an already created collection:
  for( QMap<QString, Akonadi::Collection>::const_iterator it =
    m_messageFolderCollectionMap.constBegin(); it != m_messageFolderCollectionMap.constEnd(); it ++ ) {
    if( it.key() ==  folderParseString )
      return it.value();
  }

  // The folder hasn't yet been created, create it now.
  QStringList folderList = folderParseString.split("/");
  bool isFirst = true;
  QString folderBuilder;
  Akonadi::Collection lastCollection;

  // Create each folder on the folder list and add it the map.
  foreach( const QString &folder, folderList ) {
    if( isFirst ) {
      m_messageFolderCollectionMap[folder] = addSubCollection( info, info->getRootCollection(), folder );
      folderBuilder = folder;
      lastCollection = m_messageFolderCollectionMap[folder];
      isFirst = false;
    } else {
      folderBuilder += "/" + folder;
      m_messageFolderCollectionMap[folderBuilder] = addSubCollection( info, lastCollection, folder );
      lastCollection = m_messageFolderCollectionMap[folderBuilder];
    }
  }

  return lastCollection;
}

Akonadi::Collection Filter::addSubCollection( FilterInfo* info,
                                              const Akonadi::Collection &baseCollection,
					      const QString& newCollectionPathName )
{
  // Ensure that the collection doesn't already exsit, if it does just return it.
  Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob( baseCollection,
									   Akonadi::CollectionFetchJob::FirstLevel);
  if( !fetchJob->exec() ) {
    info->alert( i18n( "<b>Error:<\b> Could not execute fetchJob when adding sub collection because %1",
		       fetchJob->errorString() ) );
    return Akonadi::Collection();
  }

  foreach( const Akonadi::Collection &subCollection, fetchJob->collections() ) {
   if( subCollection.name() == newCollectionPathName ) {
     return subCollection;
   }
  }

  // The subCollection doesn't exsit, create a new one
  Akonadi::Collection newSubCollection;
  newSubCollection.setParentCollection( baseCollection );
  newSubCollection.setName( newCollectionPathName );

  Akonadi::CollectionCreateJob * job = new Akonadi::CollectionCreateJob( newSubCollection );
  if( !job->exec() ) {
    info->alert( i18n("<b>Error:<\b> Could not create subCollection because %1",
		 job->errorString() ) );
    return Akonadi::Collection();
  }
  // Return the newly created collection
  return job->collection();
}


bool Filter::addMessage( FilterInfo* info, const QString& folderName,
                         const QString& msgPath,
                         const QString&  msgStatusFlags // Defunct - KMime will now handle the MessageStatus flags.
                         )
{
  // Create the mail folder (if not already created).
  Akonadi::Collection mailFolder = parseFolderString( info, folderName );

  KUrl msgUrl( msgPath );
  if( !msgUrl.isEmpty() && msgUrl.isLocalFile() ) {

    // Read in the temporary file.
    const QByteArray msgText =
      KPIMUtils::kFileToByteArray( msgUrl.toLocalFile(), true, false );
    if( msgText.isEmpty() ) {
      info->addLog( "Error: failed to read temporary file at" + msgPath );
      return false;
    }

    // Construct a message.
    KMime::Message::Ptr newMessage( new KMime::Message() );
    newMessage->setContent( msgText );
    newMessage->parse();

    // Check for duplicate.
    for( QMap<QString, QString>::const_iterator it = m_messageFolderMessageIDMap.constBegin();
     it != m_messageFolderMessageIDMap.constEnd(); it++ ) {
      if( it.key() == folderName &&
        it.value() == newMessage->messageID()->asUnicodeString() ) {
        count_duplicates ++;
        return false;
      }
    }

    // Add the message and folder to the map for duplicate checking.
    m_messageFolderMessageIDMap[folderName] = newMessage->messageID()->asUnicodeString();

    // Add it to the collection.
    if( mailFolder.isValid() ) {
      addAkonadiMessage( info, mailFolder, newMessage );
    } else {
      info->alert( i18n( "<b>Warning:<\b> Got a bad message collection, adding to root collection." ) );
      addAkonadiMessage( info, info->getRootCollection(), newMessage );
    }
  }
  return true;
}

bool Filter::addMessage_fastImport( FilterInfo* info, const QString& folderName,
                         	    const QString& msgPath,
                                    const QString& msgStatusFlags // Defunct - KMime will now handle the MessageStatus flags.
                                    )
{
  // Create the mail folder (if not already created).
  Akonadi::Collection mailFolder = parseFolderString( info, folderName );

  KUrl msgUrl( msgPath );
  if( !msgUrl.isEmpty() && msgUrl.isLocalFile() ) {

    // Read in the temporary file.
    const QByteArray msgText =
      KPIMUtils::kFileToByteArray( msgUrl.toLocalFile(), true, false );
    if( msgText.isEmpty() ) {
      info->addLog( "Error: failed to read temporary file at" + msgPath );
      return false;
    }

    // Construct a message.
    KMime::Message::Ptr newMessage( new KMime::Message() );
    newMessage->setContent( msgText );
    newMessage->parse();

    // Add it to the collection.
    if( mailFolder.isValid() ) {
      addAkonadiMessage( info, mailFolder, newMessage );
    } else {
      info->alert( i18n( "<b>Warning:<\b> Got a bad message collection, adding to root collection." ) );
      addAkonadiMessage( info, info->getRootCollection(), newMessage );
    }
  }
  return true;

}

void Filter::showKMailImportArchiveDialog( FilterInfo* info )
{
  #if 0
  QDBusConnectionInterface * sessionBus = 0;
  sessionBus = QDBusConnection::sessionBus().interface();
  if ( sessionBus && !sessionBus->isServiceRegistered( "org.kde.kmail" ) )
    KToolInvocation::startServiceByDesktopName( "kmail", QString() ); // Will wait until kmail is started


  org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
  QDBusReply<void> reply = kmail.showImportArchiveDialog();

  if ( !reply.isValid() )
  {
    info->alert( i18n( "<b>Fatal:</b> Unable to start KMail for D-Bus communication: %1; %2<br />"
                       "Make sure <i>kmail</i> is installed.", reply.error().message(), reply.error().message() ) );
    return;
  }
  #endif
}

bool Filter::needsSecondPage()
{
  return true;
}

// vim: ts=2 sw=2 et
