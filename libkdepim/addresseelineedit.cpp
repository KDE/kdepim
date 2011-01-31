/*
  This file is part of libkdepim.

  Copyright (c) 2002 Helge Deller <deller@gmx.de>
  Copyright (c) 2002 Lubos Lunak <llunak@suse.cz>
  Copyright (c) 2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
  Copyright (c) 2004 Daniel Molkentin <danimo@klaralvdalens-datakonsult.se>
  Copyright (c) 2004 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "addresseelineedit.h"

#ifndef Q_OS_WINCE
#include "completionordereditor.h"
#endif

#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemFetchScope>

#include <KPIMUtils/Email>

#include <KLDAP/LdapServer>

#include <KMime/Util>

#include <KConfigGroup>
#include <KCompletionBox>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <KStandardShortcut>
#include <KUrl>

#include <solid/networking.h>

#include <QApplication>
#include <QCursor>
#include <QObject>
#include <QRegExp>
#include <QEvent>
#include <QClipboard>
#include <QKeyEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include <QtDBus/QDBusConnection>

using namespace KPIM;

class AddresseeLineEditStatic
{
  public:

    AddresseeLineEditStatic()
      : completion( new KMailCompletion ),
        ldapTimer( 0 ),
        ldapSearch( 0 ),
        ldapLineEdit( 0 )
    {
    }

    ~AddresseeLineEditStatic()
    {
      delete completion;
      delete ldapTimer;
      delete ldapSearch;
    }

    KMailCompletion *completion;
    KPIM::CompletionItemsMap completionItemMap;
    QStringList completionSources;
    QTimer *ldapTimer;
    KLDAP::LdapClientSearch *ldapSearch;
    QString ldapText;
    AddresseeLineEdit *ldapLineEdit;
    // The weights associated with the completion sources in s_static->completionSources.
    // Both are maintained by addCompletionSource(), don't attempt to modifiy those yourself.
    QMap<QString, int> completionSourceWeights;
    // maps LDAP client indices to completion source indices
    // the assumption that they are always the first n indices in s_static->completion
    // does not hold when clients are added later on
    QMap<int, int> ldapClientToCompletionSourceMap;
    // holds the cached mapping from akonadi collection id to the completion source index
    QMap<Akonadi::Collection::Id, int> akonadiCollectionToCompletionSourceMap;
    // a list of akonadi items (contacts) that have not had their collection fetched yet
    Akonadi::Item::List akonadiPendingItems;
};

K_GLOBAL_STATIC( AddresseeLineEditStatic, s_static )

// needs to be unique, but the actual name doesn't matter much
static QByteArray newLineEditObjectName()
{
  static int s_count = 0;
  QByteArray name( "KPIM::AddresseeLineEdit" );
  if ( s_count++ ) {
    name += '-';
    name += QByteArray().setNum( s_count );
  }
  return name;
}

static const QString s_completionItemIndentString = "     ";

static bool itemIsHeader( const QListWidgetItem *item )
{
  return item && !item->text().startsWith( s_completionItemIndentString );
}

class SourceWithWeight
{
  public:
    int weight;           // the weight of the source
    QString sourceName;   // the name of the source, e.g. "LDAP Server"
    int index;            // index into s_static->completionSources

    bool operator< ( const SourceWithWeight &other ) const
    {
      if ( weight > other.weight ) {
        return true;
      }

      if ( weight < other.weight ) {
        return false;
      }

      return sourceName < other.sourceName;
    }
};

class AddresseeLineEdit::Private
{
  public:
    Private( AddresseeLineEdit *qq, bool enableCompletion )
      : q( qq ),
        m_useCompletion( enableCompletion ),
        m_completionInitialized( false ),
        m_smartPaste( false ),
        m_addressBookConnected( false ),
        m_searchExtended( false ),
        m_useSemicolonAsSeparator( false )
    {
    }

    void init();
    void startLoadingLDAPEntries();
    void stopLDAPLookup();
    void updateLDAPWeights();
    void setCompletedItems( const QStringList &items, bool autoSuggest );
    void addCompletionItem( const QString &string, int weight, int source,
                            const QStringList *keyWords = 0 );
    const QStringList adjustedCompletionItems( bool fullSearch );
    void updateSearchString();
    void akonadiPerformSearch();
    void akonadiHandlePending();
    void doCompletion( bool ctrlT );

    void slotCompletion();
    void slotPopupCompletion( const QString & );
    void slotReturnPressed( const QString & );
    void slotStartLDAPLookup();
    void slotLDAPSearchData( const KLDAP::LdapResult::List & );
    void slotEditCompletionOrder();
    void slotUserCancelled( const QString & );
    void slotAkonadiSearchResult( KJob * );
    void slotAkonadiCollectionsReceived( const Akonadi::Collection::List & );

    static KCompletion::CompOrder completionOrder();

    AddresseeLineEdit *q;
    QString m_previousAddresses;
    QString m_searchString;
    bool m_useCompletion;
    bool m_completionInitialized;
    bool m_smartPaste;
    bool m_addressBookConnected;
    bool m_lastSearchMode;
    bool m_searchExtended; //has \" been added?
    bool m_useSemicolonAsSeparator;
};

void AddresseeLineEdit::Private::init()
{
  if ( !s_static.exists() ) {
    s_static->completion->setOrder( completionOrder() );
    s_static->completion->setIgnoreCase( true );
  }

  if ( m_useCompletion ) {
    if ( !s_static->ldapTimer ) {
      s_static->ldapTimer = new QTimer;
      s_static->ldapSearch = new KLDAP::LdapClientSearch;
    }

    updateLDAPWeights();

    if ( !m_completionInitialized ) {
      q->setCompletionObject( s_static->completion, false );
      q->connect( q, SIGNAL(completion(const QString &)),
                  q, SLOT(slotCompletion()) );
      q->connect( q, SIGNAL(returnPressed(const QString &)),
                  q, SLOT(slotReturnPressed(const QString &)) );

      KCompletionBox *box = q->completionBox();
      q->connect( box, SIGNAL(activated(const QString &)),
                  q, SLOT(slotPopupCompletion(const QString &)) );
      q->connect( box, SIGNAL(userCancelled(const QString &)),
                  q, SLOT(slotUserCancelled(const QString &)) );

      q->connect( s_static->ldapTimer, SIGNAL(timeout()), SLOT(slotStartLDAPLookup()) );
      q->connect( s_static->ldapSearch, SIGNAL(searchData(const KLDAP::LdapResult::List &)),
                  SLOT(slotLDAPSearchData(const KLDAP::LdapResult::List &)) );

      m_completionInitialized = true;
    }
  }
}

void AddresseeLineEdit::Private::startLoadingLDAPEntries()
{
  QString text( s_static->ldapText );

  // TODO cache last?
  QString prevAddr;
  const int index = text.lastIndexOf( QLatin1Char( ',' ) );
  if ( index >= 0 ) {
    prevAddr = text.left( index + 1 ) + QLatin1Char( ' ' );
    text = text.mid( index + 1, 255 ).trimmed();
  }

  if ( text.isEmpty() ) {
    return;
  }

  s_static->ldapSearch->startSearch( text );
}

void AddresseeLineEdit::Private::stopLDAPLookup()
{
  s_static->ldapSearch->cancelSearch();
  s_static->ldapLineEdit = 0;
}

void AddresseeLineEdit::Private::updateLDAPWeights()
{
  /* Add completion sources for all ldap server, 0 to n. Added first so
   * that they map to the LdapClient::clientNumber() */
  s_static->ldapSearch->updateCompletionWeights();
  int clientIndex = 0;
  foreach ( const KLDAP::LdapClient *client, s_static->ldapSearch->clients() ) {
    const int sourceIndex =
      q->addCompletionSource( QLatin1String( "LDAP server: " ) + client->server().host(),
                              client->completionWeight() );

    s_static->ldapClientToCompletionSourceMap.insert( clientIndex, sourceIndex );

    clientIndex++;
  }
}

