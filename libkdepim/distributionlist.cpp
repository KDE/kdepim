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
  // used when loading, so the custom field is already there
  Q_ASSERT( isDistributionList( addr ) );
}

void KPIM::DistributionList::setName( const QString &name )
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
typedef QValueList<QPair<QString, QString> > ParseList;
static ParseList parseCustom( const QString& str )
{
  ParseList res;
  const QStringList lst = QStringList::split( ';', str );
  for( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( (*it).isEmpty() )
      continue;
    // parse "uid,email"
    QStringList helpList = QStringList::split( ',', (*it) );
    Q_ASSERT( !helpList.isEmpty() );
    if ( helpList.isEmpty() )
      continue;
    const QString uid = helpList.first();
    QString email;
    Q_ASSERT( helpList.count() < 3 ); // 1 or 2 items, but not more
    if ( helpList.count() == 2 )
      email = helpList.last();
    res.append( qMakePair( uid, email ) );
  }
  return res;
}

void KPIM::DistributionList::insertEntry( const Addressee &addr, const QString &email )
{
  Q_ASSERT( !email.isEmpty() || email.isNull() ); // hopefully never called with "", would lead to confusion
  removeEntry( addr, email ); // avoid duplicates
  QString str = custom( "KADDRESSBOOK", s_customFieldName );
  // Assumption: UIDs don't contain ; nor ,
  str += ";" + addr.uid() + "," + email;
  insertCustom( "KADDRESSBOOK", s_customFieldName, str ); // replace old value
}

void KPIM::DistributionList::removeEntry( const Addressee &addr, const QString &email )
{
  Q_ASSERT( !email.isEmpty() || email.isNull() ); // hopefully never called with "", would lead to confusion
  ParseList parseList = parseCustom( custom( "KADDRESSBOOK", s_customFieldName ) );
  QString str;
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const QString thisUid = (*it).first;
    const QString thisEmail = (*it).second;
    if ( thisUid == addr.uid() && thisEmail == email ) {
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
  const QString str = addr.custom( "KADDRESSBOOK", s_customFieldName );
  return !str.isEmpty();
}

KPIM::DistributionList KPIM::DistributionList::findByName( KABC::AddressBook* book,
                                                           const QString& name,
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
  kdDebug() << k_funcinfo << "not found: " << name << endl;
  return DistributionList();
}

KPIM::DistributionList::Entry::List KPIM::DistributionList::entries( KABC::AddressBook* book ) const
{
  Entry::List res;
  const QString str = custom( "KADDRESSBOOK", s_customFieldName );
  ParseList parseList = parseCustom( str );
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const QString uid = (*it).first;
    const QString email = (*it).second;
    // look up contact
    KABC::Addressee a = book->findByUid( uid );
    if ( a.isEmpty() ) {
      // ## The old DistributionListManager had a "missing entries" list...
      kdWarning() << "Addressee not found: " << uid << endl;
    } else {
      Entry e( a, email );
      res.append( e );
    }
  }
  return res;
}

QStringList KPIM::DistributionList::emails( KABC::AddressBook* book ) const
{
  QStringList emails;

  const QString str = custom( "KADDRESSBOOK", s_customFieldName );
  ParseList parseList = parseCustom( str );
  for( ParseList::ConstIterator it = parseList.begin(); it != parseList.end(); ++it ) {
    const QString thisUid = (*it).first;
    const QString thisEmail = (*it).second;

    // look up contact
    KABC::Addressee a = book->findByUid( thisUid );
    if ( a.isEmpty() ) {
      // ## The old DistributionListManager had a "missing entries" list...
      continue;
    }

    QString email = thisEmail.isEmpty() ? a.fullEmail() :
                    a.fullEmail( thisEmail );
    if ( !email.isEmpty() ) {
      emails.append( email );
    }
  }

  return emails;
}
