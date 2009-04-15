// -*- mode: C++; c-file-style: "gnu" -*-
// kaddrbook.cpp
// Author: Stefan Taferner <taferner@kde.org>
// This code is under GPL

#include <config.h>

#include "kaddrbook.h"

#ifdef KDEPIM_NEW_DISTRLISTS
#include "distributionlist.h"
#else
#include <kabc/distributionlist.h>
#endif

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kabc/errorhandler.h>
#include <kresources/selectdialog.h>
#include <dcopref.h>
#include <dcopclient.h>

#include <qeventloop.h>
#include <qregexp.h>

#include <unistd.h>

//-----------------------------------------------------------------------------
void KAddrBookExternal::openEmail( const QString &addr, QWidget *parent ) {
  QString email;
  QString name;

  KABC::Addressee::parseEmailAddress( addr, name, email );

  KABC::AddressBook *ab = KABC::StdAddressBook::self( true );

  // force a reload of the address book file so that changes that were made
  // by other programs are loaded
  ab->asyncLoad();

  // if we have to reload the address book then we should also wait until
  // it's completely reloaded
#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  while ( !ab->loadingHasFinished() ) {
    QApplication::eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

  KABC::Addressee::List addressees = ab->findByEmail( email );

  if ( addressees.count() > 0 ) {
    if ( kapp->dcopClient()->isApplicationRegistered( "kaddressbook" ) ){
      //make sure kaddressbook is loaded, otherwise showContactEditor
      //won't work as desired, see bug #87233
      DCOPRef call ( "kaddressbook", "kaddressbook" );
      call.send( "newInstance()" );
    }  else {
      kapp->startServiceByDesktopName( "kaddressbook" );
    }

    DCOPRef call( "kaddressbook", "KAddressBookIface" );
    call.send( "showContactEditor(QString)", addressees.first().uid() );
  } else {
    //TODO: Enable the better message at the next string unfreeze
#if 0
    QString text = i18n("<qt>The email address <b>%1</b> cannot be "
                        "found in your addressbook.</qt>").arg( email );
#else
    QString text = email + " " + i18n( "is not in address book" );
#endif
    KMessageBox::information( parent, text, QString::null, "notInAddressBook" );
  }
}

//-----------------------------------------------------------------------------
void KAddrBookExternal::addEmail( const QString& addr, QWidget *parent) {
  QString email;
  QString name;

  KABC::Addressee::parseEmailAddress( addr, name, email );

  KABC::AddressBook *ab = KABC::StdAddressBook::self( true );

  ab->setErrorHandler( new KABC::GuiErrorHandler( parent ) );

  // force a reload of the address book file so that changes that were made
  // by other programs are loaded
  ab->asyncLoad();

  // if we have to reload the address book then we should also wait until
  // it's completely reloaded
#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  while ( !ab->loadingHasFinished() ) {
    QApplication::eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

  KABC::Addressee::List addressees = ab->findByEmail( email );

  if ( addressees.isEmpty() ) {
    KABC::Addressee a;
    a.setNameFromString( name );
    a.insertEmail( email, true );

    {
      KConfig config( "kaddressbookrc" );
      config.setGroup( "General" );
      int type = config.readNumEntry( "FormattedNameType", 1 );

      QString name;
      switch ( type ) {
        case 1:
          name = a.givenName() + " " + a.familyName();
          break;
        case 2:
          name = a.assembledName();
          break;
        case 3:
          name = a.familyName() + ", " + a.givenName();
          break;
        case 4:
          name = a.familyName() + " " + a.givenName();
          break;
        case 5:
          name = a.organization();
          break;
        default:
          name = "";
          break;
      }
      name.simplifyWhiteSpace();

      a.setFormattedName( name );
    }

    if ( KAddrBookExternal::addAddressee( a ) ) {
      QString text = i18n("<qt>The email address <b>%1</b> was added to your "
                          "addressbook; you can add more information to this "
                          "entry by opening the addressbook.</qt>").arg( addr );
      KMessageBox::information( parent, text, QString::null, "addedtokabc" );
    }
  } else {
    QString text = i18n("<qt>The email address <b>%1</b> is already in your "
                        "addressbook.</qt>").arg( addr );
    KMessageBox::information( parent, text, QString::null,
                              "alreadyInAddressBook" );
  }
  ab->setErrorHandler( 0 );
}