void AddresseeLineEdit::Private::setCompletedItems( const QStringList &items, bool autoSuggest )
{
  KCompletionBox *completionBox = q->completionBox();

  if ( !items.isEmpty() &&
       !( items.count() == 1 && m_searchString == items.first() ) ) {

    completionBox->setItems( items );

    if ( !completionBox->isVisible() ) {
      if ( !m_searchString.isEmpty() ) {
        completionBox->setCancelledText( m_searchString );
      }
      completionBox->popup();
      // we have to install the event filter after popup(), since that
      // calls show(), and that's where KCompletionBox installs its filter.
      // We want to be first, though, so do it now.
      if ( s_static->completion->order() == KCompletion::Weighted ) {
        qApp->installEventFilter( q );
      }
    }

    QListWidgetItem *item = completionBox->item( 1 );
    if ( item ) {
      completionBox->blockSignals( true );
      completionBox->setCurrentItem( item );
      item->setSelected( true );
      completionBox->blockSignals( false );
    }

    if ( autoSuggest ) {
      const int index = items.first().indexOf( m_searchString );
      const QString newText = items.first().mid( index );
      q->setUserSelection( false );
      q->setCompletedText( newText, true );
    }
  } else {
    if ( completionBox && completionBox->isVisible() ) {
      completionBox->hide();
      completionBox->setItems( QStringList() );
    }
  }
}

void AddresseeLineEdit::Private::addCompletionItem( const QString &string, int weight,
                                                    int completionItemSource,
                                                    const QStringList *keyWords )
{
  // Check if there is an exact match for item already, and use the
  // maximum weight if so. Since there's no way to get the information
  // from KCompletion, we have to keep our own QMap.

  CompletionItemsMap::iterator it = s_static->completionItemMap.find( string );
  if ( it != s_static->completionItemMap.end() ) {
    weight = qMax( ( *it ).first, weight );
    ( *it ).first = weight;
  } else {
    s_static->completionItemMap.insert( string, qMakePair( weight, completionItemSource ) );
  }

  if ( keyWords == 0 ) {
    s_static->completion->addItem( string, weight );
  } else {
    s_static->completion->addItemWithKeys( string, weight, keyWords );
  }
}

const QStringList KPIM::AddresseeLineEdit::Private::adjustedCompletionItems( bool fullSearch )
{
  QStringList items = fullSearch ?
                      s_static->completion->allMatches( m_searchString ) :
                      s_static->completion->substringCompletion( m_searchString );

  //force items to be sorted by email
  items.sort();

  // For weighted mode, the algorithm is the following:
  // In the first loop, we add each item to its section (there is one section per completion source)
  // We also add spaces in front of the items.
  // The sections are appended to the items list.
  // In the second loop, we then walk through the sections and add all the items in there to the
  // sorted item list, which is the final result.
  //
  // The algo for non-weighted mode is different.

  int lastSourceIndex = -1;
  unsigned int i = 0;

  // Maps indices of the items list, which are section headers/source items,
  // to a QStringList which are the items of that section/source.
  QMap<int, QStringList> sections;
  QStringList sortedItems;
  for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it, ++i ) {
    CompletionItemsMap::const_iterator cit = s_static->completionItemMap.constFind( *it );
    if ( cit == s_static->completionItemMap.constEnd() ) {
      continue;
    }

    const int index = (*cit).second;

    if ( s_static->completion->order() == KCompletion::Weighted ) {
      if ( lastSourceIndex == -1 || lastSourceIndex != index ) {
        const QString sourceLabel( s_static->completionSources[ index ] );
        if ( sections.find( index ) == sections.end() ) {
          it = items.insert( it, sourceLabel );
          ++it; //skip new item
        }
        lastSourceIndex = index;
      }

      (*it) = (*it).prepend( s_completionItemIndentString );
      // remove preferred email sort <blank> added in  addContact()
      (*it).replace( QLatin1String( "  <" ), QLatin1String( " <" ) );
    }
    sections[ index ].append( *it );

    if ( s_static->completion->order() == KCompletion::Sorted ) {
      sortedItems.append( *it );
    }
  }

  if ( s_static->completion->order() == KCompletion::Weighted ) {

    // Sort the sections
    QList<SourceWithWeight> sourcesAndWeights;
    for ( int i = 0; i < s_static->completionSources.size(); i++ ) {
      SourceWithWeight sww;
      sww.sourceName = s_static->completionSources[i];
      sww.weight = s_static->completionSourceWeights[sww.sourceName];
      sww.index = i;
      sourcesAndWeights.append( sww );
    }
    qSort( sourcesAndWeights.begin(), sourcesAndWeights.end() );

    // Add the sections and their items to the final sortedItems result list
    for ( int i = 0; i < sourcesAndWeights.size(); i++ ) {
      const QStringList sectionItems = sections[sourcesAndWeights[i].index];
      if ( !sectionItems.isEmpty() ) {
        sortedItems.append( sourcesAndWeights[i].sourceName );
        foreach ( const QString &itemInSection, sectionItems ) {
          sortedItems.append( itemInSection );
        }
      }
    }
  } else {
    sortedItems.sort();
  }

  return sortedItems;
}

