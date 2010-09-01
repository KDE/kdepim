/*
    This file is part of libkdepim.
    Copyright (c) 2002 Helge Deller <deller@gmx.de>
                  2002 Lubos Lunak <llunak@suse.cz>
                  2001,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Waldo Bastian <bastian@kde.org>
                  2004 Daniel Molkentin <danimo@klaralvdalens-datakonsult.se>
                  2004 Karl-Heinz Zimmer <khz@klaralvdalens-datakonsult.se>

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

#include "resourceabc.h"
#include "completionordereditor.h"
#include "ldapclient.h"

#include <config.h>

#ifdef KDEPIM_NEW_DISTRLISTS
#include "distributionlist.h"
#else
#include <kabc/distributionlist.h>
#endif

#include <kabc/stdaddressbook.h>
#include <kabc/resource.h>
#include <libemailfunctions/email.h>

#include <kcompletionbox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kstdaccel.h>
#include <kurldrag.h>
#include <klocale.h>

#include <tqpopupmenu.h>
#include <tqapplication.h>
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqregexp.h>
#include <tqevent.h>
#include <tqdragobject.h>
#include <tqclipboard.h>

using namespace KPIM;

KMailCompletion * AddresseeLineEdit::s_completion = 0L;
KPIM::CompletionItemsMap* AddresseeLineEdit::s_completionItemMap = 0L;
TQStringList* AddresseeLineEdit::s_completionSources = 0L;
bool AddresseeLineEdit::s_addressesDirty = false;
TQTimer* AddresseeLineEdit::s_LDAPTimer = 0L;
KPIM::LdapSearch* AddresseeLineEdit::s_LDAPSearch = 0L;
TQString* AddresseeLineEdit::s_LDAPText = 0L;
AddresseeLineEdit* AddresseeLineEdit::s_LDAPLineEdit = 0L;

// The weights associated with the completion sources in s_completionSources.
// Both are maintained by addCompletionSource(), don't attempt to modifiy those yourself.
TQMap<TQString,int>* s_completionSourceWeights = 0;

// maps LDAP client indices to completion source indices
// the assumption that they are always the first n indices in s_completion
// does not hold when clients are added later on
TQMap<int, int>* AddresseeLineEdit::s_ldapClientToCompletionSourceMap = 0;

static KStaticDeleter<KMailCompletion> completionDeleter;
static KStaticDeleter<KPIM::CompletionItemsMap> completionItemsDeleter;
static KStaticDeleter<TQTimer> ldapTimerDeleter;
static KStaticDeleter<KPIM::LdapSearch> ldapSearchDeleter;
static KStaticDeleter<TQString> ldapTextDeleter;
static KStaticDeleter<TQStringList> completionSourcesDeleter;
static KStaticDeleter<TQMap<TQString,int> > completionSourceWeightsDeleter;
static KStaticDeleter<TQMap<int, int> > ldapClientToCompletionSourceMapDeleter;

// needs to be unique, but the actual name doesn't matter much
static TQCString newLineEditDCOPObjectName()
{
    static int s_count = 0;
    TQCString name( "KPIM::AddresseeLineEdit" );
    if ( s_count++ ) {
      name += '-';
      name += TQCString().setNum( s_count );
    }
    return name;
}

static const TQString s_completionItemIndentString = "     ";

static bool itemIsHeader( const TQListBoxItem* item )
{
  return item && !item->text().startsWith( s_completionItemIndentString );
}



AddresseeLineEdit::AddresseeLineEdit( TQWidget* parent, bool useCompletion,
                                      const char *name )
  : ClickLineEdit( parent, TQString::null, name ), DCOPObject( newLineEditDCOPObjectName() ),
    m_useSemiColonAsSeparator( false ), m_allowDistLists( true )
{
  m_useCompletion = useCompletion;
  m_completionInitialized = false;
  m_smartPaste = false;
  m_addressBookConnected = false;
  m_searchExtended = false;

  init();

  if ( m_useCompletion )
    s_addressesDirty = true;
}

void AddresseeLineEdit::updateLDAPWeights()
{
  /* Add completion sources for all ldap server, 0 to n. Added first so
   * that they map to the ldapclient::clientNumber() */
  s_LDAPSearch->updateCompletionWeights();
  TQValueList< LdapClient* > clients =  s_LDAPSearch->clients();
  int clientIndex = 0;
  for ( TQValueList<LdapClient*>::iterator it = clients.begin(); it != clients.end(); ++it, ++clientIndex ) {
    const int sourceIndex = addCompletionSource( "LDAP server: " + (*it)->server().host(), (*it)->completionWeight() );
    s_ldapClientToCompletionSourceMap->insert( clientIndex, sourceIndex );
  }
}

