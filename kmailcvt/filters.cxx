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

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <krun.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <kabc/stdaddressbook.h>

#include "filters.hxx"
#include "kmailcvt.h"

//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

FilterInfo::FilterInfo( KImportPageDlg* dlg, QWidget* parent )
  : m_dlg( dlg ),
    m_parent( parent )
{
}

FilterInfo::~FilterInfo()
{
}

void FilterInfo::from( const QString& from )
{
  m_dlg->_from->setText( from );
}

void FilterInfo::to( const QString& to )
{
  m_dlg->_to->setText( to );
}

void FilterInfo::current( const QString& current )
{
  m_dlg->_current->setText( current );
}

void  FilterInfo::current( int percent )
{
  m_dlg->_done_current->setProgress( percent );
}

void  FilterInfo::overall( int percent )
{
  m_dlg->_done_overall->setProgress( percent );
}

void FilterInfo::log( const QString& log )
{
  m_dlg->_log->insertItem( log );
  m_dlg->_log->setCurrentItem( m_dlg->_log->count() - 1 );
  m_dlg->_log->centerCurrentItem();
}

void FilterInfo::clear()
{
  m_dlg->_log->clear();
  current();
  overall();
  current( QString::null );
  from( QString::null );
  to( QString::null );
}

void FilterInfo::alert( const QString& message )
{
  KMessageBox::information( m_parent, message );
}

//////////////////////////////////////////////////////////////////////////////////
//
// The generic filter class
//
//////////////////////////////////////////////////////////////////////////////////

namespace
{
  QValueList< Filter::Creator >& registry()
  {
    static QValueList< Filter::Creator > list;
    return list;
  }
}

void Filter::registerFilter( Creator create )
{
  registry().append( create );
}

Filter::List Filter::createFilters()
{
  List result;
  for ( QValueList< Creator >::ConstIterator it = registry().begin();
        it != registry().end(); ++it ) result.append( ( *it )() );
  return result;
}

Filter::Filter( const QString& name, const QString& author,
                const QString& info )
  : m_name( name ),
    m_author( author ),
    m_info( info )
{
}

#if 0
KMail::~KMail()
{
  m_info->log("kmail has adopted the (new) folders and messages");
}
#endif

bool Filter::addMessage( FilterInfo* info, const QString& folderName,
                         const QString& msgPath )
{
  if ( !kapp->dcopClient()->isApplicationRegistered( "kmail" ) )
    KApplication::startServiceByDesktopName( "kmail", QString::null ); // Will wait until kmail is started

    DCOPReply reply = DCOPRef( "kmail", "KMailIface" )
                        .call( "dcopAddMessage", folderName, msgPath );
  if ( !reply.isValid() )
  {
    info->alert( i18n( "<b>Fatal:</b> Unable to start KMail for DCOP communication. "
                       "Make sure <i>kmail</i> is installed." ) );
    return false;
  }

  switch ( int( reply ) )
  {
    case -1:
      info->alert( i18n( "Cannot make folder %1 in KMail" ).arg( folderName ) );
      return false;
    case -2:
      info->alert( i18n( "Cannot add message to folder %1 in KMail" ).arg( folderName ) );
      return false;
    case 0:
      info->alert( i18n( "Error while adding message to folder %1 in KMail" ).arg( folderName ) );
      return false;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
//
// This is the kab class, it provides the interface to KAddressBook.
// This one uses kabAPI 10
//
//////////////////////////////////////////////////////////////////////////////////

KAb::KAb() : mAddressBook(0)
{
}

KAb::~KAb()
{
}

bool KAb::kabStart(FilterInfo *info)
{
  mAddressBook = KABC::StdAddressBook::self();
  mTicket = mAddressBook->requestSaveTicket();
  if ( !mTicket ) {
     info->alert(i18n("Unable to store imported data in address book."));
     return false;
  }
  return true;
}

void KAb::kabStop(FilterInfo *info)
{
  Q_UNUSED(info);
  mAddressBook->save( mTicket );
}

bool KAb::checkStr( QString &str )
{
  if ( str == QString::null ) return false;
  str = str.stripWhiteSpace();
  if ( str.length() ) return true;
  return false;
}

bool KAb::kabAddress(FilterInfo *_info,QString adrbookname,
                      QString givenname, QString email,
                      QString title,QString firstname,
                      QString additionalname,QString lastname,
		      QString nickname,
                      QString address,QString town,
                      QString /*state*/,QString zip,QString country,
                      QString organization,QString department,
                      QString subDep,QString job,
                      QString tel,QString fax,QString mobile,QString modem,
                      QString homepage,QString talk,
                      QString comment,QString /*birthday*/
                     )
{
  // initialize

  info=_info;

  // first check if givenname already exists...

  KABC::Addressee a;

  QString note;

  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    if ( givenname == (*it).formattedName() ) {
      a = *it;
      break;
    }
  }

  // Now we've got a valid addressbook entry, fill it up.

  a.setFormattedName( givenname );

  if ( checkStr( email ) ) a.insertEmail( email );

  if ( checkStr( title ) ) a.setTitle( title );
  if ( checkStr( firstname ) ) a.setGivenName ( firstname );
  if ( checkStr( additionalname ) ) a.setAdditionalName( additionalname );
  if ( checkStr( lastname ) ) a.setFamilyName( lastname );
  if ( checkStr( nickname ) ) a.setNickName( nickname );

  KABC::Address addr;
  
  addr.setId( adrbookname );
  
  if ( checkStr( town ) ) addr.setLocality( town );
  if ( checkStr( country ) ) addr.setCountry( country );
  if ( checkStr( zip ) ) addr.setPostalCode( zip );
  if ( checkStr( address ) ) addr.setLabel( address );
  a.insertAddress( addr );

  if ( checkStr( organization ) ) a.setOrganization( organization );

  if ( checkStr( department ) ) a.insertCustom("KADDRESSBOOK", "X-Department", department);
  if ( checkStr( subDep ) ) a.insertCustom("KADDRESSBOOK", "X-Office", subDep);
  if ( checkStr( job ) ) note.append( job + "\n" );

  if ( checkStr( tel ) )
    a.insertPhoneNumber( KABC::PhoneNumber( tel, KABC::PhoneNumber::Voice ) );
  if ( checkStr( fax ) )
    a.insertPhoneNumber( KABC::PhoneNumber( fax, KABC::PhoneNumber::Fax ) );
  if ( checkStr( mobile ) )
    a.insertPhoneNumber( KABC::PhoneNumber( mobile, KABC::PhoneNumber::Cell ) );
  if ( checkStr( modem ) )
    a.insertPhoneNumber( KABC::PhoneNumber( modem, KABC::PhoneNumber::Modem ) );

  if ( checkStr( homepage ) ) a.setUrl( KURL( homepage ) );
  if ( checkStr( talk ) ) note.append( "Talk: " + talk + "\n" );

  if ( checkStr( comment ) ) note.append( comment );

  a.setNote( note );

  mAddressBook->insertAddressee( a );

  a.dump();

  return true;
}

// vim: ts=2 sw=2 et