void AddresseeLineEdit::Private::updateSearchString()
{
  m_searchString = q->text();

  int n = -1;
  bool inQuote = false;
  uint searchStringLength = m_searchString.length();
  for ( uint i = 0; i < searchStringLength; ++i ) {
    if ( m_searchString[ i ] == QLatin1Char( '"' ) ) {
      inQuote = !inQuote;
    }

    if ( m_searchString[ i ] == '\\' &&
         ( i + 1 ) < searchStringLength && m_searchString[ i + 1 ] == QLatin1Char( '"' ) ) {
      ++i;
    }

    if ( inQuote ) {
      continue;
    }

    if ( i < searchStringLength &&
         ( m_searchString[ i ] == ',' ||
           ( m_useSemicolonAsSeparator && m_searchString[ i ] == ';' ) ) ) {
      n = i;
    }
  }

  if ( n >= 0 ) {
    ++n; // Go past the ","

    const int len = m_searchString.length();

    // Increment past any whitespace...
    while ( n < len && m_searchString[ n ].isSpace() ) {
      ++n;
    }

    m_previousAddresses = m_searchString.left( n );
    m_searchString = m_searchString.mid( n ).trimmed();
  } else {
    m_previousAddresses.clear();
  }
}

void AddresseeLineEdit::Private::akonadiPerformSearch()
{
  kDebug() << "searching akonadi with:" << m_searchString;
  Akonadi::ContactSearchJob *contactJob = new Akonadi::ContactSearchJob();
  Akonadi::ContactGroupSearchJob *groupJob = new Akonadi::ContactGroupSearchJob();
  contactJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  groupJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  contactJob->setQuery( Akonadi::ContactSearchJob::NameOrEmail, m_searchString,
                        Akonadi::ContactSearchJob::ContainsMatch );
  groupJob->setQuery( Akonadi::ContactGroupSearchJob::Name, m_searchString,
                      Akonadi::ContactGroupSearchJob::StartsWithMatch );
  // FIXME: ContainsMatch is broken, even though it creates the correct SPARQL query, so use
  //        StartsWith for now
                      //Akonadi::ContactGroupSearchJob::ContainsMatch );
  q->connect( contactJob, SIGNAL(result(KJob *)),
              q, SLOT(slotAkonadiSearchResult(KJob *)) );
  q->connect( groupJob, SIGNAL(result(KJob *)),
              q, SLOT(slotAkonadiSearchResult(KJob *)) );
  akonadiHandlePending();
}

void AddresseeLineEdit::Private::akonadiHandlePending()
{
  kDebug() << "Pending items: " << s_static->akonadiPendingItems.size();
  Akonadi::Item::List::iterator it = s_static->akonadiPendingItems.begin();
  while ( it != s_static->akonadiPendingItems.end() ) {
    const Akonadi::Item item = *it;

    const int sourceIndex =
      s_static->akonadiCollectionToCompletionSourceMap.value( item.parentCollection().id(), -1 );
    if ( sourceIndex >= 0 ) {
      kDebug() << "identified collection: " << s_static->completionSources[sourceIndex];
      q->addItem( item, 1, sourceIndex );

      // remove from the pending
      it = s_static->akonadiPendingItems.erase( it );
    } else {
      ++it;
    }
  }
}

