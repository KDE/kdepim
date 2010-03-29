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
#include "completionordereditor.h"
#include "distributionlist.h"

#include <Akonadi/Contact/ContactSearchJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/ItemFetchScope>

#include <kldap/ldapclient.h>
#include <kldap/ldapserver.h>
#include <kmime/kmime_util.h>

#include <KCompletionBox>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>
#include <KStandardShortcut>
#include <KUrl>

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

class AddresseeLineEditStatic {
  public:
    AddresseeLineEditStatic() :
        completion( new KMailCompletion ),
        ldapTimer( 0 ),
        ldapSearch( 0 ),
        ldapLineEdit( 0 )
    {

    }

    ~AddresseeLineEditStatic() {
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

AddresseeLineEdit::AddresseeLineEdit( QWidget *parent, bool useCompletion )
  : KLineEdit( parent )
{
  setObjectName( newLineEditObjectName() );
  setClickMessage( "" );
  m_useCompletion = useCompletion;
  m_completionInitialized = false;
  m_smartPaste = false;
  m_addressBookConnected = false;
  m_searchExtended = false;

  init();
}

void AddresseeLineEdit::updateLDAPWeights()
{
  /* Add completion sources for all ldap server, 0 to n. Added first so
   * that they map to the ldapclient::clientNumber() */
  s_static->ldapSearch->updateCompletionWeights();
  int clientIndex = 0;
  foreach ( const KLDAP::LdapClient *client, s_static->ldapSearch->clients() ) {
    const int sourceIndex = addCompletionSource(
      "LDAP server: " + client->server().host(), client->completionWeight() );
    s_static->ldapClientToCompletionSourceMap.insert( clientIndex, sourceIndex );
  }
}

void AddresseeLineEdit::init()
{
  if ( !s_static.exists() ) {
    s_static->completion->setOrder( completionOrder() );
    s_static->completion->setIgnoreCase( true );
  }
//  connect( s_static->completion, SIGNAL(match(const QString&)),
//           this, SLOT(slotMatched(const QString&)) );

  if ( m_useCompletion ) {
    if ( !s_static->ldapTimer ) {
      s_static->ldapTimer = new QTimer;
      s_static->ldapSearch = new KLDAP::LdapClientSearch;
    }

    updateLDAPWeights();

    if ( !m_completionInitialized ) {
      setCompletionObject( s_static->completion, false );
      connect( this, SIGNAL(completion(const QString&)),
               this, SLOT(slotCompletion()) );
      connect( this, SIGNAL(returnPressed(const QString&)),
               this, SLOT(slotReturnPressed(const QString&)) );

      KCompletionBox *box = completionBox();
      connect( box, SIGNAL(activated(const QString&)),
               this, SLOT(slotPopupCompletion(const QString&)) );
      connect( box, SIGNAL(userCancelled(const QString&)),
               SLOT(slotUserCancelled(const QString&)) );

      connect( s_static->ldapTimer, SIGNAL(timeout()), SLOT(slotStartLDAPLookup()) );
      connect( s_static->ldapSearch, SIGNAL(searchData(const KLDAP::LdapResult::List&)),
               SLOT(slotLDAPSearchData(const KLDAP::LdapResult::List&)) );

      m_completionInitialized = true;
    }
  }
}

AddresseeLineEdit::~AddresseeLineEdit()
{
  if ( s_static->ldapSearch && s_static->ldapLineEdit == this ) {
    stopLDAPLookup();
  }
}

void AddresseeLineEdit::setFont( const QFont &font )
{
  KLineEdit::setFont( font );
  if ( m_useCompletion ) {
    completionBox()->setFont( font );
  }
}

void AddresseeLineEdit::allowSemiColonAsSeparator( bool useSemiColonAsSeparator )
{
  m_useSemiColonAsSeparator = useSemiColonAsSeparator;
}

void AddresseeLineEdit::keyPressEvent( QKeyEvent *e )
{
  bool accept = false;

  const int key = e->key() | e->modifiers();

  if ( KStandardShortcut::shortcut( KStandardShortcut::SubstringCompletion ).contains( key ) ) {
    //TODO: add LDAP substring lookup, when it becomes available in KPIM::LDAPSearch
    updateSearchString();
    akonadiPerformSearch();
    doCompletion( true );
    accept = true;
  } else if ( KStandardShortcut::shortcut( KStandardShortcut::TextCompletion ).contains( key ) ) {
    int len = text().length();

    if ( len == cursorPosition() ) { // at End?
      updateSearchString();
      akonadiPerformSearch();
      doCompletion( true );
      accept = true;
    }
  }

  const QString oldContent = text();
  if ( !accept ) {
    KLineEdit::keyPressEvent( e );
  }

  // if the text didn't change (eg. because a cursor navigation key was pressed)
  // we don't need to trigger a new search
  if ( oldContent == text() )
    return;

  if ( e->isAccepted() ) {
    updateSearchString();
    QString searchString( m_searchString );
    //LDAP does not know about our string manipulation, remove it
    if ( m_searchExtended ) {
      searchString = m_searchString.mid( 1 );
    }

    if ( m_useCompletion && s_static->ldapTimer ) {
      if ( s_static->ldapText != searchString || s_static->ldapLineEdit != this ) {
        stopLDAPLookup();
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
  if ( !m_smartPaste ) {
    KLineEdit::insert( t );
    return;
  }

  QString newText = t.trimmed();
  if ( newText.isEmpty() ) {
    return;
  }

  // remove newlines in the to-be-pasted string
  QStringList lines = newText.split( QRegExp( "\r?\n" ), QString::SkipEmptyParts );
  for ( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
    // remove trailing commas and whitespace
    (*it).remove( QRegExp( ",?\\s*$" ) );
  }
  newText = lines.join( ", " );

  if ( newText.startsWith( QLatin1String( "mailto:" ) ) ) {
    KUrl url( newText );
    newText = url.path();
  } else if ( newText.indexOf( " at " ) != -1 ) {
    // Anti-spam stuff
    newText.replace( " at ", "@" );
    newText.replace( " dot ", "." );
  } else if ( newText.indexOf( "(at)" ) != -1 ) {
    newText.replace( QRegExp( "\\s*\\(at\\)\\s*" ), "@" );
  }

  QString contents = text();
  int start_sel = 0;
  int pos = cursorPosition( );

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
    if ( contents[ eot - 1 ] == ',' ) {
      eot--;
    }
    contents.truncate( eot );
    contents += ", ";
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
  if ( m_useCompletion ) {
    m_smartPaste = true;
  }

  KLineEdit::paste();
  m_smartPaste = false;
}

void AddresseeLineEdit::mouseReleaseEvent( QMouseEvent *e )
{
  // reimplemented from QLineEdit::mouseReleaseEvent()
  if ( m_useCompletion &&
       QApplication::clipboard()->supportsSelection() &&
       !isReadOnly() &&
       e->button() == Qt::MidButton ) {
    m_smartPaste = true;
  }

  KLineEdit::mouseReleaseEvent( e );
  m_smartPaste = false;
}

void AddresseeLineEdit::dropEvent( QDropEvent *e )
{
  if ( !isReadOnly() ) {
    KUrl::List uriList = KUrl::List::fromMimeData( e->mimeData() );
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
      for ( KUrl::List::Iterator it = uriList.begin(); it != uriList.end(); ++it ) {
        KUrl u( *it );
        if ( u.protocol() == "mailto" ) {
          mailtoURL = true;
          QString address;
          address = KUrl::fromPercentEncoding( u.path().toLatin1() );
          address = KMime::decodeRFC2047String( address.toAscii() );
          if ( !contents.isEmpty() ) {
            contents.append( ", " );
          }
          contents.append( address );
        }
      }
      if ( mailtoURL ) {
        setText( contents );
        setModified( true );
        return;
      }
    }
  }

  if ( m_useCompletion ) {
    m_smartPaste = true;
  }
  QLineEdit::dropEvent( e );
  m_smartPaste = false;
}

void AddresseeLineEdit::cursorAtEnd()
{
  setCursorPosition( text().length() );
}

void AddresseeLineEdit::enableCompletion( bool enable )
{
  m_useCompletion = enable;
}

void AddresseeLineEdit::doCompletion( bool ctrlT )
{
  m_lastSearchMode = ctrlT;

  KGlobalSettings::Completion  mode = completionMode();

  if ( mode == KGlobalSettings::CompletionNone ) {
    return;
  }

  s_static->completion->setOrder( completionOrder() );

  // cursor at end of string - or Ctrl+T pressed for substring completion?
  if ( ctrlT ) {
    const QStringList completions = getAdjustedCompletionItems( false );

    if ( completions.count() > 1 ) {
      ; //m_previousAddresses = prevAddr;
    } else if ( completions.count() == 1 ) {
      setText( m_previousAddresses + completions.first().trimmed() );
    }

    // Make sure the completion popup is closed if no matching items were found
    setCompletedItems( completions, true );

    cursorAtEnd();
    setCompletionMode( mode ); //set back to previous mode
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
    const QStringList items = getAdjustedCompletionItems( true );
    setCompletedItems( items, false );
    break;
  }

  case KGlobalSettings::CompletionShell:
  {
    QString match = s_static->completion->makeCompletion( m_searchString );
    if ( !match.isNull() && match != m_searchString ) {
      setText( m_previousAddresses + match );
      setModified( true );
      cursorAtEnd();
    }
    break;
  }

  case KGlobalSettings::CompletionMan: // Short-Auto in fact
  case KGlobalSettings::CompletionAuto:
  {
    //force autoSuggest in KLineEdit::keyPressed or setCompletedText will have no effect
    setCompletionMode( completionMode() );

    if ( !m_searchString.isEmpty() ) {

      //if only our \" is left, remove it since user has not typed it either
      if ( m_searchExtended && m_searchString == "\"" ){
        m_searchExtended = false;
        m_searchString.clear();
        setText( m_previousAddresses );
        break;
      }

      QString match = s_static->completion->makeCompletion( m_searchString );

      if ( !match.isEmpty() ) {
        if ( match != m_searchString ) {
          QString adds = m_previousAddresses + match;
          setCompletedText( adds );
        }
      } else {
        if ( !m_searchString.startsWith( '\"' ) ) {
          //try with quoted text, if user has not type one already
          match = s_static->completion->makeCompletion( "\"" + m_searchString );
          if ( !match.isEmpty() && match != m_searchString ) {
            m_searchString = "\"" + m_searchString;
            m_searchExtended = true;
            setText( m_previousAddresses + m_searchString );
            setCompletedText( m_previousAddresses + match );
          }
        } else if ( m_searchExtended ) {
          //our added \" does not work anymore, remove it
          m_searchString = m_searchString.mid( 1 );
          m_searchExtended = false;
          setText( m_previousAddresses + m_searchString );
          //now try again
          match = s_static->completion->makeCompletion( m_searchString );
          if ( !match.isEmpty() && match != m_searchString ) {
            QString adds = m_previousAddresses + match;
            setCompletedText( adds );
          }
        }
      }
    }
    break;
  }

  case KGlobalSettings::CompletionNone:
  default: // fall through
    break;
  }
}

void AddresseeLineEdit::slotPopupCompletion( const QString &completion )
{
  setText( m_previousAddresses + completion.trimmed() );
  cursorAtEnd();
//  slotMatched( m_previousAddresses + completion );
  updateSearchString();
}

void AddresseeLineEdit::slotReturnPressed( const QString &item )
{
  Q_UNUSED( item );
  if ( !completionBox()->selectedItems().isEmpty() ) {
    slotPopupCompletion( completionBox()->selectedItems().first()->text() );
  }
}

void AddresseeLineEdit::addContact( const KABC::Addressee &addr, int weight, int source )
{
  if ( KPIM::DistributionList::isDistributionList( addr ) ) {

    //for CompletionAuto
    addCompletionItem( addr.formattedName(), weight, source );

    //for CompletionShell, CompletionPopup
    QStringList sl( addr.formattedName() );
    addCompletionItem( addr.formattedName(), weight, source, &sl );

    return;
  }
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
      addCompletionItem( fullEmail, weight + isPrefEmail, source ); // use whatever is there
    } else {
      const QString byFirstName=  "\"" + givenName + " " + familyName + "\" <" + email + ">";
      const QString byLastName =  "\"" + familyName + ", " + givenName + "\" <" + email + ">";
      addCompletionItem( byFirstName, weight + isPrefEmail, source );
      addCompletionItem( byLastName, weight + isPrefEmail, source );
    }

    addCompletionItem( email, weight + isPrefEmail, source );

    if ( !nickName.isEmpty() ){
      const QString byNick     =  "\"" + nickName + "\" <" + email + ">";
      addCompletionItem( byNick, weight + isPrefEmail, source );
    }

    if ( !domain.isEmpty() ){
      const QString byDomain = '\"' + domain + ' ' +
                               familyName + ' ' + givenName +
                               "\" <" + email + '>';
      addCompletionItem( byDomain, weight + isPrefEmail, source );
    }

    //for CompletionShell, CompletionPopup
    QStringList keyWords;
    const QString realName  = addr.realName();

    if ( !givenName.isEmpty() && !familyName.isEmpty() ) {
      keyWords.append( givenName  + ' '  + familyName );
      keyWords.append( familyName + ' '  + givenName );
      keyWords.append( familyName + ", " + givenName );
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
     * We remove the <blank> in getAdjustedCompletionItems.
     */
    if ( isPrefEmail == prefEmailWeight ) {
      fullEmail.replace( " <", "  <" );
    }

    addCompletionItem( fullEmail, weight + isPrefEmail, source, &keyWords );
    isPrefEmail = 0;

#if 0
    int len = (*it).length();
    if ( len == 0 ) {
      continue;
    }
    if( '\0' == (*it)[len-1] ) {
      --len;
    }
    const QString tmp = (*it).left( len );
    const QString fullEmail = addr.fullEmail( tmp );
    //kDebug(5300) <<"AddresseeLineEdit::addContact() \"" << fullEmail <<"\" weight=" << weight;
    addCompletionItem( fullEmail.simplified(), weight, source );
    // Try to guess the last name: if found, we add an extra
    // entry to the list to make sure completion works even
    // if the user starts by typing in the last name.
    QString name( addr.realName().simplified() );
    if ( name.endsWith( "\"" ) ) {
      name.truncate( name.length()-1 );
    }
    if ( name.startsWith( "\"" ) ) {
      name = name.mid( 1 );
    }

    // While we're here also add "email (full name)" for completion on the email
    if ( !name.isEmpty() ) {
      addCompletionItem( addr.preferredEmail() + " (" + name + ')', weight, source );
    }

    bool bDone = false;
    int i = -1;
    while ( ( i = name.lastIndexOf( ' ' ) ) > 1 && !bDone ) {
      QString sLastName( name.mid( i + 1 ) );
      // last names must be at least 2 chars long and must not end
      // with a '.', like in "Jr." or "Sr."
      if( !sLastName.isEmpty() &&
          2 <= sLastName.length() &&
          !sLastName.endsWith( "." ) ) {
        name.truncate( i );
        if ( !name.isEmpty() ){
          sLastName.prepend( "\"" );
          sLastName.append( ", " + name + "\" <" );
        }
        QString sExtraEntry( sLastName );
        sExtraEntry.append( tmp.isEmpty() ? addr.preferredEmail() : tmp );
        sExtraEntry.append( ">" );
        addCompletionItem( sExtraEntry.simplified(), weight, source );
        bDone = true;
      }
      if ( !bDone ) {
        name.truncate( i );
        if ( name.endsWith( "\"" ) ) {
          name.truncate( name.length() - 1 );
        }
      }
    }
#endif
  }
}

void AddresseeLineEdit::addCompletionItem( const QString &string, int weight,
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

void AddresseeLineEdit::slotStartLDAPLookup()
{
  KGlobalSettings::Completion  mode = completionMode();

  if ( mode == KGlobalSettings::CompletionNone ) {
    return;
  }

  if ( !s_static->ldapSearch->isAvailable() ) {
    return;
  }
  if ( s_static->ldapLineEdit != this ) {
    return;
  }

  startLoadingLDAPEntries();
}

void AddresseeLineEdit::stopLDAPLookup()
{
  s_static->ldapSearch->cancelSearch();
  s_static->ldapLineEdit = 0;
}

void AddresseeLineEdit::startLoadingLDAPEntries()
{
  QString s( s_static->ldapText );
  // TODO cache last?
  QString prevAddr;
  int n = s.lastIndexOf( ',' );
  if ( n >= 0 ) {
    prevAddr = s.left( n + 1 ) + ' ';
    s = s.mid( n + 1, 255 ).trimmed();
  }

  if ( s.isEmpty() ) {
    return;
  }
  s_static->ldapSearch->startSearch( s );
}

void AddresseeLineEdit::slotLDAPSearchData( const KLDAP::LdapResult::List &adrs )
{
  if ( adrs.isEmpty() || s_static->ldapLineEdit != this ) {
    return;
  }

  for ( KLDAP::LdapResult::List::ConstIterator it = adrs.begin(); it != adrs.end(); ++it ) {
    KABC::Addressee addr;
    addr.setNameFromString( (*it).name );
    addr.setEmails( (*it).email );

    if ( !s_static->ldapClientToCompletionSourceMap.contains( (*it).clientNumber ) )
      updateLDAPWeights(); // we got results from a new source, so update the completion sources

    addContact( addr, (*it).completionWeight, s_static->ldapClientToCompletionSourceMap[ (*it ).clientNumber ]  );
  }
  if ( ( hasFocus() || completionBox()->hasFocus() ) &&
       completionMode() != KGlobalSettings::CompletionNone &&
       completionMode() != KGlobalSettings::CompletionShell ) {
    setText( m_previousAddresses + m_searchString );
    // only complete again if the user didn't change the selection while
    // we were waiting; otherwise the completion box will be closed
    QListWidgetItem *current = completionBox()->currentItem();
    if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
      doCompletion( m_lastSearchMode );
    }
  }
}

void AddresseeLineEdit::setCompletedItems( const QStringList &items, bool autoSuggest )
{
  KCompletionBox *completionBox = this->completionBox();

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
        qApp->installEventFilter( this );
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
      int index = items.first().indexOf( m_searchString );
      QString newText = items.first().mid( index );
      setUserSelection( false );
      setCompletedText( newText, true );
    }
  } else {
    if ( completionBox && completionBox->isVisible() ) {
      completionBox->hide();
      completionBox->setItems( QStringList() );
    }
  }
}

