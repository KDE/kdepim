#include "distributionlist.h"
#include <kabc/addressbook.h>

static const char* s_customFieldName = "DistributionList";

KPIM::DistributionList::DistributionList()
 : KABC::Addressee()
{
  // can't insert the custom entry here, we need to remain a null addressee
}

KPIM::DistributionList::DistributionList( const KABC::Addressee& addr )
 : KABC::Addressee( addr )
{
}

void KPIM::DistributionList::setName( const TQString &name )
{
  // We can't use Addressee::setName, the name isn't saved/loaded in the vcard (fixed in 3.4)
  Addressee::setFormattedName( name );
  // Also set family name, just in case this entry appears in the normal contacts list (e.g. old kaddressbook)
  Addressee::setFamilyName( name );
  // We're not an empty addressee anymore
  // Set the custom field to non-empty, so that isDistributionList works
  if ( custom( "KADDRESSBOOK", s_customFieldName ).isEmpty() )
    insertCustom( "KADDRESSBOOK", s_customFieldName, ";" );
}

// Helper function, to parse the contents of the custom field
// Returns a list of { uid, email }
typedef TQValueList<QPair<TQString, TQString> > ParseList;
static ParseList parseCustom( const TQString& str )
{
  ParseList res;
  const TQStringList lst = TQStringList::split( ';', str );
  for( TQStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( (*it).isEmpty() )
      continue;
    // parse "uid,email"
    TQStringList helpList = TQStringList::split( ',', (*it) );
    Q_ASSERT( !helpList.isEmpty() );
    if ( helpList.isEmpty() )
      continue;
    const TQString uid = helpList.first();
    TQString email;
    Q_ASSERT( helpList.count() < 3 ); // 1 or 2 items, but not more
    if ( helpList.count() == 2 )
      email = helpList.last();
    res.append( qMakePair( uid, email ) );
  }
  return res;
}

void KPIM::DistributionList::insertEntry( const Addressee& addr, const TQString& email )
{
  // insertEntry will removeEntry(uid), but not with formattedName
  removeEntry( addr.formattedName(), email );
  insertEntry( addr.uid(), email );
}

void KPIM::DistributionList::insertEntry( const TQString& uid, const TQString& email )
{
  Q_ASSERT( !email.isEmpty() || email.isNull() ); // hopefully never called with "", would lead to confusion
  removeEntry( uid, email ); // avoid duplicates
  TQString str = custom( "KADDRESSBOOK", s_customFieldName );
  // Assumption: UIDs don't contain ; nor ,
  str += ";" + uid + "," + email;
  insertCustom( "KADDRESSBOOK", s_customFieldName, str ); // replace old value
}

void KPIM::DistributionList::removeEntry( const Addressee& addr, const TQString& email )
{
  removeEntry( addr.uid(), email );
  // Also remove entries with the full name as uid (for the kolab thing)
  removeEntry( addr.formattedName(), email );
}

void KPIM::DistributionList::removeEntry( const TQString& uid, const TQString& email )
{
  Q_ASSERT( !email.isEmpty() || email.isNull() ); // hopefully never called with "", would lead to confusion
  ParseList parseList = parseCustom( custom( "KADDRESSBOOK", s_customFieldName ) );
  TQString str;
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const TQString thisUid = (*it).first;
    const TQString thisEmail = (*it).second;
    if ( thisUid == uid && thisEmail == email ) {
      continue; // remove that one
    }
    str += ";" + thisUid + "," + thisEmail;
  }
  if ( str.isEmpty() )
    str = ";"; // keep something, for isDistributionList to work
  insertCustom( "KADDRESSBOOK", s_customFieldName, str ); // replace old value
}

bool KPIM::DistributionList::isDistributionList( const KABC::Addressee& addr )
{
  const TQString str = addr.custom( "KADDRESSBOOK", s_customFieldName );
  return !str.isEmpty();
}