void AddresseeLineEdit::Private::doCompletion( bool ctrlT )
{
  m_lastSearchMode = ctrlT;

  const KGlobalSettings::Completion mode = q->completionMode();

  if ( mode == KGlobalSettings::CompletionNone ) {
    return;
  }

  s_static->completion->setOrder( completionOrder() );

  // cursor at end of string - or Ctrl+T pressed for substring completion?
  if ( ctrlT ) {
    const QStringList completions = adjustedCompletionItems( false );

    if ( completions.count() > 1 ) {
      ; //m_previousAddresses = prevAddr;
    } else if ( completions.count() == 1 ) {
      q->setText( m_previousAddresses + completions.first().trimmed() );
    }

    // Make sure the completion popup is closed if no matching items were found
    setCompletedItems( completions, true );

    q->cursorAtEnd();
    q->setCompletionMode( mode ); //set back to previous mode
    return;
  }

  switch ( mode ) {
  case KGlobalSettings::CompletionPopupAuto:
  {
    if ( m_searchString.isEmpty() ) {
      break;
    }
    //else: fall-through to the CompletionPopup case
  }

  case KGlobalSettings::CompletionPopup:
  {
    const QStringList items = adjustedCompletionItems( true );
    setCompletedItems( items, false );
  }
  break;

  case KGlobalSettings::CompletionShell:
  {
    const QString match = s_static->completion->makeCompletion( m_searchString );
    if ( !match.isNull() && match != m_searchString ) {
      q->setText( m_previousAddresses + match );
      q->setModified( true );
      q->cursorAtEnd();
    }
  }
  break;

  case KGlobalSettings::CompletionMan: // Short-Auto in fact
  case KGlobalSettings::CompletionAuto:
  {
    //force autoSuggest in KLineEdit::keyPressed or setCompletedText will have no effect
    q->setCompletionMode( q->completionMode() );

    if ( !m_searchString.isEmpty() ) {

      //if only our \" is left, remove it since user has not typed it either
      if ( m_searchExtended && m_searchString == QLatin1String( "\"" ) ) {
        m_searchExtended = false;
        m_searchString.clear();
        q->setText( m_previousAddresses );
        break;
      }

      QString match = s_static->completion->makeCompletion( m_searchString );

      if ( !match.isEmpty() ) {
        if ( match != m_searchString ) {
          QString adds = m_previousAddresses + match;
          q->setCompletedText( adds );
        }
      } else {
        if ( !m_searchString.startsWith( QLatin1Char( '\"' ) ) ) {
          //try with quoted text, if user has not type one already
          match = s_static->completion->makeCompletion( QLatin1String( "\"" ) + m_searchString );
          if ( !match.isEmpty() && match != m_searchString ) {
            m_searchString = QLatin1String( "\"" ) + m_searchString;
            m_searchExtended = true;
            q->setText( m_previousAddresses + m_searchString );
            q->setCompletedText( m_previousAddresses + match );
          }
        } else if ( m_searchExtended ) {
          //our added \" does not work anymore, remove it
          m_searchString = m_searchString.mid( 1 );
          m_searchExtended = false;
          q->setText( m_previousAddresses + m_searchString );
          //now try again
          match = s_static->completion->makeCompletion( m_searchString );
          if ( !match.isEmpty() && match != m_searchString ) {
            const QString adds = m_previousAddresses + match;
            q->setCompletedText( adds );
          }
        }
      }
    }
  }
  break;

  case KGlobalSettings::CompletionNone:
  default: // fall through
    break;
  }
}

void AddresseeLineEdit::Private::slotCompletion()
{
  // Called by KLineEdit's keyPressEvent for CompletionModes
  // Auto,Popup -> new text, update search string.
  // not called for CompletionShell, this is been taken care of
  // in AddresseeLineEdit::keyPressEvent

  updateSearchString();
  if ( q->completionBox() ) {
    q->completionBox()->setCancelledText( m_searchString );
  }

  akonadiPerformSearch();
  doCompletion( false );
}

void AddresseeLineEdit::Private::slotPopupCompletion( const QString &completion )
{
  q->setText( m_previousAddresses + completion.trimmed() );
  q->cursorAtEnd();
  updateSearchString();
  q->emitTextCompleted();
}

void AddresseeLineEdit::Private::slotReturnPressed( const QString & )
{
  if ( !q->completionBox()->selectedItems().isEmpty() ) {
    slotPopupCompletion( q->completionBox()->selectedItems().first()->text() );
  }
}

void AddresseeLineEdit::Private::slotStartLDAPLookup()
{
  if ( Solid::Networking::status() == Solid::Networking::Unconnected ) {
    return;
  }

  const KGlobalSettings::Completion mode = q->completionMode();

  if ( mode == KGlobalSettings::CompletionNone ) {
    return;
  }

  if ( !s_static->ldapSearch->isAvailable() ) {
    return;
  }

  if ( s_static->ldapLineEdit != q ) {
    return;
  }

  startLoadingLDAPEntries();
}

void AddresseeLineEdit::Private::slotLDAPSearchData( const KLDAP::LdapResult::List &results )
{
  if ( results.isEmpty() || s_static->ldapLineEdit != q ) {
    return;
  }

  foreach ( const KLDAP::LdapResult &result, results ) {
    KABC::Addressee contact;
    contact.setNameFromString( result.name );
    contact.setEmails( result.email );

    if ( !s_static->ldapClientToCompletionSourceMap.contains( result.clientNumber ) ) {
      updateLDAPWeights(); // we got results from a new source, so update the completion sources
    }

    q->addContact( contact, result.completionWeight,
                   s_static->ldapClientToCompletionSourceMap[ result.clientNumber ] );
  }

  if ( ( q->hasFocus() || q->completionBox()->hasFocus() ) &&
       q->completionMode() != KGlobalSettings::CompletionNone &&
       q->completionMode() != KGlobalSettings::CompletionShell ) {
    q->setText( m_previousAddresses + m_searchString );
    // only complete again if the user didn't change the selection while
    // we were waiting; otherwise the completion box will be closed
    const QListWidgetItem *current = q->completionBox()->currentItem();
    if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
      doCompletion( m_lastSearchMode );
    }
  }
}

void AddresseeLineEdit::Private::slotEditCompletionOrder()
{
  init(); // for s_static->ldapSearch
#ifndef Q_OS_WINCE
  CompletionOrderEditor editor( s_static->ldapSearch, q );
  editor.exec();
#endif
  if ( m_useCompletion ) {
    updateLDAPWeights();
  }
}

void AddresseeLineEdit::Private::slotUserCancelled( const QString &cancelText )
{
  if ( s_static->ldapSearch && s_static->ldapLineEdit == q ) {
    stopLDAPLookup();
  }

  q->userCancelled( m_previousAddresses + cancelText ); // in KLineEdit
}