void KAddrBookExternal::openAddressBook(QWidget *) {
  kapp->startServiceByDesktopName( "kaddressbook" );
}

void KAddrBookExternal::addNewAddressee( QWidget* )
{
  kapp->startServiceByDesktopName("kaddressbook");
  DCOPRef call("kaddressbook", "KAddressBookIface");
  call.send("newContact()");
}

bool KAddrBookExternal::addVCard( const KABC::Addressee& addressee, QWidget *parent )
{
  KABC::AddressBook *ab = KABC::StdAddressBook::self( true );
  bool inserted = false;

  ab->setErrorHandler( new KABC::GuiErrorHandler( parent ) );

  KABC::Addressee::List addressees =
      ab->findByEmail( addressee.preferredEmail() );

  if ( addressees.isEmpty() ) {
    if ( KAddrBookExternal::addAddressee( addressee ) ) {
      QString text = i18n("The VCard was added to your addressbook; "
                          "you can add more information to this "
                          "entry by opening the addressbook.");
      KMessageBox::information( parent, text, QString::null, "addedtokabc" );
      inserted = true;
    }
  } else {
    QString text = i18n("The VCard's primary email address is already in "
                        "your addressbook; however, you may save the VCard "
                        "into a file and import it into the addressbook "
                        "manually.");
    KMessageBox::information( parent, text );
    inserted = true;
  }

  ab->setErrorHandler( 0 );
  return inserted;
}

bool KAddrBookExternal::addAddressee( const KABC::Addressee& addr )
{
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );

#if KDE_IS_VERSION(3,4,89)
  // This ugly hack will be removed in 4.0
  while ( !addressBook->loadingHasFinished() ) {
    QApplication::eventLoop()->processEvents( QEventLoop::ExcludeUserInput );

    // use sleep here to reduce cpu usage
    usleep( 100 );
  }
#endif

  // Select a resource
  QPtrList<KABC::Resource> kabcResources = addressBook->resources();

  QPtrList<KRES::Resource> kresResources;
  QPtrListIterator<KABC::Resource> resIt( kabcResources );
  KABC::Resource *kabcResource;
  while ( ( kabcResource = resIt.current() ) != 0 ) {
    ++resIt;
    if ( !kabcResource->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( kabcResource );
      if ( res )
        kresResources.append( res );
    }
  }

  kabcResource = static_cast<KABC::Resource*>( KRES::SelectDialog::getResource( kresResources, 0 ) );
  if( !kabcResource ) 
     return false;
  KABC::Ticket *ticket = addressBook->requestSaveTicket( kabcResource );
  bool saved = false;
  if ( ticket ) {
    KABC::Addressee addressee( addr );
    addressee.setResource( kabcResource );
    addressBook->insertAddressee( addressee );
    saved = addressBook->save( ticket );
    if ( !saved )
      addressBook->releaseSaveTicket( ticket );
  }

  addressBook->emitAddressBookChanged();

  return saved;
}

QString KAddrBookExternal::expandDistributionList( const QString& listName )
{
  if ( listName.isEmpty() )
    return QString::null;

  const QString lowerListName = listName.lower();
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self( true );
#ifdef KDEPIM_NEW_DISTRLISTS
  KPIM::DistributionList distrList = KPIM::DistributionList::findByName( addressBook, lowerListName, false );
  if ( !distrList.isEmpty() ) {
    return distrList.emails( addressBook ).join( ", " );
  }
#else
  KABC::DistributionListManager manager( addressBook );
  manager.load();
  const QStringList listNames = manager.listNames();

  for ( QStringList::ConstIterator it = listNames.begin();
        it != listNames.end(); ++it) {
    if ( (*it).lower() == lowerListName ) {
      const QStringList addressList = manager.list( *it )->emails();
      return addressList.join( ", " );
    }
  }
#endif
  return QString::null;
}