void AddresseeLineEdit::contextMenuEvent( QContextMenuEvent *e )
{
  QMenu *menu = createStandardContextMenu();
  menu->exec( e->globalPos() );
  delete menu;
}

QMenu *AddresseeLineEdit::createStandardContextMenu()
{
  //disable modes not supported by KMailCompletion
  setCompletionModeDisabled( KGlobalSettings::CompletionMan );
  setCompletionModeDisabled( KGlobalSettings::CompletionPopupAuto );
  QMenu *menu = KLineEdit::createStandardContextMenu();
  if ( !menu ) {
    return 0;
  }

  if ( m_useCompletion ) {
    menu->addAction( i18n( "Configure Completion Order..." ),
                     this, SLOT(slotEditCompletionOrder()) );
  }
  return menu;
}

void AddresseeLineEdit::slotEditCompletionOrder()
{
  init(); // for s_static->ldapSearch
  CompletionOrderEditor editor( s_static->ldapSearch, this );
  editor.exec();
  if ( m_useCompletion ) {
    updateLDAPWeights();
  }
}

void KPIM::AddresseeLineEdit::slotUserCancelled( const QString &cancelText )
{
  if ( s_static->ldapSearch && s_static->ldapLineEdit == this ) {
    stopLDAPLookup();
  }
  userCancelled( m_previousAddresses + cancelText ); // in KLineEdit
}