void AddresseeLineEdit::Private::slotAkonadiSearchResult( KJob *job )
{
  const Akonadi::ContactSearchJob *contactJob =
    qobject_cast<Akonadi::ContactSearchJob*>( job );
  const Akonadi::ContactGroupSearchJob *groupJob =
    qobject_cast<Akonadi::ContactGroupSearchJob*>( job );

  if ( contactJob ) {
    kDebug() << "Found" << contactJob->contacts().size() << "contacts";
  } else if ( groupJob ) {
    kDebug() << "Found" << groupJob->contactGroups().size() << "groups";
  }

  /* We have to fetch the collections of the items, so that
     the source name can be correctly labeled.*/
  Akonadi::Item::List items;
  if ( contactJob ) items += contactJob->items();
  if ( groupJob ) items += groupJob->items();
  foreach ( const Akonadi::Item &item, items ) {

    // check the local cache of collections
    const int sourceIndex =
      s_static->akonadiCollectionToCompletionSourceMap.value( item.parentCollection().id(), -1 );
    if ( sourceIndex == -1 ) {
      kDebug() << "Fetching New collection: " << item.parentCollection().id();
      // the collection isn't there, start the fetch job.
      Akonadi::CollectionFetchJob *collectionJob =
        new Akonadi::CollectionFetchJob( item.parentCollection(),
                                         Akonadi::CollectionFetchJob::Base, q );
      connect( collectionJob, SIGNAL(collectionsReceived(const Akonadi::Collection::List &)),
               q, SLOT(slotAkonadiCollectionsReceived(const Akonadi::Collection::List &)) );
      /* we don't want to start multiple fetch jobs for the same collection,
         so insert the collection with an index value of -2 */
      s_static->akonadiCollectionToCompletionSourceMap.insert( item.parentCollection().id(), -2 );
      s_static->akonadiPendingItems.append( item );
    } else if ( sourceIndex == -2 ) {
      /* fetch job already started, don't need to start another one,
         so just append the item as pending */
      s_static->akonadiPendingItems.append( item );
    } else {
      q->addItem( item, 1, sourceIndex );
    }
  }

  if ( ( contactJob && contactJob->contacts().size() > 0 ) ||
       ( groupJob && groupJob->contactGroups().size() > 0 ) ) {
    const QListWidgetItem *current = q->completionBox()->currentItem();
    if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
      doCompletion( m_lastSearchMode );
    }
  }
}

void AddresseeLineEdit::Private::slotAkonadiCollectionsReceived(
  const Akonadi::Collection::List &collections )
{
  foreach ( const Akonadi::Collection &collection, collections ) {
    if ( collection.isValid() ) {
      const Akonadi::EntityDisplayAttribute *attribute =
        collection.attribute<Akonadi::EntityDisplayAttribute>();

      QString sourceString = collection.name();
      if ( attribute && !attribute->displayName().isEmpty() )
        sourceString = attribute->displayName();

      const int index = q->addCompletionSource( sourceString, 1 );
      kDebug() << "\treceived: " << sourceString << "index: " << index;
      s_static->akonadiCollectionToCompletionSourceMap.insert( collection.id(), index );
    }
  }

  // now that we have added the new collections, recheck our list of pending contacts
  akonadiHandlePending();
  // do completion
  const QListWidgetItem *current = q->completionBox()->currentItem();
  if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
    doCompletion( m_lastSearchMode );
  }
}

// not cached, to make sure we get an up-to-date value when it changes
KCompletion::CompOrder AddresseeLineEdit::Private::completionOrder()
{
  KConfig _config( QLatin1String( "kpimcompletionorder" ) );
  const KConfigGroup config( &_config, QLatin1String( "General" ) );
  const QString order =
    config.readEntry( QLatin1String( "CompletionOrder" ), QString::fromLatin1( "Weighted" ) );

  if ( order == QLatin1String( "Weighted" ) ) {
    return KCompletion::Weighted;
  } else {
    return KCompletion::Sorted;
  }
}

AddresseeLineEdit::AddresseeLineEdit( QWidget *parent, bool enableCompletion )
  : KLineEdit( parent ), d( new Private( this, enableCompletion ) )
{
  setObjectName( newLineEditObjectName() );
  setClickMessage( QString() );

  d->init();
}

AddresseeLineEdit::~AddresseeLineEdit()
{
  if ( s_static->ldapSearch && s_static->ldapLineEdit == this ) {
    d->stopLDAPLookup();
  }

  delete d;
}

void AddresseeLineEdit::setFont( const QFont &font )
{
  KLineEdit::setFont( font );

  if ( d->m_useCompletion ) {
    completionBox()->setFont( font );
  }
}

void AddresseeLineEdit::allowSemicolonAsSeparator( bool useSemicolonAsSeparator )
{
  d->m_useSemicolonAsSeparator = useSemicolonAsSeparator;
}

void AddresseeLineEdit::keyPressEvent( QKeyEvent *event )
{
  bool accept = false;

  const int key = event->key() | event->modifiers();

  if ( KStandardShortcut::shortcut( KStandardShortcut::SubstringCompletion ).contains( key ) ) {
    //TODO: add LDAP substring lookup, when it becomes available in KPIM::LDAPSearch
    d->updateSearchString();
    d->akonadiPerformSearch();
    d->doCompletion( true );
    accept = true;
  } else if ( KStandardShortcut::shortcut( KStandardShortcut::TextCompletion ).contains( key ) ) {
    const int len = text().length();

    if ( len == cursorPosition() ) { // at End?
      d->updateSearchString();
      d->akonadiPerformSearch();
      d->doCompletion( true );
      accept = true;
    }
  }

  const QString oldContent = text();
  if ( !accept ) {
    KLineEdit::keyPressEvent( event );
  }

  // if the text didn't change (eg. because a cursor navigation key was pressed)
  // we don't need to trigger a new search
  if ( oldContent == text() ) {
    return;
  }

  if ( event->isAccepted() ) {
    d->updateSearchString();

    QString searchString( d->m_searchString );
    //LDAP does not know about our string manipulation, remove it
    if ( d->m_searchExtended ) {
      searchString = d->m_searchString.mid( 1 );
    }

    if ( d->m_useCompletion && s_static->ldapTimer ) {
      if ( s_static->ldapText != searchString || s_static->ldapLineEdit != this ) {
        d->stopLDAPLookup();
      }

      s_static->ldapText = searchString;
      s_static->ldapLineEdit = this;
      s_static->ldapTimer->setSingleShot( true );
      s_static->ldapTimer->start( 500 );
    }
  }
}

