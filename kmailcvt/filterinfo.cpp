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
/* Copyright (c) 2012 Montel Laurent <montel@kde.org>                      */

#include "filterinfo.h"
#include <KApplication>
#include <KMessageBox>


bool FilterInfo::s_terminateASAP = false;

FilterInfo::FilterInfo( KImportPageDlg* dlg, QWidget* parent)
  : m_dlg( dlg ),
    m_parent( parent ),
    m_removeDupMsg( false )
{
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

bool FilterInfo::shouldTerminate() const
{
  return s_terminateASAP;
}

Akonadi::Collection FilterInfo::rootCollection() const
{
  return m_rootCollection;
}

void FilterInfo::setRootCollection( const Akonadi::Collection &collection )
{
  m_rootCollection = collection;
}

void FilterInfo::setRemoveDupMessage( bool removeDupMessage )
{
  m_removeDupMsg = removeDupMessage;
}

bool FilterInfo::removeDupMessage() const
{
  return m_removeDupMsg;
}