void AddresseeLineEdit::slotAkonadiSearchResult( KJob* job )
{
  Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
  kDebug() << "akonadi found " << searchJob->contacts().size() << "contacts";
  /* We have to fetch the collections of the items, so that
     the source name can be correctly labeled.*/
  Akonadi::Item::List items = searchJob->items();
  foreach( Akonadi::Item item, items ) {
    // check the local cache of collections
    int sourceIndex;
    if( ( sourceIndex = s_static->akonadiCollectionToCompletionSourceMap.value( item.parentCollection().id(), -1 ) ) == -1 ) {
      kDebug() << "Fetching New collection: " << item.parentCollection().id();
      // the collection isn't there, start the fetch job.
      Akonadi::CollectionFetchJob *collectionJob = new Akonadi::CollectionFetchJob( item.parentCollection(),
                                                                                    Akonadi::CollectionFetchJob::Base, this );
      connect( collectionJob, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ),
               this, SLOT( slotAkonadiCollectionsReceived(Akonadi::Collection::List ) ) );
      /* we don't want to start multiple fetch jobs for the same collection,
         so insert the collection with an index value of -2 */
      s_static->akonadiCollectionToCompletionSourceMap.insert( item.parentCollection().id(), -2 );
      s_static->akonadiPendingItems.append( item );
    } else if( sourceIndex == -2 ) {
      /* fetch job already started, don't need to start another one,
         so just append the item as pending */
      s_static->akonadiPendingItems.append( item );
    } else if ( item.hasPayload<KABC::Addressee>() ) {
      addContact( item.payload<KABC::Addressee>(), 1, sourceIndex ); // TODO calculate proper weight somehow..
    }
    
  }
  if( searchJob->contacts().size() > 0 ) {
    QListWidgetItem *current = completionBox()->currentItem();
    if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
      doCompletion( m_lastSearchMode );
    }
  }
}