void AddresseeLineEdit::insert( const QString &t )
{
  if ( !d->m_smartPaste ) {
    KLineEdit::insert( t );
    return;
  }

  QString newText = t.trimmed();
  if ( newText.isEmpty() ) {
    return;
  }

  // remove newlines in the to-be-pasted string
  QStringList lines = newText.split( QRegExp( QLatin1String( "\r?\n" ) ), QString::SkipEmptyParts );
  for ( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
    // remove trailing commas and whitespace
    (*it).remove( QRegExp( QLatin1String( ",?\\s*$" ) ) );
  }
  newText = lines.join( QLatin1String( ", " ) );

  if ( newText.startsWith( QLatin1String( "mailto:" ) ) ) {
    const KUrl url( newText );
    newText = url.path();
  } else if ( newText.indexOf( QLatin1String( " at " ) ) != -1 ) {
    // Anti-spam stuff
    newText.replace( QLatin1String( " at " ), QLatin1String( "@" ) );
    newText.replace( QLatin1String( " dot " ), QLatin1String( "." ) );
  } else if ( newText.indexOf( QLatin1String( "(at)" ) ) != -1 ) {
    newText.replace( QRegExp( QLatin1String( "\\s*\\(at\\)\\s*" ) ), QLatin1String( "@" ) );
  }

  QString contents = text();
  int start_sel = 0;
  int pos = cursorPosition();

  if ( hasSelectedText() ) {
    // Cut away the selection.
    start_sel = selectionStart();
    pos = start_sel;
    contents = contents.left( start_sel ) + contents.mid( start_sel + selectedText().length() );
  }

  int eot = contents.length();
  while ( ( eot > 0 ) && contents[ eot - 1 ].isSpace() ) {
    eot--;
  }
  if ( eot == 0 ) {
    contents.clear();
  } else if ( pos >= eot ) {
    if ( contents[ eot - 1 ] == QLatin1Char( ',' ) ) {
      eot--;
    }
    contents.truncate( eot );
    contents += QLatin1String( ", " );
    pos = eot + 2;
  }

  contents = contents.left( pos ) + newText + contents.mid( pos );
  setText( contents );
  setModified( true );
  setCursorPosition( pos + newText.length() );
}

void AddresseeLineEdit::setText( const QString & text )
{
  KLineEdit::setText( text.trimmed() );
}

void AddresseeLineEdit::paste()
{
  if ( d->m_useCompletion ) {
    d->m_smartPaste = true;
  }

#ifndef QT_NO_CLIPBOARD
  KLineEdit::paste();
#endif
  d->m_smartPaste = false;
}

void AddresseeLineEdit::mouseReleaseEvent( QMouseEvent *event )
{
  // reimplemented from QLineEdit::mouseReleaseEvent()
#ifndef QT_NO_CLIPBOARD
  if ( d->m_useCompletion &&
       QApplication::clipboard()->supportsSelection() &&
       !isReadOnly() &&
       event->button() == Qt::MidButton ) {
    d->m_smartPaste = true;
  }
#endif

  KLineEdit::mouseReleaseEvent( event );
  d->m_smartPaste = false;
}

#ifndef QT_NO_DRAGANDDROP
void AddresseeLineEdit::dropEvent( QDropEvent *event )
{
  if ( !isReadOnly() ) {
    const KUrl::List uriList = KUrl::List::fromMimeData( event->mimeData() );
    if ( !uriList.isEmpty() ) {
      QString contents = text();
      // remove trailing white space and comma
      int eot = contents.length();
      while ( ( eot > 0 ) && contents[ eot - 1 ].isSpace() ) {
        eot--;
      }
      if ( eot == 0 ) {
        contents.clear();
      } else if ( contents[ eot - 1 ] == ',' ) {
        eot--;
        contents.truncate( eot );
      }
      bool mailtoURL = false;
      // append the mailto URLs
      foreach ( const KUrl &url, uriList ) {
        if ( url.protocol() == QLatin1String( "mailto" ) ) {
          mailtoURL = true;
          QString address;
          address = KUrl::fromPercentEncoding( url.path().toLatin1() );
          address = KMime::decodeRFC2047String( address.toAscii() );
          if ( !contents.isEmpty() ) {
            contents.append( QLatin1String( ", " ) );
          }
          contents.append( address );
        }
      }
      if ( mailtoURL ) {
        setText( contents );
        setModified( true );
        return;
      }
    } else {
      // Let's see if this drop contains a comma separated list of emails
      QString dropData = QString::fromUtf8( event->encodedData( "text/plain" ) );
      QStringList addrs = KPIMUtils::splitAddressList( dropData );
      if ( addrs.count() > 0 ) {
        setText( KPIMUtils::normalizeAddressesAndDecodeIdn( dropData ) );
        setModified( true );
        return;
      }
    }
  }

  if ( d->m_useCompletion ) {
    d->m_smartPaste = true;
  }

  QLineEdit::dropEvent( event );
  d->m_smartPaste = false;
}
#endif // QT_NO_DRAGANDDROP

void AddresseeLineEdit::cursorAtEnd()
{
  setCursorPosition( text().length() );
}

void AddresseeLineEdit::enableCompletion( bool enable )
{
  d->m_useCompletion = enable;
}

void AddresseeLineEdit::addItem( const Akonadi::Item &item, int weight, int source )
{
  if ( item.hasPayload<KABC::Addressee>() ) {
    addContact( item.payload<KABC::Addressee>(), weight, source );
  } else if ( item.hasPayload<KABC::ContactGroup>() ) {
    addContactGroup( item.payload<KABC::ContactGroup>(), weight, source );
  }
}

void AddresseeLineEdit::addContactGroup( const KABC::ContactGroup &group, int weight, int source )
{
  d->addCompletionItem( group.name(), weight, source );
  QStringList keyWords;
  keyWords.append( group.name() );
  d->addCompletionItem( group.name(), weight, source, &keyWords );
}