// ###### KDE4: add findByFormattedName to KABC::AddressBook
static KABC::Addressee::List findByFormattedName( KABC::AddressBook* book,
                                            const TQString& name,
                                            bool caseSensitive = true )
{
  KABC::Addressee::List res;
  KABC::AddressBook::Iterator abIt;
  for ( abIt = book->begin(); abIt != book->end(); ++abIt )
  {
    if ( caseSensitive && (*abIt).formattedName() == name )
      res.append( *abIt );
    if ( !caseSensitive && (*abIt).formattedName().lower() == name.lower() )
      res.append( *abIt );
  }
  return res;
}

KPIM::DistributionList KPIM::DistributionList::findByName( KABC::AddressBook* book,
                                                           const TQString& name,
                                                           bool caseSensitive )
{
  KABC::AddressBook::Iterator abIt;
  for ( abIt = book->begin(); abIt != book->end(); ++abIt )
  {
    if ( isDistributionList( *abIt ) ) {
      if ( caseSensitive && (*abIt).formattedName() == name )
        return *abIt;
      if ( !caseSensitive && (*abIt).formattedName().lower() == name.lower() )
        return *abIt;
    }
  }
  return DistributionList();
}

static KABC::Addressee findByUidOrName( KABC::AddressBook* book, const TQString& uidOrName, const TQString& email )
{
  KABC::Addressee a = book->findByUid( uidOrName );
  if ( a.isEmpty() ) {
    // UID not found, maybe it is a name instead.
    // If we have an email, let's use that for the lookup.
    // [This is used by e.g. the Kolab resource]
    if ( !email.isEmpty() ) {
      KABC::Addressee::List lst = book->findByEmail( email );
      KABC::Addressee::List::ConstIterator listit = lst.begin();
      for ( ; listit != lst.end(); ++listit )
        if ( (*listit).formattedName() == uidOrName ) {
          a = *listit;
          break;
        }
      if ( !lst.isEmpty() && a.isEmpty() ) { // found that email, but no match on the fullname
        a = lst.first(); // probably the last name changed
      }
    }
    // If we don't have an email, or if we didn't find any match for it, look up by full name
    if ( a.isEmpty() ) {
      // (But this has to be done here, since when loading we might not have the entries yet)
      KABC::Addressee::List lst = findByFormattedName( book, uidOrName );
      if ( !lst.isEmpty() )
        a = lst.first();
    }
  }
  return a;
}

KPIM::DistributionList::Entry::List KPIM::DistributionList::entries( KABC::AddressBook* book ) const
{
  Entry::List res;
  const TQString str = custom( "KADDRESSBOOK", s_customFieldName );
  const ParseList parseList = parseCustom( str );
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const TQString uid = (*it).first;
    const TQString email = (*it).second;
    // look up contact
    KABC::Addressee a = findByUidOrName( book, uid, email );
    if ( a.isEmpty() ) {
      // ## The old DistributionListManager had a "missing entries" list...
      kdWarning() << "Addressee not found: " << uid << endl;
    } else {
      res.append( Entry( a, email ) );
    }
  }
  return res;
}

TQStringList KPIM::DistributionList::emails( KABC::AddressBook* book ) const
{
  TQStringList emails;

  const TQString str = custom( "KADDRESSBOOK", s_customFieldName );
  ParseList parseList = parseCustom( str );
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const TQString thisUid = (*it).first;
    const TQString thisEmail = (*it).second;

    // look up contact
    KABC::Addressee a = findByUidOrName( book, thisUid, thisEmail );
    if ( a.isEmpty() ) {
      // ## The old DistributionListManager had a "missing entries" list...
      continue;
    }

    const TQString email = thisEmail.isEmpty() ? a.fullEmail() :
                          a.fullEmail( thisEmail );
    if ( !email.isEmpty() ) {
      emails.append( email );
    }
  }

  return emails;
}

TQValueList<KPIM::DistributionList>
 KPIM::DistributionList::allDistributionLists( KABC::AddressBook* book )
{
  TQValueList<KPIM::DistributionList> lst;
  KABC::AddressBook::Iterator abIt;
  for ( abIt = book->begin(); abIt != book->end(); ++abIt )
  {
    if ( isDistributionList( *abIt ) ) {
      lst.append( KPIM::DistributionList( *abIt ) );
    }
  }
  return lst;
}
