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
#include "filterinfogui.h"

using namespace MailImporter;

bool FilterInfo::s_terminateASAP = false;

FilterInfo::FilterInfo()
  : m_removeDupMsg( false ),
    m_filterInfoGui( 0 )
  
{
  s_terminateASAP = false;
}

FilterInfo::~FilterInfo()
{
}

void FilterInfo::setFilterInfoGui( FilterInfoGui *filterinfogui )
{
  delete m_filterInfoGui;
  m_filterInfoGui = filterinfogui;
}

void FilterInfo::setStatusMsg( const QString& status )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setStatusMsg( status );
}

void FilterInfo::setFrom( const QString& from )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setFrom( from );
}

void FilterInfo::setTo( const QString& to )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setTo(to );
}

void FilterInfo::setCurrent( const QString& current )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setCurrent( current );
}

void  FilterInfo::setCurrent( int percent )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setCurrent( percent );
}

void  FilterInfo::setOverall( int percent )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->setOverall(percent );
}

void FilterInfo::addLog( const QString& log )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->addLog( log );
}

void FilterInfo::clear()
{
  if ( m_filterInfoGui )
    m_filterInfoGui->clear();
}

void FilterInfo::alert( const QString& message )
{
  if ( m_filterInfoGui )
    m_filterInfoGui->alert( message );
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

QWidget* FilterInfo::parent()
{
  if ( m_filterInfoGui )
    return m_filterInfoGui->parent();
  return 0;
}
  