void AddresseeLineEdit::addContact( const KABC::Addressee &addr, int weight, int source )
{
  const QStringList emails = addr.emails();
  QStringList::ConstIterator it;
  const int prefEmailWeight = 1;     //increment weight by prefEmailWeight
  int isPrefEmail = prefEmailWeight; //first in list is preferredEmail
  for ( it = emails.begin(); it != emails.end(); ++it ) {
    //TODO: highlight preferredEmail
    const QString email( (*it) );
    const QString givenName = addr.givenName();
    const QString familyName= addr.familyName();
    const QString nickName  = addr.nickName();
    const QString domain    = email.mid( email.indexOf( '@' ) + 1 );
    QString fullEmail       = addr.fullEmail( email );
    //TODO: let user decide what fields to use in lookup, e.g. company, city, ...
    //for CompletionAuto
    if ( givenName.isEmpty() && familyName.isEmpty() ) {
      d->addCompletionItem( fullEmail, weight + isPrefEmail, source ); // use whatever is there
    } else {
      const QString byFirstName = QLatin1Char( '"' ) + givenName +
                                  QLatin1Char( ' ' ) + familyName +
                                  QLatin1String( "\" <" ) + email + QLatin1Char( '>' );
      const QString byLastName = QLatin1Char( '"' ) + familyName +
                                 QLatin1String( ", " ) + givenName +
                                 QLatin1String( "\" <" ) + email + QLatin1Char( '>' );
      d->addCompletionItem( byFirstName, weight + isPrefEmail, source );
      d->addCompletionItem( byLastName, weight + isPrefEmail, source );
    }

    d->addCompletionItem( email, weight + isPrefEmail, source );

    if ( !nickName.isEmpty() ) {
      const QString byNick = QLatin1Char( '"' ) + nickName +
                             QLatin1String( "\" <" ) + email + QLatin1Char( '>' );
      d->addCompletionItem( byNick, weight + isPrefEmail, source );
    }

    if ( !domain.isEmpty() ) {
      const QString byDomain = QLatin1Char( '"' ) + domain + QLatin1Char( ' ' ) +
                               familyName + QLatin1Char( ' ' ) + givenName +
                               QLatin1String( "\" <" ) + email + QLatin1Char( '>' );
      d->addCompletionItem( byDomain, weight + isPrefEmail, source );
    }

    //for CompletionShell, CompletionPopup
    QStringList keyWords;
    const QString realName  = addr.realName();

    if ( !givenName.isEmpty() && !familyName.isEmpty() ) {
      keyWords.append( givenName  + QLatin1Char( ' ' ) + familyName );
      keyWords.append( familyName + QLatin1Char( ' ' ) + givenName );
      keyWords.append( familyName + QLatin1String( ", " ) + givenName );
    } else if ( !givenName.isEmpty() ) {
      keyWords.append( givenName );
    } else if ( !familyName.isEmpty() ) {
      keyWords.append( familyName );
    }

    if ( !nickName.isEmpty() ) {
      keyWords.append( nickName );
    }

    if ( !realName.isEmpty() ) {
      keyWords.append( realName );
    }

    if ( !domain.isEmpty() ) {
      keyWords.append( domain );
    }

    keyWords.append( email );

    /* KMailCompletion does not have knowledge about identities, it stores emails and
     * keywords for each email. KMailCompletion::allMatches does a lookup on the
     * keywords and returns an ordered list of emails. In order to get the preferred
     * email before others for each identity we use this little trick.
     * We remove the <blank> in adjustedCompletionItems.
     */
    if ( isPrefEmail == prefEmailWeight ) {
      fullEmail.replace( QLatin1String( " <" ), QLatin1String( "  <" ) );
    }

    d->addCompletionItem( fullEmail, weight + isPrefEmail, source, &keyWords );
    isPrefEmail = 0;
  }
}

#ifndef QT_NO_CONTEXTMENU
void AddresseeLineEdit::contextMenuEvent( QContextMenuEvent *event )
{
  QMenu *menu = createStandardContextMenu();
  if ( menu ) { // can be 0 on platforms with only a touch interface
    menu->exec( event->globalPos() );
    delete menu;
  }
}

QMenu *AddresseeLineEdit::createStandardContextMenu()
{
  // disable modes not supported by KMailCompletion
  setCompletionModeDisabled( KGlobalSettings::CompletionMan );
  setCompletionModeDisabled( KGlobalSettings::CompletionPopupAuto );

  QMenu *menu = KLineEdit::createStandardContextMenu();
  if ( !menu ) {
    return 0;
  }

  if ( d->m_useCompletion ) {
    menu->addAction( i18n( "Configure Completion Order..." ),
                     this, SLOT(slotEditCompletionOrder()) );
  }
  return menu;
}
#endif

int KPIM::AddresseeLineEdit::addCompletionSource( const QString &source, int weight )
{
  QMap<QString,int>::iterator it = s_static->completionSourceWeights.find( source );
  if ( it == s_static->completionSourceWeights.end() ) {
    s_static->completionSourceWeights.insert( source, weight );
  } else {
    s_static->completionSourceWeights[source] = weight;
  }

  const int sourceIndex = s_static->completionSources.indexOf( source );
  if ( sourceIndex == -1 ) {
    s_static->completionSources.append( source );
    return s_static->completionSources.size() - 1;
  } else {
    return sourceIndex;
  }
}