void AddresseeLineEdit::init()
{
  if ( !s_completion ) {
    completionDeleter.setObject( s_completion, new KMailCompletion() );
    s_completion->setOrder( completionOrder() );
    s_completion->setIgnoreCase( true );

    completionItemsDeleter.setObject( s_completionItemMap, new KPIM::CompletionItemsMap() );
    completionSourcesDeleter.setObject( s_completionSources, new TQStringList() );
    completionSourceWeightsDeleter.setObject( s_completionSourceWeights, new TQMap<TQString,int> );
    ldapClientToCompletionSourceMapDeleter.setObject( s_ldapClientToCompletionSourceMap, new TQMap<int,int> );
  }
//  connect( s_completion, TQT_SIGNAL( match( const TQString& ) ),
//           this, TQT_SLOT( slotMatched( const TQString& ) ) );

  if ( m_useCompletion ) {
    if ( !s_LDAPTimer ) {
      ldapTimerDeleter.setObject( s_LDAPTimer, new TQTimer( 0, "ldapTimerDeleter" ) );
      ldapSearchDeleter.setObject( s_LDAPSearch, new KPIM::LdapSearch );
      ldapTextDeleter.setObject( s_LDAPText, new TQString );
    }

    updateLDAPWeights();

    if ( !m_completionInitialized ) {
      setCompletionObject( s_completion, false );
      connect( this, TQT_SIGNAL( completion( const TQString& ) ),
          this, TQT_SLOT( slotCompletion() ) );
      connect( this, TQT_SIGNAL( returnPressed( const TQString& ) ),
          this, TQT_SLOT( slotReturnPressed( const TQString& ) ) );

      KCompletionBox *box = completionBox();
      connect( box, TQT_SIGNAL( highlighted( const TQString& ) ),
          this, TQT_SLOT( slotPopupCompletion( const TQString& ) ) );
      connect( box, TQT_SIGNAL( userCancelled( const TQString& ) ),
          TQT_SLOT( slotUserCancelled( const TQString& ) ) );

      // The emitter is always called KPIM::IMAPCompletionOrder by contract
      if ( !connectDCOPSignal( 0, "KPIM::IMAPCompletionOrder", "orderChanged()",
            "slotIMAPCompletionOrderChanged()", false ) )
        kdError() << "AddresseeLineEdit: connection to orderChanged() failed" << endl;

      connect( s_LDAPTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( slotStartLDAPLookup() ) );
      connect( s_LDAPSearch, TQT_SIGNAL( searchData( const KPIM::LdapResultList& ) ),
          TQT_SLOT( slotLDAPSearchData( const KPIM::LdapResultList& ) ) );

      m_completionInitialized = true;
    }
  }
}

AddresseeLineEdit::~AddresseeLineEdit()
{
  if ( s_LDAPSearch && s_LDAPLineEdit == this )
    stopLDAPLookup();
}

void AddresseeLineEdit::setFont( const TQFont& font )
{
  KLineEdit::setFont( font );
  if ( m_useCompletion )
    completionBox()->setFont( font );
}

void AddresseeLineEdit::allowSemiColonAsSeparator( bool useSemiColonAsSeparator )
{
  m_useSemiColonAsSeparator = useSemiColonAsSeparator;
}

void AddresseeLineEdit::allowDistributionLists( bool allowDistLists )
{
  m_allowDistLists = allowDistLists;
}

void AddresseeLineEdit::keyPressEvent( TQKeyEvent *e )
{
  bool accept = false;

  if ( KStdAccel::shortcut( KStdAccel::SubstringCompletion ).contains( KKey( e ) ) ) {
    //TODO: add LDAP substring lookup, when it becomes available in KPIM::LDAPSearch
    updateSearchString();
    doCompletion( true );
    accept = true;
  } else if ( KStdAccel::shortcut( KStdAccel::TextCompletion ).contains( KKey( e ) ) ) {
    int len = text().length();

    if ( len == cursorPosition() ) { // at End?
      updateSearchString();
      doCompletion( true );
      accept = true;
    }
  }

  const TQString oldContent = text();
  if ( !accept )
    KLineEdit::keyPressEvent( e );

  // if the text didn't change (eg. because a cursor navigation key was pressed)
  // we don't need to trigger a new search
  if ( oldContent == text() )
    return;

  if ( e->isAccepted() ) {
    updateSearchString();
    TQString searchString( m_searchString );
    //LDAP does not know about our string manipulation, remove it
    if ( m_searchExtended )
        searchString = m_searchString.mid( 1 );

    if ( m_useCompletion && s_LDAPTimer != NULL ) {
      if ( *s_LDAPText != searchString || s_LDAPLineEdit != this )
        stopLDAPLookup();

      *s_LDAPText = searchString;
      s_LDAPLineEdit = this;
      s_LDAPTimer->start( 500, true );
    }
  }
}

void AddresseeLineEdit::insert( const TQString &t )
{
  if ( !m_smartPaste ) {
    KLineEdit::insert( t );
    return;
  }

  //kdDebug(5300) << "     AddresseeLineEdit::insert( \"" << t << "\" )" << endl;

  TQString newText = t.stripWhiteSpace();
  if ( newText.isEmpty() )
    return;

  // remove newlines in the to-be-pasted string
  TQStringList lines = TQStringList::split( TQRegExp("\r?\n"), newText, false );
  for ( TQStringList::iterator it = lines.begin();
       it != lines.end(); ++it ) {
    // remove trailing commas and whitespace
    (*it).remove( TQRegExp(",?\\s*$") );
  }
  newText = lines.join( ", " );

  if ( newText.startsWith("mailto:") ) {
    KURL url( newText );
    newText = url.path();
  }
  else if ( newText.find(" at ") != -1 ) {
    // Anti-spam stuff
    newText.replace( " at ", "@" );
    newText.replace( " dot ", "." );
  }
  else if ( newText.find("(at)") != -1 ) {
    newText.replace( TQRegExp("\\s*\\(at\\)\\s*"), "@" );
  }

  TQString contents = text();
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
    contents = TQString::null;
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
  setEdited( true );
  setCursorPosition( pos + newText.length() );
}

void AddresseeLineEdit::setText( const TQString & text )
{
  ClickLineEdit::setText( text.stripWhiteSpace() );
}

void AddresseeLineEdit::paste()
{
  if ( m_useCompletion )
    m_smartPaste = true;

  KLineEdit::paste();
  m_smartPaste = false;
}