void AddresseeLineEdit::slotAkonadiCollectionsReceived( const Akonadi::Collection::List &collections )
{
  foreach( Akonadi::Collection collection, collections ) {
    if( collection.isValid() ) {
      QString sourceString = collection.name();
      int index = addCompletionSource( sourceString, 1 );
      kDebug() << "\treceived: " << sourceString << "index: " << index;
      s_static->akonadiCollectionToCompletionSourceMap.insert( collection.id(), index );
    }
  }
  // now that we have added the new collections, recheck our list of pending contacts
  akonadiHandlePending();
  // do completion
  QListWidgetItem *current = completionBox()->currentItem();
  if ( !current || m_searchString.trimmed() != current->text().trimmed() ) {
    doCompletion( m_lastSearchMode );
  }
}

void AddresseeLineEdit::updateSearchString()
{
  m_searchString = text();

  int n = -1;
  bool inQuote = false;
  for ( uint i = 0, searchStringLength = m_searchString.length(); i < searchStringLength; ++i ) {
    if ( m_searchString[ i ] == '"' ) {
      inQuote = !inQuote;
    }
    if ( m_searchString[ i ] == '\\' &&
         ( i + 1 ) < searchStringLength && m_searchString[ i + 1 ] == '"' ) {
      ++i;
    }
    if ( inQuote ) {
      continue;
    }
    if ( m_searchString[ i ] == ',' || ( m_useSemiColonAsSeparator && m_searchString[ i ] == ';' ) ) {
      n = i;
    }
  }

  if ( n >= 0 ) {
    ++n; // Go past the ","

    int len = m_searchString.length();

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

void AddresseeLineEdit::akonadiPerformSearch()
{
  kDebug() << "searching akonadi with:" << m_searchString;
  Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
  job->fetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  job->setQuery( Akonadi::ContactSearchJob::NameOrEmail, m_searchString, Akonadi::ContactSearchJob::ContainsMatch );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotAkonadiSearchResult( KJob* ) ) );
  akonadiHandlePending();
}

