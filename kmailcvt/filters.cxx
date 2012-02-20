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
#include <KPIMUtils/KFileIO>

// Akonadi Includes
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/CollectionCreateJob>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>

// KDE Includes
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>

// Qt Includes

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

bool FilterInfo::shouldTerminate() const
{
  return s_terminateASAP;
}

Akonadi::Collection FilterInfo::rootCollection() const
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
  : m_count_duplicates( 0 ),
    m_filterInfo( 0 ), 
    m_name( name ),
    m_author( author ),
    m_info( info )
{
}

void Filter::setFilterInfo( FilterInfo* info )
{
  m_filterInfo = info;
}

bool Filter::addAkonadiMessage( const Akonadi::Collection &collection,
                                const KMime::Message::Ptr& message )
{
  Akonadi::Item item;
  Akonadi::MessageStatus status;

  item.setMimeType( "message/rfc822" );

  KMime::Headers::Base *statusHeaders = message->headerByType( "X-Status" );
  if( statusHeaders ) {
    if( !statusHeaders->isEmpty() ) {
      status.setStatusFromStr( statusHeaders->asUnicodeString() );
      item.setFlags( status.statusFlags() );
    }
  }
  item.setPayload<KMime::Message::Ptr>( message );
  Akonadi::ItemCreateJob* job = new Akonadi::ItemCreateJob( item, collection );
  if( !job->exec() ) {
    m_filterInfo->alert( i18n( "<b>Error:</b> Could not add message to folder %1. Reason: %2",
		       collection.name(), job->errorString() ) );
    return false;
  }
  return true;
}

Akonadi::Collection Filter::parseFolderString(const QString& folderParseString)
{
  // Return an already created collection:
  QMap<QString, Akonadi::Collection>::const_iterator end(  m_messageFolderCollectionMap.constEnd() );
  for( QMap<QString, Akonadi::Collection>::const_iterator it = m_messageFolderCollectionMap.constBegin(); it != end; it ++ ) {
    if( it.key() ==  folderParseString )
      return it.value();
  }

  // The folder hasn't yet been created, create it now.
  const QStringList folderList = folderParseString.split( '/', QString::SkipEmptyParts );
  bool isFirst = true;
  QString folderBuilder;
  Akonadi::Collection lastCollection;

  // Create each folder on the folder list and add it the map.
  foreach( const QString &folder, folderList ) {
    if( isFirst ) {
      m_messageFolderCollectionMap[folder] = addSubCollection( m_filterInfo->rootCollection(), folder );
      folderBuilder = folder;
      lastCollection = m_messageFolderCollectionMap[folder];
      isFirst = false;
    } else {
      folderBuilder += '/' + folder;
      m_messageFolderCollectionMap[folderBuilder] = addSubCollection( lastCollection, folder );
      lastCollection = m_messageFolderCollectionMap[folderBuilder];
    }
  }

  return lastCollection;
}

