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

#include <kmessagebox.h>
#include <klocale.h>

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
}

void  FilterInfo::setCurrent( int percent )
{
  m_dlg->_done_current->setProgress( percent );
  kapp->processEvents();
}

void  FilterInfo::setOverall( int percent )
{
  m_dlg->_done_overall->setProgress( percent );
}

void FilterInfo::addLog( const QString& log )
{
  m_dlg->_log->insertItem( log );
  m_dlg->_log->setCurrentItem( m_dlg->_log->count() - 1 );
  m_dlg->_log->centerCurrentItem();
}

void FilterInfo::clear()
{
  m_dlg->_log->clear();
  setCurrent();
  setOverall();
  setCurrent( QString::null );
  setFrom( QString::null );
  setTo( QString::null );
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

bool Filter::openAddressBook( FilterInfo* info )
{
  saveTicket = KABC::StdAddressBook::self()->requestSaveTicket();
  if (!saveTicket) {
    info->alert(i18n("Unable to store imported data in address book."));
    info->addLog(i18n("Unable to get exclusive access to the address book"));
    info->addLog(i18n("If you have another program loaded that may be accessing the address book, please close it"));
    info->addLog(i18n("If this error message still appears, remove all the files in %1").arg("~/.kde/share/apps/kabc/lock"));
    return false;
  }
  return true;
}

bool Filter::closeAddressBook( )
{
  return KABC::StdAddressBook::self()->save(saveTicket);
}

void Filter::addContact( const KABC::Addressee& a )
{
  KABC::StdAddressBook::self()->insertAddressee( a );
}

// vim: ts=2 sw=2 et