void AddresseeLineEdit::akonadiHandlePending()
{
  kDebug() << "Pending items: " << s_static->akonadiPendingItems.size();
  Akonadi::Item::List::iterator it = s_static->akonadiPendingItems.begin();
  while( it != s_static->akonadiPendingItems.end() ) {
    Akonadi::Item item = *it;
    int sourceIndex;
    if( ( sourceIndex = s_static->akonadiCollectionToCompletionSourceMap.value( item.parentCollection().id(), -1 ) ) >= 0 ) {
      kDebug() << "identified collection: " << s_static->completionSources[sourceIndex];
      if ( item.hasPayload<KABC::Addressee>() )
        addContact( item.payload<KABC::Addressee>(), 1, sourceIndex ); // TODO calculate proper weight somehow..
      // remove from the pending
      it = s_static->akonadiPendingItems.erase( it );
    } else {
      ++it;
    }
  }
}


void KPIM::AddresseeLineEdit::slotCompletion()
{
  // Called by KLineEdit's keyPressEvent for CompletionModes
  // Auto,Popup -> new text, update search string.
  // not called for CompletionShell, this is been taken care of
  // in AddresseeLineEdit::keyPressEvent

  updateSearchString();
  if ( completionBox() ) {
    completionBox()->setCancelledText( m_searchString );
  }
  akonadiPerformSearch();
  doCompletion( false );
}

