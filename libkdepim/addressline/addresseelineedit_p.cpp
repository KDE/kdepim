/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "completionordereditor.h"
#include "kmailcompletion.h"

#include <QMap>


namespace KPIM {
typedef QMap< QString, QPair<int,int> > CompletionItemsMap;
}

namespace KPIM {
class AddresseeLineEditStatic
{
public:

    AddresseeLineEditStatic()
        : completion( new KMailCompletion ),
          ldapTimer( 0 ),
          ldapSearch( 0 ),
          ldapLineEdit( 0 ),
          akonadiSession( new Akonadi::Session("contactsCompletionSession") ),
          balooCompletionSource( 0 )
    {
    }

    ~AddresseeLineEditStatic()
    {
        delete completion;
        delete ldapTimer;
        delete ldapSearch;
    }

    void slotEditCompletionOrder()
    {
        CompletionOrderEditor editor( ldapSearch, 0 );
        if( editor.exec() ) {
            updateLDAPWeights();
            updateCollectionWeights();
        }
    }

    void updateCollectionWeights()
    {
        akonadiCollectionToCompletionSourceMap.clear();
    }

    void updateLDAPWeights()
    {
        /* Add completion sources for all ldap server, 0 to n. Added first so
       * that they map to the LdapClient::clientNumber() */
        ldapSearch->updateCompletionWeights();
        int clientIndex = 0;
        foreach ( const KLDAP::LdapClient *client, ldapSearch->clients() ) {
            const int sourceIndex =
                    addCompletionSource( i18n( "LDAP server: %1" ,client->server().host()),
                                         client->completionWeight() );

            ldapClientToCompletionSourceMap.insert( clientIndex, sourceIndex );

            ++clientIndex;
        }
    }

    int addCompletionSource( const QString &source, int weight )
    {
        QMap<QString,int>::iterator it = completionSourceWeights.find( source );
        if ( it == completionSourceWeights.end() ) {
            completionSourceWeights.insert( source, weight );
        } else {
            completionSourceWeights[source] = weight;
        }

        const int sourceIndex = completionSources.indexOf( source );
        if ( sourceIndex == -1 ) {
            completionSources.append( source );
            return completionSources.size() - 1;
        } else {
            return sourceIndex;
        }
    }

    void removeCompletionSource( const QString &source )
    {
        QMap<QString,int>::iterator it = completionSourceWeights.find( source );
        if ( it != completionSourceWeights.end() ) {
            completionSourceWeights.remove(source);
            completion->clear();
        }
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
    Akonadi::Session *akonadiSession;
    QVector<QWeakPointer<Akonadi::Job> > akonadiJobsInFlight;
    int balooCompletionSource;
};

K_GLOBAL_STATIC( AddresseeLineEditStatic, s_static )

// needs to be unique, but the actual name doesn't matter much
static QString newLineEditObjectName()
{
    static int s_count = 0;
    QString name( QLatin1String("KPIM::AddresseeLineEdit") );
    if ( s_count++ ) {
        name += QLatin1Char('-');
        name += QString::number( s_count );
    }
    return name;
}

static const QString s_completionItemIndentString = QLatin1String("     ");

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
          m_lastSearchMode( false ),
          m_searchExtended( false ),
          m_useSemicolonAsSeparator( false ),
          m_showOU(false),
          m_enableBalooSearch(true)
    {
        m_delayedQueryTimer.setSingleShot(true);
        connect( &m_delayedQueryTimer, SIGNAL(timeout()), q, SLOT(slotTriggerDelayedQueries()) );
    }
    QStringList cleanupEmailList(const QStringList &inputList);
    void loadBalooBlackList();
    void alternateColor();
    void init();
    void startLoadingLDAPEntries();
    void stopLDAPLookup();
    void setCompletedItems( const QStringList &items, bool autoSuggest );
    void addCompletionItem( const QString &string, int weight, int source,
                            const QStringList *keyWords = 0 );
    const QStringList adjustedCompletionItems( bool fullSearch );
    void updateSearchString();
    void startSearches();
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
    void slotAkonadiHandleItems( const Akonadi::Item::List &items );
    void slotAkonadiSearchResult( KJob * );
    void slotAkonadiCollectionsReceived( const Akonadi::Collection::List & );
    void searchInBaloo();
    void slotTriggerDelayedQueries();
    void slotShowOUChanged( bool );
    void slotConfigureBalooBlackList();

