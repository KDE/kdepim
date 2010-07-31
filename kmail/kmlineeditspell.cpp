// -*- mode: C++; c-file-style: "gnu" -*-
// kmcomposewin.cpp
// Author: Markus Wuebben <markus.wuebben@kde.org>
// This code is published under the GPL.

#include "kmlineeditspell.h"

#include "recentaddresses.h"
#include "kmkernel.h"
#include "globalsettings.h"

#include <libkdepim/kvcarddrag.h>
#include <libemailfunctions/email.h>

#include <kabc/vcardconverter.h>
#include <kio/netaccess.h>

#include <kpopupmenu.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kmessagebox.h>
#include <kcompletionbox.h>
#include <klocale.h>

#include <tqevent.h>
#include <tqfile.h>
#include <tqcstring.h>
#include <tqcursor.h>


KMLineEdit::KMLineEdit(bool useCompletion,
                       TQWidget *parent, const char *name)
    : KPIM::AddresseeLineEdit(parent,useCompletion,name)
{
   allowSemiColonAsSeparator( GlobalSettings::allowSemicolonAsAddressSeparator() );
}


//-----------------------------------------------------------------------------
void KMLineEdit::keyPressEvent(TQKeyEvent *e)
{
  if ((e->key() == Key_Enter || e->key() == Key_Return) &&
      !completionBox()->isVisible())
  {
    emit focusDown();
    AddresseeLineEdit::keyPressEvent(e);
    return;
  }
  if (e->key() == Key_Up)
  {
    emit focusUp();
    return;
  }
  if (e->key() == Key_Down)
  {
    emit focusDown();
    return;
  }
  AddresseeLineEdit::keyPressEvent(e);
}


void KMLineEdit::insertEmails( const TQStringList & emails )
{
  if ( emails.empty() )
    return;

  TQString contents = text();
  if ( !contents.isEmpty() )
    contents += ',';
  // only one address, don't need kpopup to choose
  if ( emails.size() == 1 ) {
    setText( contents + emails.front() );
    return;
  }
  //multiple emails, let the user choose one
  KPopupMenu menu( this, "Addresschooser" );
  for ( TQStringList::const_iterator it = emails.begin(), end = emails.end() ; it != end; ++it )
    menu.insertItem( *it );
  const int result = menu.exec( TQCursor::pos() );
  if ( result < 0 )
    return;
  setText( contents + menu.text( result ) );
}

void KMLineEdit::dropEvent(TQDropEvent *event)
{
  TQString vcards;
  KVCardDrag::decode( event, vcards );
  if ( !vcards.isEmpty() ) {
    KABC::VCardConverter converter;
    KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::Iterator ait;
    for ( ait = list.begin(); ait != list.end(); ++ait ){
      insertEmails( (*ait).emails() );
    }
  } else {
    KURL::List urls;
    if ( KURLDrag::decode( event, urls) ) {
      //kdDebug(5006) << "urlList" << endl;
      KURL::List::Iterator it = urls.begin();
      KABC::VCardConverter converter;
      KABC::Addressee::List list;
      TQString fileName;
      TQString caption( i18n( "vCard Import Failed" ) );
      for ( it = urls.begin(); it != urls.end(); ++it ) {
        if ( KIO::NetAccess::download( *it, fileName, parentWidget() ) ) {
          TQFile file( fileName );
          file.open( IO_ReadOnly );
          TQByteArray rawData = file.readAll();
          file.close();
          TQString data = TQString::fromUtf8( rawData.data(), rawData.size() + 1 );
          list += converter.parseVCards( data );
          KIO::NetAccess::removeTempFile( fileName );
        } else {
          TQString text = i18n( "<qt>Unable to access <b>%1</b>.</qt>" );
          KMessageBox::error( parentWidget(), text.arg( (*it).url() ), caption );
        }
        KABC::Addressee::List::Iterator ait;
        for ( ait = list.begin(); ait != list.end(); ++ait )
          insertEmails((*ait).emails());
      }
    } else {
      KPIM::AddresseeLineEdit::dropEvent( event );
    }
  }
}

TQPopupMenu *KMLineEdit::createPopupMenu()
{
    TQPopupMenu *menu = KPIM::AddresseeLineEdit::createPopupMenu();
    if ( !menu )
        return 0;

    menu->insertSeparator();
    menu->insertItem( i18n( "Edit Recent Addresses..." ),
                      this, TQT_SLOT( editRecentAddresses() ) );

    return menu;
}

void KMLineEdit::editRecentAddresses()
{
  KRecentAddress::RecentAddressDialog dlg( this );
  dlg.setAddresses( KRecentAddress::RecentAddresses::self( KMKernel::config() )->addresses() );
  if ( !dlg.exec() )
    return;
  KRecentAddress::RecentAddresses::self( KMKernel::config() )->clear();
  const TQStringList addrList = dlg.addresses();
  for ( TQStringList::const_iterator it = addrList.begin(), end = addrList.end() ; it != end ; ++it )
    KRecentAddress::RecentAddresses::self( KMKernel::config() )->add( *it );
  loadContacts();
}


//-----------------------------------------------------------------------------
void KMLineEdit::loadContacts()
{
  // was: KABC::AddressLineEdit::loadAddresses()
  AddresseeLineEdit::loadContacts();

  if ( GlobalSettings::self()->showRecentAddressesInComposer() ){
    if ( KMKernel::self() ) {
      TQStringList recent =
        KRecentAddress::RecentAddresses::self( KMKernel::config() )->addresses();
      TQStringList::Iterator it = recent.begin();
      TQString name, email;
      int idx = addCompletionSource( i18n( "Recent Addresses" ) );
      for ( ; it != recent.end(); ++it ) {
        KABC::Addressee addr;
        KPIM::getNameAndMail(*it, name, email);
        addr.setNameFromString( KPIM::quoteNameIfNecessary( name ));
        addr.insertEmail( email, true );
        addContact( addr, 120, idx ); // more weight than kabc entries and more than ldap results
      }
    }
  }
}


KMLineEditSpell::KMLineEditSpell(bool useCompletion,
                       TQWidget *parent, const char *name)
    : KMLineEdit(useCompletion,parent,name)
{
}


void KMLineEditSpell::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void KMLineEditSpell::spellCheckDone( const TQString &s )
{
    if( s != text() )
        setText( s );
}

void KMLineEditSpell::spellCheckerMisspelling( const TQString &_text, const TQStringList&, unsigned int pos)
{
     highLightWord( _text.length(),pos );
}

void KMLineEditSpell::spellCheckerCorrected( const TQString &old, const TQString &corr, unsigned int pos)
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
        emit subjectTextSpellChecked();
    }
}


#include "kmlineeditspell.moc"
