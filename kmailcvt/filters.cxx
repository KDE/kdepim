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
#include <kurl.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kmailinterface.h>
#include "filters.hxx"
#include "kmailcvt.h"


//////////////////////////////////////////////////////////////////////////////////
//
// The API to the kmailcvt dialog --> Gives the import filter access to
// put information on the dialog.
//
//////////////////////////////////////////////////////////////////////////////////

bool FilterInfo::s_terminateASAP = false;

FilterInfo::FilterInfo( KImportPageDlg* dlg, QWidget* parent , bool _removeDupMsg)
  : m_dlg( dlg ),
    m_parent( parent )
{
  removeDupMsg = _removeDupMsg;
  s_terminateASAP = false;
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
  kapp->processEvents();
}

void  FilterInfo::setCurrent( int percent )
{
  m_dlg->_done_current->setValue( percent );
  kapp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  FilterInfo::setOverall( int percent )
{
  m_dlg->_done_overall->setValue( percent );
}

void FilterInfo::addLog( const QString& log )
{
  m_dlg->_log->addItem( log );
  m_dlg->_log->setCurrentItem( m_dlg->_log->item(m_dlg->_log->count() - 1 ));
  kapp->processEvents();
}

void FilterInfo::clear()
{
  m_dlg->_log->clear();
  setCurrent();
  setOverall();
  setCurrent( QString() );
  setFrom( QString() );
  setTo( QString() );
}

void FilterInfo::alert( const QString& message )
{
  KMessageBox::information( m_parent, message );
}

void FilterInfo::terminateASAP()
{
  s_terminateASAP = true;
}

bool FilterInfo::shouldTerminate()
{
  return s_terminateASAP;
}

//////////////////////////////////////////////////////////////////////////////////
//
// The generic filter class
//
//////////////////////////////////////////////////////////////////////////////////


Filter::Filter( const QString& name, const QString& author,
                const QString& info )
  : m_name( name ),
    m_author( author ),
    m_info( info )
{
    //public
    count_duplicates = 0;
}

bool Filter::addMessage( FilterInfo* info, const QString& folderName,
                         const QString& msgPath, const QString &  msgStatusFlags)
{
  KUrl msgURL;
  msgURL.setPath( msgPath );

  QDBusConnectionInterface * sessionBus = 0;
  sessionBus = QDBusConnection::sessionBus().interface();
  if ( sessionBus && !sessionBus->isServiceRegistered( "org.kde.kmail" ).value() )
    KToolInvocation::startServiceByDesktopName( "kmail", QString() ); // Will wait until kmail is started

  org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
  QDBusReply<int> reply = kmail.dbusAddMessage(folderName, msgURL.url(), msgStatusFlags);

  if ( !reply.isValid() )
  {
    info->alert( i18n( "<b>Fatal:</b> Unable to start KMail for D-Bus communication: %1; %2<br />"
                       "Make sure <i>kmail</i> is installed.", reply.error().message(), reply.error().message() ) );
    return false;
  }

  switch ( int( reply ) )
  {
    case -1:
      info->alert( i18n( "Cannot make folder %1 in KMail", folderName ) );
      return false;
    case -2:
      info->alert( i18n( "Cannot add message to folder %1 in KMail", folderName ) );
      return false;
    case -4:
      count_duplicates++;
      return false;
    case 0:
      info->alert( i18n( "Error while adding message to folder %1 in KMail", folderName ) );
      return false;
  }
  return true;
}

bool Filter::addMessage_fastImport( FilterInfo* info, const QString& folderName,
                         	        const QString& msgPath, const QString& msgStatusFlags )
{
  KUrl msgURL;
  msgURL.setPath( msgPath );

  QDBusConnectionInterface * sessionBus = 0;
  sessionBus = QDBusConnection::sessionBus().interface();
  if ( sessionBus && !sessionBus->isServiceRegistered( "org.kde.kmail" ) )
    KToolInvocation::startServiceByDesktopName( "kmail", QString() ); // Will wait until kmail is started


  org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
  QDBusReply<int> reply = kmail.dbusAddMessage_fastImport(folderName, msgURL.url(), msgStatusFlags);

  if ( !reply.isValid() )
  {
    info->alert( i18n( "<b>Fatal:</b> Unable to start KMail for D-Bus communication: %1; %2<br />"
                       "Make sure <i>kmail</i> is installed.", reply.error().message(), reply.error().message() ) );
    return false;
  }

  switch ( int( reply ) )
  {
    case -1:
      info->alert( i18n( "Cannot make folder %1 in KMail", folderName ) );
      return false;
    case -2:
      info->alert( i18n( "Cannot add message to folder %1 in KMail", folderName ) );
      return false;
    case 0:
      info->alert( i18n( "Error while adding message to folder %1 in KMail", folderName ) );
      return false;
  }
  return true;
}

bool Filter::endImport()
{
    QDBusConnectionInterface * sessionBus = 0;
    sessionBus = QDBusConnection::sessionBus().interface();
    if ( sessionBus && !sessionBus->isServiceRegistered( "org.kde.kmail" ) )
    	KToolInvocation::startServiceByDesktopName( "kmail", QString() ); // Will wait until kmail is started

    org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    QDBusReply<int> reply = kmail.dbusAddMessage(QString(), QString(),QString());
    if ( !reply.isValid() ) return false;

    QDBusReply<void> reply2 = kmail.dbusResetAddMessage();
    if ( !reply2.isValid() ) return false;

    return true;
}
// vim: ts=2 sw=2 et