void AddresseeLineEdit::mouseReleaseEvent( TQMouseEvent *e )
{
  // reimplemented from TQLineEdit::mouseReleaseEvent()
  if ( m_useCompletion
       && TQApplication::clipboard()->supportsSelection()
       && !isReadOnly()
       && e->button() == MidButton ) {
    m_smartPaste = true;
  }

  KLineEdit::mouseReleaseEvent( e );
  m_smartPaste = false;
}

void AddresseeLineEdit::dropEvent( TQDropEvent *e )
{
  KURL::List uriList;
  if ( !isReadOnly() ) {
    if ( KURLDrag::canDecode(e) && KURLDrag::decode( e, uriList ) ) {
      TQString contents = text();
      // remove trailing white space and comma
      int eot = contents.length();
      while ( ( eot > 0 ) && contents[ eot - 1 ].isSpace() )
        eot--;
      if ( eot == 0 )
        contents = TQString::null;
      else if ( contents[ eot - 1 ] == ',' ) {
        eot--;
        contents.truncate( eot );
      }
      bool mailtoURL = false;
      // append the mailto URLs
      for ( KURL::List::Iterator it = uriList.begin();
            it != uriList.end(); ++it ) {
        if ( !contents.isEmpty() )
          contents.append( ", " );
        KURL u( *it );
        if ( u.protocol() == "mailto" ) {
          mailtoURL = true;
          contents.append( (*it).path() );
        }
      }
      if ( mailtoURL ) {
        setText( contents );
        setEdited( true );
        return;
      }
    } else {
      // Let's see if this drop contains a comma separated list of emails
      TQString dropData = TQString::fromUtf8( e->encodedData( "text/plain" ) );
      TQStringList addrs = splitEmailAddrList( dropData );
      if ( addrs.count() > 0 ) {
        setText( normalizeAddressesAndDecodeIDNs( dropData ) );
        setEdited( true );
        return;
      }
    }
  }

  if ( m_useCompletion )
    m_smartPaste = true;
  TQLineEdit::dropEvent( e );
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

  if ( mode == KGlobalSettings::CompletionNone  )
    return;

  if ( s_addressesDirty ) {
    loadContacts(); // read from local address book
    s_completion->setOrder( completionOrder() );
  }

  // cursor at end of string - or Ctrl+T pressed for substring completion?
  if ( ctrlT ) {
    const TQStringList completions = getAdjustedCompletionItems( false );

    if ( completions.count() > 1 )
      ; //m_previousAddresses = prevAddr;
    else if ( completions.count() == 1 )
      setText( m_previousAddresses + completions.first().stripWhiteSpace() );

    setCompletedItems( completions, true ); // this makes sure the completion popup is closed if no matching items were found

    cursorAtEnd();
    setCompletionMode( mode ); //set back to previous mode
    return;
  }


  switch ( mode ) {
    case KGlobalSettings::CompletionPopupAuto:
    {
      if ( m_searchString.isEmpty() )
        break;
    }

    case KGlobalSettings::CompletionPopup:
    {
      const TQStringList items = getAdjustedCompletionItems( true );
      setCompletedItems( items, false );
      break;
    }

    case KGlobalSettings::CompletionShell:
    {
      TQString match = s_completion->makeCompletion( m_searchString );
      if ( !match.isNull() && match != m_searchString ) {
        setText( m_previousAddresses + match );
        setEdited( true );
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
          m_searchString = TQString::null;
          setText( m_previousAddresses );
          break;
        }

        TQString match = s_completion->makeCompletion( m_searchString );

        if ( !match.isEmpty() ) {
          if ( match != m_searchString ) {
            TQString adds = m_previousAddresses + match;
            setCompletedText( adds );
          }
        } else {
          if ( !m_searchString.startsWith( "\"" ) ) {
            //try with quoted text, if user has not type one already
            match = s_completion->makeCompletion( "\"" + m_searchString );
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
            match = s_completion->makeCompletion( m_searchString );
            if ( !match.isEmpty() && match != m_searchString ) {
              TQString adds = m_previousAddresses + match;
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

void AddresseeLineEdit::slotPopupCompletion( const TQString& completion )
{
  setText( m_previousAddresses + completion.stripWhiteSpace() );
  cursorAtEnd();
//  slotMatched( m_previousAddresses + completion );
  updateSearchString();
}

void AddresseeLineEdit::slotReturnPressed( const TQString& item )
{
  Q_UNUSED( item );
  TQListBoxItem* i = completionBox()->selectedItem();
  if ( i != 0 )
    slotPopupCompletion( i->text() );
}

void AddresseeLineEdit::loadContacts()
{
  s_completion->clear();
  s_completionItemMap->clear();
  s_addressesDirty = false;
  //m_contactMap.clear();

  TQApplication::setOverrideCursor( KCursor::waitCursor() ); // loading might take a while

  KConfig config( "kpimcompletionorder" ); // The weights for non-imap kabc resources is there.
  config.setGroup( "CompletionWeights" );

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
  // Can't just use the addressbook's iterator, we need to know which subresource
  // is behind which contact.
  TQPtrList<KABC::Resource> resources( addressBook->resources() );
  for( TQPtrListIterator<KABC::Resource> resit( resources ); *resit; ++resit ) {
    KABC::Resource* resource = *resit;
    KPIM::ResourceABC* resabc = dynamic_cast<ResourceABC *>( resource );
    if ( resabc ) { // IMAP KABC resource; need to associate each contact with the subresource
      const TQMap<TQString, TQString> uidToResourceMap = resabc->uidToResourceMap();
      KABC::Resource::Iterator it;
      for ( it = resource->begin(); it != resource->end(); ++it ) {
        TQString uid = (*it).uid();
        TQMap<TQString, TQString>::const_iterator wit = uidToResourceMap.find( uid );
        const TQString subresourceLabel = resabc->subresourceLabel( *wit );
        const int weight = ( wit != uidToResourceMap.end() ) ? resabc->subresourceCompletionWeight( *wit ) : 80;
        const int idx = addCompletionSource( subresourceLabel, weight );

        //kdDebug(5300) << (*it).fullEmail() << " subres=" << *wit << " weight=" << weight << endl;
        addContact( *it, weight, idx );
      }
    } else { // KABC non-imap resource
      int weight = config.readNumEntry( resource->identifier(), 60 );
      int sourceIndex = addCompletionSource( resource->resourceName(), weight );
      KABC::Resource::Iterator it;
      for ( it = resource->begin(); it != resource->end(); ++it ) {
        addContact( *it, weight, sourceIndex );
      }
    }
  }

#ifndef KDEPIM_NEW_DISTRLISTS // new distr lists are normal contact, already done above
  int weight = config.readNumEntry( "DistributionLists", 60 );
  KABC::DistributionListManager manager( addressBook );
  manager.load();
  const TQStringList distLists = manager.listNames();
  TQStringList::const_iterator listIt;
  int idx = addCompletionSource( i18n( "Distribution Lists" ) );
  for ( listIt = distLists.begin(); listIt != distLists.end(); ++listIt ) {

    //for KGlobalSettings::CompletionAuto
    addCompletionItem( (*listIt).simplifyWhiteSpace(), weight, idx );

    //for CompletionShell, CompletionPopup
    TQStringList sl( (*listIt).simplifyWhiteSpace() );
    addCompletionItem( (*listIt).simplifyWhiteSpace(), weight, idx, &sl );

  }
#endif

  TQApplication::restoreOverrideCursor();

  if ( !m_addressBookConnected ) {
    connect( addressBook, TQT_SIGNAL( addressBookChanged( AddressBook* ) ), TQT_SLOT( loadContacts() ) );
    m_addressBookConnected = true;
  }
}

void AddresseeLineEdit::addContact( const KABC::Addressee& addr, int weight, int source )
{
#ifdef KDEPIM_NEW_DISTRLISTS
  if ( KPIM::DistributionList::isDistributionList( addr ) ) {
    //kdDebug(5300) << "AddresseeLineEdit::addContact() distribution list \"" << addr.formattedName() << "\" weight=" << weight << endl;

    if ( m_allowDistLists ) {
      //for CompletionAuto
      addCompletionItem( addr.formattedName(), weight, source );

      //for CompletionShell, CompletionPopup
      TQStringList sl( addr.formattedName() );
      addCompletionItem( addr.formattedName(), weight, source, &sl );
    }

    return;
  }
#endif
  //m_contactMap.insert( addr.realName(), addr );
  const TQStringList emails = addr.emails();
  TQStringList::ConstIterator it;
  const int prefEmailWeight = 1;     //increment weight by prefEmailWeight
  int isPrefEmail = prefEmailWeight; //first in list is preferredEmail
  for ( it = emails.begin(); it != emails.end(); ++it ) {
    //TODO: highlight preferredEmail
    const TQString email( (*it) );
    const TQString givenName = addr.givenName();
    const TQString familyName= addr.familyName();
    const TQString nickName  = addr.nickName();
    const TQString domain    = email.mid( email.find( '@' ) + 1 );
    TQString fullEmail = addr.fullEmail( email );
    //TODO: let user decide what fields to use in lookup, e.g. company, city, ...

    //for CompletionAuto
    if ( givenName.isEmpty() && familyName.isEmpty() ) {
      addCompletionItem( fullEmail, weight + isPrefEmail, source ); // use whatever is there
    } else {
      const TQString byFirstName=  "\"" + givenName + " " + familyName + "\" <" + email + ">";
      const TQString byLastName =  "\"" + familyName + ", " + givenName + "\" <" + email + ">";
      addCompletionItem( byFirstName, weight + isPrefEmail, source );
      addCompletionItem( byLastName, weight + isPrefEmail, source );
    }

    addCompletionItem( email, weight + isPrefEmail, source );

    if ( !nickName.isEmpty() ){
      const TQString byNick     =  "\"" + nickName + "\" <" + email + ">";
      addCompletionItem( byNick, weight + isPrefEmail, source );
    }

    if ( !domain.isEmpty() ){
      const TQString byDomain   =  "\"" + domain + " " + familyName + " " + givenName + "\" <" + email + ">";
      addCompletionItem( byDomain, weight + isPrefEmail, source );
    }

    //for CompletionShell, CompletionPopup
    TQStringList keyWords;
    const TQString realName  = addr.realName();

    if ( !givenName.isEmpty() && !familyName.isEmpty() ) {
      keyWords.append( givenName  + " "  + familyName );
      keyWords.append( familyName + " "  + givenName );
      keyWords.append( familyName + ", " + givenName);
    }else if ( !givenName.isEmpty() )
      keyWords.append( givenName );
    else if ( !familyName.isEmpty() )
      keyWords.append( familyName );

    if ( !nickName.isEmpty() )
      keyWords.append( nickName );

    if ( !realName.isEmpty() )
      keyWords.append( realName );

    if ( !domain.isEmpty() )
      keyWords.append( domain );

    keyWords.append( email );

    /* KMailCompletion does not have knowledge about identities, it stores emails and
     * keywords for each email. KMailCompletion::allMatches does a lookup on the
     * keywords and returns an ordered list of emails. In order to get the preferred
     * email before others for each identity we use this little trick.
     * We remove the <blank> in getAdjustedCompletionItems.
     */
    if ( isPrefEmail == prefEmailWeight )
      fullEmail.replace( " <", "  <" );

    addCompletionItem( fullEmail, weight + isPrefEmail, source, &keyWords );
    isPrefEmail = 0;

#if 0
    int len = (*it).length();
    if ( len == 0 ) continue;
    if( '\0' == (*it)[len-1] )
      --len;
    const TQString tmp = (*it).left( len );
    const TQString fullEmail = addr.fullEmail( tmp );
    //kdDebug(5300) << "AddresseeLineEdit::addContact() \"" << fullEmail << "\" weight=" << weight << endl;
    addCompletionItem( fullEmail.simplifyWhiteSpace(), weight, source );
    // Try to guess the last name: if found, we add an extra
    // entry to the list to make sure completion works even
    // if the user starts by typing in the last name.
    TQString name( addr.realName().simplifyWhiteSpace() );
    if( name.endsWith("\"") )
      name.truncate( name.length()-1 );
    if( name.startsWith("\"") )
      name = name.mid( 1 );

    // While we're here also add "email (full name)" for completion on the email
    if ( !name.isEmpty() )
      addCompletionItem( addr.preferredEmail() + " (" + name + ")", weight, source );

    bool bDone = false;
    int i = -1;
    while( ( i = name.findRev(' ') ) > 1 && !bDone ) {
      TQString sLastName( name.mid( i+1 ) );
      if( ! sLastName.isEmpty() &&
            2 <= sLastName.length() &&   // last names must be at least 2 chars long
          ! sLastName.endsWith(".") ) { // last names must not end with a dot (like "Jr." or "Sr.")
        name.truncate( i );
        if( !name.isEmpty() ){
          sLastName.prepend( "\"" );
          sLastName.append( ", " + name + "\" <" );
        }
        TQString sExtraEntry( sLastName );
        sExtraEntry.append( tmp.isEmpty() ? addr.preferredEmail() : tmp );
        sExtraEntry.append( ">" );
        //kdDebug(5300) << "AddresseeLineEdit::addContact() added extra \"" << sExtraEntry.simplifyWhiteSpace() << "\" weight=" << weight << endl;
        addCompletionItem( sExtraEntry.simplifyWhiteSpace(), weight, source );
        bDone = true;
      }
      if( !bDone ) {
        name.truncate( i );
        if( name.endsWith("\"") )
          name.truncate( name.length()-1 );
      }
    }
#endif
  }
}

void AddresseeLineEdit::addCompletionItem( const TQString& string, int weight, int completionItemSource, const TQStringList * keyWords )
{
  // Check if there is an exact match for item already, and use the max weight if so.
  // Since there's no way to get the information from KCompletion, we have to keep our own QMap
  CompletionItemsMap::iterator it = s_completionItemMap->find( string );
  if ( it != s_completionItemMap->end() ) {
    weight = QMAX( ( *it ).first, weight );
    ( *it ).first = weight;
  } else {
    s_completionItemMap->insert( string, qMakePair( weight, completionItemSource ) );
  }
  if ( keyWords == 0 )
    s_completion->addItem( string, weight );
  else
    s_completion->addItemWithKeys( string, weight, keyWords );
}

void AddresseeLineEdit::slotStartLDAPLookup()
{
  KGlobalSettings::Completion  mode = completionMode();

  if ( mode == KGlobalSettings::CompletionNone  )
    return;

  if ( !s_LDAPSearch->isAvailable() ) {
    return;
  }
  if (  s_LDAPLineEdit != this )
    return;

  startLoadingLDAPEntries();
}

void AddresseeLineEdit::stopLDAPLookup()
{
  s_LDAPSearch->cancelSearch();
  s_LDAPLineEdit = NULL;
}

void AddresseeLineEdit::startLoadingLDAPEntries()
{
  TQString s( *s_LDAPText );
  // TODO cache last?
  TQString prevAddr;
  int n = s.findRev( ',' );
  if ( n >= 0 ) {
    prevAddr = s.left( n + 1 ) + ' ';
    s = s.mid( n + 1, 255 ).stripWhiteSpace();
  }

  if ( s.isEmpty() )
    return;

  //loadContacts(); // TODO reuse these?
  s_LDAPSearch->startSearch( s );
}

void AddresseeLineEdit::slotLDAPSearchData( const KPIM::LdapResultList& adrs )
{
  if ( adrs.isEmpty() || s_LDAPLineEdit != this )
    return;

  for ( KPIM::LdapResultList::ConstIterator it = adrs.begin(); it != adrs.end(); ++it ) {
    KABC::Addressee addr;
    addr.setNameFromString( (*it).name );
    addr.setEmails( (*it).email );

    if ( !s_ldapClientToCompletionSourceMap->contains( (*it).clientNumber ) )
      updateLDAPWeights(); // we got results from a new source, so update the completion sources

    addContact( addr, (*it).completionWeight, (*s_ldapClientToCompletionSourceMap)[ (*it ).clientNumber ]  );
  }

  if ( (hasFocus() || completionBox()->hasFocus() )
       && completionMode() != KGlobalSettings::CompletionNone
       && completionMode() != KGlobalSettings::CompletionShell ) {
    setText( m_previousAddresses + m_searchString );
    // only complete again if the user didn't change the selection while we were waiting
    // otherwise the completion box will be closed
    if ( m_searchString.stripWhiteSpace() != completionBox()->currentText().stripWhiteSpace() )
      doCompletion( m_lastSearchMode );
  }
}

void AddresseeLineEdit::setCompletedItems( const TQStringList& items, bool autoSuggest )
{
    KCompletionBox* completionBox = this->completionBox();

    if ( !items.isEmpty() &&
         !(items.count() == 1 && m_searchString == items.first()) )
    {
        TQString oldCurrentText = completionBox->currentText();
        TQListBoxItem *itemUnderMouse = completionBox->itemAt(
            completionBox->viewport()->mapFromGlobal(TQCursor::pos()) );
        TQString oldTextUnderMouse;
        TQPoint oldPosOfItemUnderMouse;
        if ( itemUnderMouse ) {
            oldTextUnderMouse = itemUnderMouse->text();
            oldPosOfItemUnderMouse = completionBox->itemRect( itemUnderMouse ).topLeft();
        }

        completionBox->setItems( items );

        if ( !completionBox->isVisible() ) {
          if ( !m_searchString.isEmpty() )
            completionBox->setCancelledText( m_searchString );
          completionBox->popup();
          // we have to install the event filter after popup(), since that
          // calls show(), and that's where KCompletionBox installs its filter.
          // We want to be first, though, so do it now.
          if ( s_completion->order() == KCompletion::Weighted )
            qApp->installEventFilter( this );
        }

        // Try to re-select what was selected before, otherrwise use the first
        // item, if there is one
        TQListBoxItem* item = 0;
        if ( oldCurrentText.isEmpty()
           || ( item = completionBox->findItem( oldCurrentText ) ) == 0 ) {
            item = completionBox->item( 1 );
        }
        if ( item )
        {
          if ( itemUnderMouse ) {
              TQListBoxItem *newItemUnderMouse = completionBox->findItem( oldTextUnderMouse );
              // if the mouse was over an item, before, but now that's elsewhere,
              // move the cursor, so folks don't accidently click the wrong item
              if ( newItemUnderMouse ) {
                  TQRect r = completionBox->itemRect( newItemUnderMouse );
                  TQPoint target = r.topLeft();
                  if ( oldPosOfItemUnderMouse != target ) {
                      target.setX( target.x() + r.width()/2 );
                      TQCursor::setPos( completionBox->viewport()->mapToGlobal(target) );
                  }
              }
          }
          completionBox->blockSignals( true );
          completionBox->setSelected( item, true );
          completionBox->setCurrentItem( item );
          completionBox->ensureCurrentVisible();

          completionBox->blockSignals( false );
        }

        if ( autoSuggest )
        {
            int index = items.first().find( m_searchString );
            TQString newText = items.first().mid( index );
            setUserSelection(false);
            setCompletedText(newText,true);
        }
    }
    else
    {
        if ( completionBox && completionBox->isVisible() ) {
            completionBox->hide();
            completionBox->setItems( TQStringList() );
        }
    }
}

TQPopupMenu* AddresseeLineEdit::createPopupMenu()
{
  TQPopupMenu *menu = KLineEdit::createPopupMenu();
  if ( !menu )
    return 0;

  if ( m_useCompletion ){
    menu->setItemVisible( ShortAutoCompletion, false );
    menu->setItemVisible( PopupAutoCompletion, false );
    menu->insertItem( i18n( "Configure Completion Order..." ),
                      this, TQT_SLOT( slotEditCompletionOrder() ) );
  }
  return menu;
}

void AddresseeLineEdit::slotEditCompletionOrder()
{
  init(); // for s_LDAPSearch
  CompletionOrderEditor editor( s_LDAPSearch, this );
  editor.exec();
  if ( m_useCompletion ) {
    updateLDAPWeights();
    s_addressesDirty = true;
  }
}

void KPIM::AddresseeLineEdit::slotIMAPCompletionOrderChanged()
{
  if ( m_useCompletion )
    s_addressesDirty = true;
}

void KPIM::AddresseeLineEdit::slotUserCancelled( const TQString& cancelText )
{
  if ( s_LDAPSearch && s_LDAPLineEdit == this )
    stopLDAPLookup();
  userCancelled( m_previousAddresses + cancelText ); // in KLineEdit
}

void AddresseeLineEdit::updateSearchString()
{
  m_searchString = text();

  int n = -1;
  bool inQuote = false;
  uint searchStringLength = m_searchString.length();
  for ( uint i = 0; i < searchStringLength; ++i ) {
    if ( m_searchString[ i ] == '"' ) {
      inQuote = !inQuote;
    }
    if ( m_searchString[ i ] == '\\' &&
         (i + 1) < searchStringLength && m_searchString[ i + 1 ] == '"' ) {
      ++i;
    }
    if ( inQuote ) {
      continue;
    }
    if ( i < searchStringLength &&
         ( m_searchString[ i ] == ',' ||
           ( m_useSemiColonAsSeparator && m_searchString[ i ] == ';' ) ) ) {
      n = i;
    }
  }

  if ( n >= 0 ) {
    ++n; // Go past the ","

    int len = m_searchString.length();

    // Increment past any whitespace...
    while ( n < len && m_searchString[ n ].isSpace() )
      ++n;

    m_previousAddresses = m_searchString.left( n );
    m_searchString = m_searchString.mid( n ).stripWhiteSpace();
  } else {
    m_previousAddresses = TQString::null;
  }
}

void KPIM::AddresseeLineEdit::slotCompletion()
{
  // Called by KLineEdit's keyPressEvent for CompletionModes Auto,Popup -> new text, update search string
  // not called for CompletionShell, this is been taken care of in AddresseeLineEdit::keyPressEvent
  updateSearchString();
  if ( completionBox() )
    completionBox()->setCancelledText( m_searchString );
  doCompletion( false );
}

// not cached, to make sure we get an up-to-date value when it changes
KCompletion::CompOrder KPIM::AddresseeLineEdit::completionOrder()
{
  KConfig config( "kpimcompletionorder" );
  config.setGroup( "General" );
  const TQString order = config.readEntry( "CompletionOrder", "Weighted" );

  if ( order == "Weighted" )
    return KCompletion::Weighted;
  else
    return KCompletion::Sorted;
}

int KPIM::AddresseeLineEdit::addCompletionSource( const TQString &source, int weight )
{
  TQMap<TQString,int>::iterator it = s_completionSourceWeights->find( source );
  if ( it == s_completionSourceWeights->end() )
    s_completionSourceWeights->insert( source, weight );
  else
    (*s_completionSourceWeights)[source] = weight;

  int sourceIndex = s_completionSources->findIndex( source );
  if ( sourceIndex == -1 ) {
    s_completionSources->append( source );
    return s_completionSources->size() - 1;
  }
  else
    return sourceIndex;
}

bool KPIM::AddresseeLineEdit::eventFilter(TQObject *obj, TQEvent *e)
{
  if ( obj == completionBox() ) {
    if ( e->type() == TQEvent::MouseButtonPress ||
         e->type() == TQEvent::MouseMove ||
         e->type() == TQEvent::MouseButtonRelease ||
         e->type() == TQEvent::MouseButtonDblClick ) {
      TQMouseEvent* me = static_cast<TQMouseEvent*>( e );
      // find list box item at the event position
      TQListBoxItem *item = completionBox()->itemAt( me->pos() );
      if ( !item ) {
        // In the case of a mouse move outside of the box we don't want
        // the parent to fuzzy select a header by mistake.
        bool eat = e->type() == TQEvent::MouseMove;
        return eat;
      }
      // avoid selection of headers on button press, or move or release while
      // a button is pressed
      if ( e->type() == TQEvent::MouseButtonPress
          || me->state() & LeftButton || me->state() & MidButton
          || me->state() & RightButton ) {
        if ( itemIsHeader(item) ) {
          return true; // eat the event, we don't want anything to happen
        } else {
          // if we are not on one of the group heading, make sure the item
          // below or above is selected, not the heading, inadvertedly, due
          // to fuzzy auto-selection from QListBox
          completionBox()->setCurrentItem( item );
          completionBox()->setSelected( completionBox()->index( item ), true );
          if ( e->type() == TQEvent::MouseMove )
            return true; // avoid fuzzy selection behavior
        }
      }
    }
  }
  if ( ( obj == this ) &&
     ( e->type() == TQEvent::AccelOverride ) ) {
    TQKeyEvent *ke = static_cast<TQKeyEvent*>( e );
    if ( ke->key() == Key_Up || ke->key() == Key_Down || ke->key() == Key_Tab ) {
      ke->accept();
      return true;
    }
  }
  if ( ( obj == this ) &&
       ( e->type() == TQEvent::KeyPress || e->type() == TQEvent::KeyRelease ) &&
       completionBox()->isVisible() ) {
    TQKeyEvent *ke = static_cast<TQKeyEvent*>( e );
    int currentIndex = completionBox()->currentItem();
    if ( currentIndex < 0 ) {
      return true;
    }

    if ( ke->key() == Key_Up ) {
      //kdDebug() << "EVENTFILTER: Key_Up currentIndex=" << currentIndex << endl;
      // figure out if the item we would be moving to is one we want
      // to ignore. If so, go one further
      TQListBoxItem *itemAbove = completionBox()->item( currentIndex );
      if ( itemAbove && itemIsHeader(itemAbove) ) {
        // there is a header above us, check if there is even further up
        // and if so go one up, so it'll be selected
        if ( currentIndex > 0 && completionBox()->item( currentIndex - 1 ) ) {
          //kdDebug() << "EVENTFILTER: Key_Up -> skipping " << currentIndex - 1 << endl;
          completionBox()->setCurrentItem( itemAbove->prev() );
          completionBox()->setSelected( currentIndex - 1, true );
        } else if ( currentIndex == 0 ) {
            // nothing to skip to, let's stay where we are, but make sure the
            // first header becomes visible, if we are the first real entry
            completionBox()->ensureVisible( 0, 0 );
            //Kolab issue 2941: be sure to add email even if it's the only element.
            if ( itemIsHeader( completionBox()->item( currentIndex ) ) ) {
              currentIndex++;
            }
            completionBox()->setCurrentItem( itemAbove );
            completionBox()->setSelected( currentIndex, true );
        }
        return true;
      }
    } else if ( ke->key() == Key_Down  ) {
      // same strategy for downwards
      //kdDebug() << "EVENTFILTER: Key_Down. currentIndex=" << currentIndex << endl;
      TQListBoxItem *itemBelow = completionBox()->item( currentIndex );
      if ( itemBelow && itemIsHeader( itemBelow ) ) {
        if ( completionBox()->item( currentIndex + 1 ) ) {
          //kdDebug() << "EVENTFILTER: Key_Down -> skipping " << currentIndex+1 << endl;
          completionBox()->setCurrentItem( itemBelow->next() );
          completionBox()->setSelected( currentIndex + 1, true );
        } else {
          // nothing to skip to, let's stay where we are
          completionBox()->setCurrentItem( itemBelow );
          completionBox()->setSelected( currentIndex, true );
        }
        return true;
      }
      // special case of the last and only item in the list needing selection
      if ( !itemBelow && currentIndex == 1 ) {
        completionBox()->setSelected( currentIndex, true );
      }
      // special case of the initial selection, which is unfortunately a header.
      // Setting it to selected tricks KCompletionBox into not treating is special
      // and selecting making it current, instead of the one below.
      TQListBoxItem *item = completionBox()->item( currentIndex );
      if ( item && itemIsHeader(item) ) {
        completionBox()->setSelected( currentIndex, true );
       }
    } else if ( e->type() == TQEvent::KeyRelease &&
                ( ke->key() == Key_Tab || ke->key() == Key_Backtab ) ) {
      //kdDebug() << "EVENTFILTER: Key_Tab. currentIndex=" << currentIndex << endl;
      /// first, find the header of the current section
      TQListBoxItem *myHeader = 0;
      const int iterationstep = ke->key() == Key_Tab ?  1 : -1;
      int i = QMIN( QMAX( currentIndex - iterationstep, 0 ), completionBox()->count() - 1 );
      while ( i>=0 ) {
        if ( itemIsHeader( completionBox()->item(i) ) ) {
          myHeader = completionBox()->item( i );
          break;
        }
        i--;
      }
      Q_ASSERT( myHeader ); // we should always be able to find a header

      // find the next header (searching backwards, for Key_Backtab)
      TQListBoxItem *nextHeader = 0;
      // when iterating forward, start at the currentindex, when backwards,
      // one up from our header, or at the end
      uint j;
      if ( ke->key() == Key_Tab ) {
        j = currentIndex;
      } else {
        i = completionBox()->index( myHeader );
        if ( i == 0 ) {
          j = completionBox()->count() - 1;
        } else {
          j = ( i - 1 ) % completionBox()->count();
        }
      }
      while ( ( nextHeader = completionBox()->item( j ) ) && nextHeader != myHeader ) {
          if ( itemIsHeader(nextHeader) ) {
            break;
          }
          j = (j + iterationstep) % completionBox()->count();
      }
      if ( nextHeader && nextHeader != myHeader ) {
        TQListBoxItem *item = completionBox()->item( j + 1 );
        if ( item && !itemIsHeader(item) ) {
          completionBox()->setSelected( item, true );
          completionBox()->setCurrentItem( item );
          completionBox()->ensureCurrentVisible();
        }
      }
      return true;
    }
  }
  return ClickLineEdit::eventFilter( obj, e );
}

class SourceWithWeight {
  public:
    int weight;           // the weight of the source
    TQString sourceName;   // the name of the source, e.g. "LDAP Server"
    int index;            // index into s_completionSources

    bool operator< ( const SourceWithWeight &other ) {
      if ( weight > other.weight )
        return true;
      if ( weight < other.weight )
        return false;
      return sourceName < other.sourceName;
    }
};

const TQStringList KPIM::AddresseeLineEdit::getAdjustedCompletionItems( bool fullSearch )
{
  TQStringList items = fullSearch ?
    s_completion->allMatches( m_searchString )
    : s_completion->substringCompletion( m_searchString );

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
  // to a TQStringList which are the items of that section/source.
  TQMap<int, TQStringList> sections;
  TQStringList sortedItems;
  for ( TQStringList::Iterator it = items.begin(); it != items.end(); ++it, ++i ) {
    CompletionItemsMap::const_iterator cit = s_completionItemMap->find(*it);
    if ( cit == s_completionItemMap->end() )
      continue;
    int idx = (*cit).second;

    if ( s_completion->order() == KCompletion::Weighted ) {
      if ( lastSourceIndex == -1 || lastSourceIndex != idx ) {
        const TQString sourceLabel(  (*s_completionSources)[idx] );
        if ( sections.find(idx) == sections.end() ) {
          items.insert( it, sourceLabel );
        }
        lastSourceIndex = idx;
      }
      (*it) = (*it).prepend( s_completionItemIndentString );
      // remove preferred email sort <blank> added in  addContact()
      (*it).replace( "  <", " <" );
    }
    sections[idx].append( *it );

    if ( s_completion->order() == KCompletion::Sorted ) {
      sortedItems.append( *it );
    }
  }

  if ( s_completion->order() == KCompletion::Weighted ) {

    // Sort the sections
    TQValueList<SourceWithWeight> sourcesAndWeights;
    for ( uint i = 0; i < s_completionSources->size(); i++ ) {
      SourceWithWeight sww;
      sww.sourceName = (*s_completionSources)[i];
      sww.weight = (*s_completionSourceWeights)[sww.sourceName];
      sww.index = i;
      sourcesAndWeights.append( sww );
    }
    qHeapSort( sourcesAndWeights );

    // Add the sections and their items to the final sortedItems result list
    for( uint i = 0; i < sourcesAndWeights.size(); i++ ) {
      TQStringList sectionItems = sections[sourcesAndWeights[i].index];
      if ( !sectionItems.isEmpty() ) {
        sortedItems.append( sourcesAndWeights[i].sourceName );
        TQStringList sectionItems = sections[sourcesAndWeights[i].index];
        for ( TQStringList::Iterator sit( sectionItems.begin() ), send( sectionItems.end() );
              sit != send; ++sit ) {
          sortedItems.append( *sit );
        }
      }
    }
  } else {
    sortedItems.sort();
  }
  return sortedItems;
}
#include "addresseelineedit.moc"