    AddresseeLineEdit *q;
    QStringList m_balooBlackList;
    QStringList m_domainExcludeList;
    QString m_previousAddresses;
    QString m_searchString;
    bool m_useCompletion;
    bool m_completionInitialized;
    bool m_smartPaste;
    bool m_addressBookConnected;
    bool m_lastSearchMode;
    bool m_searchExtended; //has \" been added?
    bool m_useSemicolonAsSeparator;
    bool m_showOU;
    bool m_enableBalooSearch;
    QTimer m_delayedQueryTimer;
    QColor m_alternateColor;
};

void AddresseeLineEdit::Private::init()
{
    if ( !s_static.exists() ) {
        s_static->completion->setOrder( KCompletion::Weighted );
        s_static->completion->setIgnoreCase( true );
    }

    if ( m_useCompletion ) {
        if ( !s_static->ldapTimer ) {
            s_static->ldapTimer = new QTimer;
            s_static->ldapSearch = new KLDAP::LdapClientSearch;

            /* The reasoning behind this filter is:
             * If it's a person, or a distlist, show it, even if it doesn't have an email address.
             * If it's not a person, or a distlist, only show it if it has an email attribute.
             * This allows both resource accounts with an email address which are not a person and
             * person entries without an email address to show up, while still not showing things
             * like structural entries in the ldap tree.
             */

            #if 0
                s_static->ldapSearch->setFilter(QString::fromLatin1("&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                                          "(|(cn=%1*)(mail=%1*)(mail=*@%1*)(givenName=%1*)(sn=%1*))"));
            #endif
            //Fix bug 323272 "Exchange doesn't like any queries beginning with *."
            s_static->ldapSearch->setFilter(QString::fromLatin1( "&(|(objectclass=person)(objectclass=groupOfNames)(mail=*))"
                                "(|(cn=%1*)(mail=%1*)(givenName=%1*)(sn=%1*))" ));

        }

        s_static->balooCompletionSource = q->addCompletionSource( i18nc( "@title:group", "Contacts found in your data"), -1 );

        s_static->updateLDAPWeights();

        if ( !m_completionInitialized ) {
            q->setCompletionObject( s_static->completion, false );
            q->connect( q, SIGNAL(completion(QString)),
                        q, SLOT(slotCompletion()) );
            q->connect( q, SIGNAL(returnPressed(QString)),
                        q, SLOT(slotReturnPressed(QString)) );

            KCompletionBox *box = q->completionBox();
            q->connect( box, SIGNAL(activated(QString)),
                        q, SLOT(slotPopupCompletion(QString)) );
            q->connect( box, SIGNAL(userCancelled(QString)),
                        q, SLOT(slotUserCancelled(QString)) );

            q->connect( s_static->ldapTimer, SIGNAL(timeout()), SLOT(slotStartLDAPLookup()) );
            q->connect( s_static->ldapSearch, SIGNAL(searchData(KLDAP::LdapResult::List)),
                        SLOT(slotLDAPSearchData(KLDAP::LdapResult::List)) );

            m_completionInitialized = true;
        }

        KConfigGroup group( KGlobal::config(), "AddressLineEdit" );
        m_showOU = group.readEntry( "ShowOU", false );
        loadBalooBlackList();
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

QStringList AddresseeLineEdit::cleanupEmailList(const QStringList &inputList)
{
    return d->cleanupEmailList(inputList);
}

QStringList AddresseeLineEdit::Private::cleanupEmailList(const QStringList &inputList)
{
    KPIM::BalooCompletionEmail completionEmail;
    completionEmail.setEmailList(inputList);
    completionEmail.setBlackList(m_balooBlackList);
    completionEmail.setExcludeDomain(m_domainExcludeList);
    const QStringList listEmail = completionEmail.cleanupEmailList();
    return listEmail;
}

void AddresseeLineEdit::Private::searchInBaloo()
{
    const QString trimmedString = m_searchString.trimmed();
    Baloo::PIM::ContactCompleter com(trimmedString, 20);
    const QStringList listEmail = cleanupEmailList(com.complete());
    Q_FOREACH (const QString& email, listEmail) {
        addCompletionItem(email, 1, s_static->balooCompletionSource);
    }
    doCompletion( m_lastSearchMode );
    //  if ( q->hasFocus() || q->completionBox()->hasFocus() ) {
    //}
}

void AddresseeLineEdit::Private::alternateColor()
{
    const KColorScheme colorScheme( QPalette::Active, KColorScheme::View );
    m_alternateColor = colorScheme.background(KColorScheme::AlternateBackground).color();
}

void AddresseeLineEdit::Private::setCompletedItems( const QStringList &items, bool autoSuggest )
{
    KCompletionBox *completionBox = q->completionBox();

    if ( !items.isEmpty() &&
         !( items.count() == 1 && m_searchString == items.first() ) ) {

        completionBox->clear();
        const int numberOfItems( items.count() );
        for ( int i = 0; i< numberOfItems; ++i )
        {
            QListWidgetItem *item =new QListWidgetItem(items.at( i ), completionBox);
            if ( !items.at( i ).startsWith( s_completionItemIndentString ) ) {
                if (!m_alternateColor.isValid()) {
                    alternateColor();
                }
                item->setBackgroundColor(m_alternateColor);
            }
            completionBox->addItem( item );
        }
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
    // We also update the source since the item should always be shown from the source with the highest weight

    CompletionItemsMap::iterator it = s_static->completionItemMap.find( string );
    if ( it != s_static->completionItemMap.end() ) {
        weight = qMax( ( *it ).first, weight );
        ( *it ).first = weight;
        ( *it ).second = completionItemSource;
    } else {
        s_static->completionItemMap.insert( string, qMakePair( weight, completionItemSource ) );
    }

    s_static->completion->addItem(string, weight);
    if (keyWords && !keyWords->isEmpty())
        s_static->completion->addItemWithKeys(string, weight, keyWords); // see kmailcompletion.cpp
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
                const QString sourceLabel( s_static->completionSources.at( index ) );
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
        const int numberOfCompletionSources(s_static->completionSources.size());
        for ( int i = 0; i < numberOfCompletionSources; ++i ) {
            SourceWithWeight sww;
            sww.sourceName = s_static->completionSources.at(i);
            sww.weight = s_static->completionSourceWeights[sww.sourceName];
            sww.index = i;
            sourcesAndWeights.append( sww );
        }

        qSort( sourcesAndWeights.begin(), sourcesAndWeights.end() );
        // Add the sections and their items to the final sortedItems result list
        const int numberOfSources(sourcesAndWeights.size());
        for ( int i = 0; i < numberOfSources; ++i ) {
            const SourceWithWeight source = sourcesAndWeights.at(i);
            const QStringList sectionItems = sections[source.index];
            if ( !sectionItems.isEmpty() ) {
                sortedItems.append( source.sourceName );
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
    const int searchStringLength = m_searchString.length();
    for ( int i = 0; i < searchStringLength; ++i ) {
        const QChar searchChar = m_searchString.at( i );
        if ( searchChar == QLatin1Char( '"' ) ) {
            inQuote = !inQuote;
        }

        if ( searchChar == QLatin1Char('\\') &&
             ( i + 1 ) < searchStringLength && m_searchString.at( i + 1 ) == QLatin1Char( '"' ) ) {
            ++i;
        }

        if ( inQuote ) {
            continue;
        }

        if ( i < searchStringLength &&
             ( searchChar == QLatin1Char(',') ||
               ( m_useSemicolonAsSeparator && searchChar == QLatin1Char(';') ) ) ) {
            n = i;
        }
    }

    if ( n >= 0 ) {
        ++n; // Go past the ","

        const int len = m_searchString.length();

        // Increment past any whitespace...
        while ( n < len && m_searchString.at( n ).isSpace() ) {
            ++n;
        }

        m_previousAddresses = m_searchString.left( n );
        m_searchString = m_searchString.mid( n ).trimmed();
    } else {
        m_previousAddresses.clear();
    }
}

void AddresseeLineEdit::Private::slotTriggerDelayedQueries()
{
    if (m_searchString.isEmpty() || m_searchString.trimmed().size() <= 2)
        return;

    if (m_enableBalooSearch) {
       searchInBaloo();
    }

    // We send a contactsearch job through akonadi.
    // This not only searches baloo but also servers if remote search is enabled
    akonadiPerformSearch();
}

void AddresseeLineEdit::Private::startSearches()
{
    if (!m_delayedQueryTimer.isActive())
        m_delayedQueryTimer.start(50);
}

void AddresseeLineEdit::Private::akonadiPerformSearch()
{
    kDebug() << "searching akonadi with:" << m_searchString;

    // first, kill all job still in flight, they are no longer current
    Q_FOREACH( QWeakPointer<Akonadi::Job> job, s_static->akonadiJobsInFlight ) {
        if ( !job.isNull() ) {
            job.data()->kill();
        }
    }
    s_static->akonadiJobsInFlight.clear();

    // now start new jobs
    Akonadi::ContactSearchJob *contactJob = new Akonadi::ContactSearchJob( s_static->akonadiSession );
    contactJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    contactJob->setQuery( Akonadi::ContactSearchJob::NameOrEmail, m_searchString,
                          Akonadi::ContactSearchJob::ContainsWordBoundaryMatch );
    q->connect( contactJob, SIGNAL(itemsReceived(Akonadi::Item::List)),
                q, SLOT(slotAkonadiHandleItems(Akonadi::Item::List)) );
    q->connect( contactJob, SIGNAL(result(KJob*)),
                q, SLOT(slotAkonadiSearchResult(KJob*)) );

    Akonadi::ContactGroupSearchJob *groupJob = new Akonadi::ContactGroupSearchJob( s_static->akonadiSession );
    groupJob->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
    groupJob->setQuery( Akonadi::ContactGroupSearchJob::Name, m_searchString,
                        Akonadi::ContactGroupSearchJob::ContainsMatch );
    q->connect( contactJob, SIGNAL(itemsReceived(Akonadi::Item::List)),
                q, SLOT(slotAkonadiHandleItems(Akonadi::Item::List)) );
    q->connect( groupJob, SIGNAL(result(KJob*)),
                q, SLOT(slotAkonadiSearchResult(KJob*)) );

    s_static->akonadiJobsInFlight.append( contactJob );
    s_static->akonadiJobsInFlight.append( groupJob );
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

    s_static->completion->setOrder( KCompletion::Weighted );

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
        const QStringList items = adjustedCompletionItems( false );
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

    startSearches();
    doCompletion( false );
}

void AddresseeLineEdit::Private::slotPopupCompletion( const QString &completion )
{
    QString c = completion.trimmed();
    if ( c.endsWith( QLatin1Char( ')' ) ) ) {
        c = completion.mid( 0, completion.lastIndexOf( QLatin1String( " (" )) ).trimmed();
    }
    q->setText( m_previousAddresses + c );
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
        QString ou;

        if ( m_showOU ) {
            const int depth = result.dn.depth();
            for ( int i = 0; i < depth; ++i ) {
                const QString rdnStr = result.dn.rdnString( i );
                if ( rdnStr.startsWith( QLatin1String( "ou=" ), Qt::CaseInsensitive ) ) {
                    ou = rdnStr.mid( 3 );
                    break;
                }
            }
        }

        if ( !s_static->ldapClientToCompletionSourceMap.contains( result.clientNumber ) ) {
            s_static->updateLDAPWeights(); // we got results from a new source, so update the completion sources
        }

        q->addContact( contact, result.completionWeight,
                       s_static->ldapClientToCompletionSourceMap[ result.clientNumber ], ou );
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
    if(m_useCompletion){
        s_static->slotEditCompletionOrder();
    }
}

void AddresseeLineEdit::Private::slotUserCancelled( const QString &cancelText )
{
    if ( s_static->ldapSearch && s_static->ldapLineEdit == q ) {
        stopLDAPLookup();
    }

    q->userCancelled( m_previousAddresses + cancelText ); // in KLineEdit
}

void AddresseeLineEdit::Private::slotAkonadiHandleItems( const Akonadi::Item::List &items )
{
    /* We have to fetch the collections of the items, so that
       the source name can be correctly labeled.*/
    foreach ( const Akonadi::Item &item, items ) {

        // check the local cache of collections
        const int sourceIndex =
                s_static->akonadiCollectionToCompletionSourceMap.value( item.parentCollection().id(), -1 );
        if ( sourceIndex == -1 ) {
            kDebug() << "Fetching New collection: " << item.parentCollection().id();
            // the collection isn't there, start the fetch job.
            Akonadi::CollectionFetchJob *collectionJob =
                    new Akonadi::CollectionFetchJob( item.parentCollection(),
                                                     Akonadi::CollectionFetchJob::Base,
                                                     s_static->akonadiSession );
            connect( collectionJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)),
                     q, SLOT(slotAkonadiCollectionsReceived(Akonadi::Collection::List)) );
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

    if ( !items.isEmpty() ) {
        const QListWidgetItem *current = q->completionBox()->currentItem();
        if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
            doCompletion( m_lastSearchMode );
        }
    }
}

void AddresseeLineEdit::Private::slotAkonadiSearchResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << "Akonadi search job failed: " << job->errorString();
    } else {
        Akonadi::ItemSearchJob *searchJob = static_cast<Akonadi::ItemSearchJob*>(job);
        kDebug() << "Found" << searchJob->items().size() << "items";
    }
    const int index = s_static->akonadiJobsInFlight.indexOf( qobject_cast<Akonadi::Job*>( job ) );
    if( index != -1 )
        s_static->akonadiJobsInFlight.remove( index );
}

void AddresseeLineEdit::Private::slotAkonadiCollectionsReceived(
        const Akonadi::Collection::List &collections )
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig( QLatin1String("kpimcompletionorder") );
    KConfigGroup group( config, "CompletionWeights" );
    foreach ( const Akonadi::Collection &collection, collections ) {
        if ( collection.isValid() ) {
            const QString sourceString = collection.displayName();
            const int weight = group.readEntry( QString::number(collection.id()), 1 );
            const int index = q->addCompletionSource( sourceString, weight );
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

void AddresseeLineEdit::Private::slotShowOUChanged(bool checked)
{
    if ( checked != m_showOU ) {
        KConfigGroup group( KGlobal::config(), "AddressLineEdit" );
        group.writeEntry( "ShowOU", checked );
        m_showOU = checked;
    }
}

void AddresseeLineEdit::Private::slotConfigureBalooBlackList()
{
    QPointer<KPIM::BlackListBalooEmailCompletionDialog> dlg = new KPIM::BlackListBalooEmailCompletionDialog(q);
    dlg->setEmailBlackList(m_balooBlackList);
    if (dlg->exec()) {
        loadBalooBlackList();
    }
    delete dlg;
}

void AddresseeLineEdit::Private::loadBalooBlackList()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig( QLatin1String("kpimbalooblacklist") );
    KConfigGroup group( config, "AddressLineEdit" );
    m_balooBlackList = group.readEntry( "BalooBackList", QStringList() );
    m_domainExcludeList = group.readEntry("ExcludeDomain", QStringList());
}
}