// not cached, to make sure we get an up-to-date value when it changes
KCompletion::CompOrder KPIM::AddresseeLineEdit::completionOrder()
{
  KConfig _config( "kpimcompletionorder" );
  KConfigGroup config( &_config, "General" );
  const QString order = config.readEntry( "CompletionOrder", "Weighted" );

  if ( order == "Weighted" ) {
    return KCompletion::Weighted;
  } else {
    return KCompletion::Sorted;
  }
}

int KPIM::AddresseeLineEdit::addCompletionSource( const QString &source, int weight )
{
  QMap<QString,int>::iterator it = s_static->completionSourceWeights.find( source );
  if ( it == s_static->completionSourceWeights.end() )
    s_static->completionSourceWeights.insert( source, weight );
  else
    s_static->completionSourceWeights[source] = weight;

  int sourceIndex = s_static->completionSources.indexOf( source );
  if ( sourceIndex == -1 ) {
    s_static->completionSources.append( source );
    return s_static->completionSources.size() - 1;
  }
  else
    return sourceIndex;
}

bool KPIM::AddresseeLineEdit::eventFilter( QObject *obj, QEvent *e )
{
  if ( m_completionInitialized &&
       ( obj == completionBox() ||
         completionBox()->findChild<QWidget*>( obj->objectName() ) == obj ) ) {
    if ( e->type() == QEvent::MouseButtonPress ||
         e->type() == QEvent::MouseMove ||
         e->type() == QEvent::MouseButtonRelease ||
         e->type() == QEvent::MouseButtonDblClick ) {
      QMouseEvent* me = static_cast<QMouseEvent*>( e );
      // find list box item at the event position
      QListWidgetItem *item = completionBox()->itemAt( me->pos() );
      if ( !item ) {
        // In the case of a mouse move outside of the box we don't want
        // the parent to fuzzy select a header by mistake.
        bool eat = e->type() == QEvent::MouseMove;
        return eat;
      }
      // avoid selection of headers on button press, or move or release while
      // a button is pressed
      Qt::MouseButtons btns = me->buttons();
      if ( e->type() == QEvent::MouseButtonPress ||
           e->type() == QEvent::MouseButtonDblClick ||
           btns & Qt::LeftButton || btns & Qt::MidButton ||
           btns & Qt::RightButton ) {
        if ( itemIsHeader( item ) ) {
          return true; // eat the event, we don't want anything to happen
        } else {
          // if we are not on one of the group heading, make sure the item
          // below or above is selected, not the heading, inadvertedly, due
          // to fuzzy auto-selection from QListBox
          completionBox()->setCurrentItem( item );
          item->setSelected( true );
          if ( e->type() == QEvent::MouseMove ) {
            return true; // avoid fuzzy selection behavior
          }
        }
      }
    }
  }
  if ( ( obj == this ) &&
     ( e->type() == QEvent::ShortcutOverride ) ) {
    QKeyEvent *ke = static_cast<QKeyEvent*>( e );
    if ( ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Tab ) {
      ke->accept();
      return true;
    }
  }
  if ( ( obj == this ) &&
      ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) &&
      completionBox()->isVisible() ) {
    QKeyEvent *ke = static_cast<QKeyEvent*>( e );
    int currentIndex = completionBox()->currentRow();
    if ( ke->key() == Qt::Key_Up ) {
      //kDebug() <<"EVENTFILTER: Qt::Key_Up currentIndex=" << currentIndex;
      // figure out if the item we would be moving to is one we want
      // to ignore. If so, go one further
      QListWidgetItem *itemAbove = completionBox()->item( currentIndex );
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
          QListWidgetItem *i = completionBox()->item( currentIndex );
          if ( i ) {
            if ( itemIsHeader( i ) ) {
              currentIndex++;
              i = completionBox()->item( currentIndex );
            }
            completionBox()->setCurrentItem( i );
            i->setSelected( true );
          }
        }
        return true;
      }
    } else if ( ke->key() == Qt::Key_Down ) {
      // same strategy for downwards
      //kDebug() <<"EVENTFILTER: Qt::Key_Down. currentIndex=" << currentIndex;
      QListWidgetItem *itemBelow = completionBox()->item( currentIndex );
      if ( itemBelow && itemIsHeader( itemBelow ) ) {
        if ( completionBox()->item( currentIndex + 1 ) ) {
          //kDebug() <<"EVENTFILTER: Qt::Key_Down -> skipping" << currentIndex+1;
          completionBox()->setCurrentRow( currentIndex + 1 );
          completionBox()->item( currentIndex + 1 )->setSelected( true );
        } else {
          // nothing to skip to, let's stay where we are
          QListWidgetItem *i = completionBox()->item( currentIndex );
          if ( i ) {
            completionBox()->setCurrentItem( i );
            i->setSelected( true );
          }
        }
        return true;
      }
      // special case of the initial selection, which is unfortunately a header.
      // Setting it to selected tricks KCompletionBox into not treating is special
      // and selecting making it current, instead of the one below.
      QListWidgetItem *item = completionBox()->item( currentIndex );
      if ( item && itemIsHeader(item) ) {
        completionBox()->setCurrentItem( item );
        item->setSelected( true );
      }
    } else if ( ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab ) {
      /// first, find the header of the current section
      QListWidgetItem *myHeader = 0;
      int i = currentIndex;
      while ( i>=0 ) {
        if ( itemIsHeader( completionBox()->item(i) ) ) {
          myHeader = completionBox()->item( i );
          break;
        }
        i--;
      }
      Q_ASSERT( myHeader ); // we should always be able to find a header

      // find the next header (searching backwards, for Qt::Key_Backtab
      QListWidgetItem *nextHeader = 0;
      const int iterationstep = ke->key() == Qt::Key_Tab ?  1 : -1;
      // when iterating forward, start at the currentindex, when backwards,
      // one up from our header, or at the end
      uint j = ke->key() == Qt::Key_Tab ?
               currentIndex : i == 0 ?
               completionBox()->count() - 1 : ( i - 1 ) % completionBox()->count();
      while ( ( nextHeader = completionBox()->item( j ) ) && nextHeader != myHeader ) {
          if ( itemIsHeader(nextHeader) ) {
              break;
          }
          j = ( j + iterationstep ) % completionBox()->count();
      }
      if ( nextHeader && nextHeader != myHeader ) {
        QListWidgetItem *item = completionBox()->item( j + 1 );
        if ( item && !itemIsHeader(item) ) {
          completionBox()->setCurrentItem( item );
          item->setSelected( true );
        }
      }
      return true;
    }
  }
  return KLineEdit::eventFilter( obj, e );
}