Akonadi::Collection Filter::addSubCollection( const Akonadi::Collection &baseCollection,
					      const QString& newCollectionPathName )
{
  // Ensure that the collection doesn't already exsit, if it does just return it.
  Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob( baseCollection,
									   Akonadi::CollectionFetchJob::FirstLevel);
  if( !fetchJob->exec() ) {
    m_filterInfo->alert( i18n( "<b>Warning:</b> Could not check that the folder already exists. Reason: %1",
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
    m_filterInfo->alert( i18n("<b>Error:</b> Could not create folder. Reason: %1",
		 job->errorString() ) );
    return Akonadi::Collection();
  }
  // Return the newly created collection
  return job->collection();
}

bool Filter::checkForDuplicates ( const QString& msgID,
                                  const Akonadi::Collection& msgCollection,
                                  const QString& messageFolder )
{
  bool folderFound = false;

  // Check if the contents of this collection have already been found.
  QMultiMap<QString, QString>::const_iterator end( m_messageFolderMessageIDMap.constEnd() );
  for( QMultiMap<QString, QString>::const_iterator it = m_messageFolderMessageIDMap.constBegin();
       it != end; it++ ) {
    if( it.key() == messageFolder ) {
      folderFound = true;
      break;
    }
  }

  if( !folderFound ) {
    // Populate the map with message IDs that are in that collection.
    if( msgCollection.isValid() ) {
      Akonadi::ItemFetchJob job( msgCollection );
      job.fetchScope().fetchPayloadPart( Akonadi::MessagePart::Header );
      if( !job.exec() ) {
        m_filterInfo->addLog( i18n( "<b>Warning:</b> Could not fetch mail in folder %1. Reason: %2"
        " You may have duplicate messages.", messageFolder, job.errorString() ) );
      } else {
        foreach( const Akonadi::Item& messageItem, job.items() ) {
          if( !messageItem.isValid() ) {
            m_filterInfo->addLog( i18n( "<b>Warning:</b> Got an invalid message in folder %1.", messageFolder ) );
          } else {
            if( !messageItem.hasPayload<KMime::Message::Ptr>() )
              continue;
            const KMime::Message::Ptr message = messageItem.payload<KMime::Message::Ptr>();
            const KMime::Headers::Base* messageID = message->messageID( false );
            if( messageID ) {
              if( !messageID->isEmpty() ) {
                m_messageFolderMessageIDMap.insert( messageFolder, messageID->asUnicodeString() );
              }
            }
          }
        }
      }
    }
  }

  // Check if this message has a duplicate
  QMultiMap<QString, QString>::const_iterator endMsgID( m_messageFolderMessageIDMap.constEnd() );
  for( QMultiMap<QString, QString>::const_iterator it = m_messageFolderMessageIDMap.constBegin();
       it !=endMsgID ; it++ ) {
    if( it.key() == messageFolder &&
        it.value() == msgID )
      return true;
  }

  // The message isn't a duplicate, but add it to the map for checking in the future.
  m_messageFolderMessageIDMap.insert( messageFolder, msgID );
  return false;
}


bool Filter::addMessage( const QString& folderName,
                         const QString& msgPath,
                         const QString&  msgStatusFlags // Defunct - KMime will now handle the MessageStatus flags.
                         )
{
  Q_UNUSED( msgStatusFlags );

  // Add the message.
  return doAddMessage( folderName, msgPath, true );
}

bool Filter::addMessage_fastImport( const QString& folderName,
                         	    const QString& msgPath,
                                    const QString& msgStatusFlags // Defunct - KMime will now handle the MessageStatus flags.
                                    )
{
  Q_UNUSED( msgStatusFlags );

  // Add the message.
  return doAddMessage( folderName, msgPath );
}

bool Filter::doAddMessage( const QString& folderName,
                           const QString& msgPath,
                           bool duplicateCheck )
{
  QString messageID;
  // Create the mail folder (if not already created).
  Akonadi::Collection mailFolder = parseFolderString(folderName );

  KUrl msgUrl( msgPath );
  if( !msgUrl.isEmpty() && msgUrl.isLocalFile() ) {

    // Read in the temporary file.
    const QByteArray msgText =
      KPIMUtils::kFileToByteArray( msgUrl.toLocalFile(), true, false );
    if( msgText.isEmpty() ) {
      m_filterInfo->addLog( i18n( "Error: failed to read temporary file at %1", msgPath ) );
      return false;
    }

    // Construct a message.
    KMime::Message::Ptr newMessage( new KMime::Message() );
    newMessage->setContent( msgText );
    newMessage->parse();

    if( duplicateCheck ) {
      // Get the messageID.
      const KMime::Headers::Base* messageIDHeader = newMessage->messageID( false );
      if( messageIDHeader )
        messageID = messageIDHeader->asUnicodeString();

      if( !messageID.isEmpty() ) {
        // Check for duplicate.
        if( checkForDuplicates( messageID, mailFolder, folderName ) ) {
          m_count_duplicates++;
          return false;
        }
      }
    }

    // Add it to the collection.
    if( mailFolder.isValid() ) {
      addAkonadiMessage( mailFolder, newMessage );
    } else {
      m_filterInfo->alert( i18n( "<b>Warning:</b> Got a bad message folder, adding to root folder." ) );
      addAkonadiMessage( m_filterInfo->rootCollection(), newMessage );
    }
  }
  return true;
}


bool Filter::needsSecondPage()
{
  return true;
}

// vim: ts=2 sw=2 et