bool KPIM::AddresseeLineEdit::eventFilter( QObject *object, QEvent *event )
{
  if ( d->m_completionInitialized &&
       ( object == completionBox() ||
         completionBox()->findChild<QWidget*>( object->objectName() ) == object ) ) {
    if ( event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseMove ||
         event->type() == QEvent::MouseButtonRelease ||
         event->type() == QEvent::MouseButtonDblClick ) {

      const QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
      // find list box item at the event position
      QListWidgetItem *item = completionBox()->itemAt( mouseEvent->pos() );
      if ( !item ) {
        // In the case of a mouse move outside of the box we don't want
        // the parent to fuzzy select a header by mistake.
        bool eat = event->type() == QEvent::MouseMove;
        return eat;
      }
      // avoid selection of headers on button press, or move or release while
      // a button is pressed
      const Qt::MouseButtons buttons = mouseEvent->buttons();
      if ( event->type() == QEvent::MouseButtonPress ||
           event->type() == QEvent::MouseButtonDblClick ||
           buttons & Qt::LeftButton || buttons & Qt::MidButton ||
           buttons & Qt::RightButton ) {
        if ( itemIsHeader( item ) ) {
          return true; // eat the event, we don't want anything to happen
        } else {
          // if we are not on one of the group heading, make sure the item
          // below or above is selected, not the heading, inadvertedly, due
          // to fuzzy auto-selection from QListBox
          completionBox()->setCurrentItem( item );
          item->setSelected( true );
          if ( event->type() == QEvent::MouseMove ) {
            return true; // avoid fuzzy selection behavior
          }
        }
      }
    }
  }

  if ( ( object == this ) &&
     ( event->type() == QEvent::ShortcutOverride ) ) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
    if ( keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down ||
         keyEvent->key() == Qt::Key_Tab ) {
      keyEvent->accept();
      return true;
    }
  }

  if ( ( object == this ) &&
      ( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease ) &&
      completionBox()->isVisible() ) {
    const QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
    int currentIndex = completionBox()->currentRow();
    if ( currentIndex < 0 ) {
      return true;
    }
    if ( keyEvent->key() == Qt::Key_Up ) {
      //kDebug() <<"EVENTFILTER: Qt::Key_Up currentIndex=" << currentIndex;
      // figure out if the item we would be moving to is one we want
      // to ignore. If so, go one further
      const QListWidgetItem *itemAbove = completionBox()->item( currentIndex );
      if ( itemAbove && itemIsHeader( itemAbove ) ) {
        // there is a header above is, check if there is even further up
        // and if so go one up, so it'll be selected
        if ( currentIndex > 0 && completionBox()->item( currentIndex - 1 ) ) {
          //kDebug() <<"EVENTFILTER: Qt::Key_Up -> skipping" << currentIndex - 1;
          completionBox()->setCurrentRow( currentIndex - 1 );
          completionBox()->item( currentIndex - 1 )->setSelected( true );
        } else if ( currentIndex == 0 ) {
          // nothing to skip to, let's stay where we are, but make sure the
          // first header becomes visible, if we are the first real entry
          completionBox()->scrollToItem( completionBox()->item( 0 ) );
          QListWidgetItem *item = completionBox()->item( currentIndex );
          if ( item ) {
            if ( itemIsHeader( item ) ) {
              currentIndex++;
              item = completionBox()->item( currentIndex );
            }
            completionBox()->setCurrentItem( item );
            item->setSelected( true );
          }
        }

        return true;
      }
    } else if ( keyEvent->key() == Qt::Key_Down ) {
      // same strategy for downwards
      //kDebug() <<"EVENTFILTER: Qt::Key_Down. currentIndex=" << currentIndex;
      const QListWidgetItem *itemBelow = completionBox()->item( currentIndex );
      if ( itemBelow && itemIsHeader( itemBelow ) ) {
        if ( completionBox()->item( currentIndex + 1 ) ) {
          //kDebug() <<"EVENTFILTER: Qt::Key_Down -> skipping" << currentIndex+1;
          completionBox()->setCurrentRow( currentIndex + 1 );
          completionBox()->item( currentIndex + 1 )->setSelected( true );
        } else {
          // nothing to skip to, let's stay where we are
          QListWidgetItem *item = completionBox()->item( currentIndex );
          if ( item ) {
            completionBox()->setCurrentItem( item );
            item->setSelected( true );
          }
        }

        return true;
      }
      // special case of the initial selection, which is unfortunately a header.
      // Setting it to selected tricks KCompletionBox into not treating is special
      // and selecting making it current, instead of the one below.
      QListWidgetItem *item = completionBox()->item( currentIndex );
      if ( item && itemIsHeader( item ) ) {
        completionBox()->setCurrentItem( item );
        item->setSelected( true );
      }
    } else if ( event->type() == QEvent::KeyRelease &&
                ( keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab ) ) {
      /// first, find the header of the current section
      QListWidgetItem *myHeader = 0;
      int myHeaderIndex = -1;
      const int iterationStep = keyEvent->key() == Qt::Key_Tab ?  1 : -1;
      int index = qMin( qMax( currentIndex - iterationStep, 0 ), completionBox()->count() - 1 );
      while ( index >= 0 ) {
        if ( itemIsHeader( completionBox()->item( index ) ) ) {
          myHeader = completionBox()->item( index );
          myHeaderIndex = index;
          break;
        }

        index--;
      }
      Q_ASSERT( myHeader ); // we should always be able to find a header

      // find the next header (searching backwards, for Qt::Key_Backtab)
      QListWidgetItem *nextHeader = 0;

      // when iterating forward, start at the currentindex, when backwards,
      // one up from our header, or at the end
      uint j;
      if ( keyEvent->key() == Qt::Key_Tab ) {
        j = currentIndex;
      } else {
        index = myHeaderIndex;
        if ( index == 0 ) {
          j = completionBox()->count() - 1;
        } else {
          j = ( index - 1 ) % completionBox()->count();
        }
      }
      while ( ( nextHeader = completionBox()->item( j ) ) && nextHeader != myHeader ) {
          if ( itemIsHeader( nextHeader ) ) {
            break;
          }
          j = ( j + iterationStep ) % completionBox()->count();
      }

      if ( nextHeader && nextHeader != myHeader ) {
        QListWidgetItem *item = completionBox()->item( j + 1 );
        if ( item && !itemIsHeader( item ) ) {
          completionBox()->setCurrentItem( item );
          item->setSelected( true );
        }
      }

      return true;
    }
  }

  return KLineEdit::eventFilter( object, event );
}

void AddresseeLineEdit::emitTextCompleted()
{
    emit textCompleted();
}


#include "addresseelineedit.moc"