class SourceWithWeight {
  public:
    int weight;           // the weight of the source
    QString sourceName;   // the name of the source, e.g. "LDAP Server"
    int index;            // index into s_static->completionSources

    bool operator< ( const SourceWithWeight &other ) const {
      if ( weight > other.weight )
        return true;
      if ( weight < other.weight )
        return false;
      return sourceName < other.sourceName;
    }
};

const QStringList KPIM::AddresseeLineEdit::getAdjustedCompletionItems( bool fullSearch )
{
  QStringList items = fullSearch ?
    s_static->completion->allMatches( m_searchString )
    : s_static->completion->substringCompletion( m_searchString );

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
    CompletionItemsMap::const_iterator cit = s_static->completionItemMap.constFind(*it);
    if ( cit == s_static->completionItemMap.constEnd() ) {
      continue;
    }
    int idx = (*cit).second;

    if ( s_static->completion->order() == KCompletion::Weighted ) {
      if ( lastSourceIndex == -1 || lastSourceIndex != idx ) {
        const QString sourceLabel( s_static->completionSources[idx] );
        if ( sections.find(idx) == sections.end() ) {
          it = items.insert( it, sourceLabel );
          ++it; //skip new item
        }
        lastSourceIndex = idx;
      }
      (*it) = (*it).prepend( s_completionItemIndentString );
      // remove preferred email sort <blank> added in  addContact()
      (*it).replace( "  <", " <" );
    }
    sections[idx].append( *it );

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
    for( int i = 0; i < sourcesAndWeights.size(); i++ ) {
      QStringList sectionItems = sections[sourcesAndWeights[i].index];
      if ( !sectionItems.isEmpty() ) {
        sortedItems.append( sourcesAndWeights[i].sourceName );
        QStringList sectionItems = sections[sourcesAndWeights[i].index];
        foreach( const QString &itemInSection, sectionItems ) {
          sortedItems.append( itemInSection );
        }
      }
    }
  } else {
    sortedItems.sort();
  }
  return sortedItems;
}
#include "addresseelineedit.moc"
