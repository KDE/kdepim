/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "importfilterinfogui.h"
#include "importmailpage.h"
#include <KMessageBox>
#include <KApplication>

ImportFilterInfoGui::ImportFilterInfoGui(ImportMailPage* parent)
  : MailImporter::FilterInfoGui(),
    m_parent( parent )
{
}

ImportFilterInfoGui::~ImportFilterInfoGui()
{
}

void ImportFilterInfoGui::setStatusMessage( const QString& status )
{
}

void ImportFilterInfoGui::setFrom( const QString& from )
{
}

void ImportFilterInfoGui::setTo( const QString& to )
{
}

void ImportFilterInfoGui::setCurrent( const QString& current )
{
  kapp->processEvents();
}

void  ImportFilterInfoGui::setCurrent( int percent )
{
  kapp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  ImportFilterInfoGui::setOverall( int percent )
{
}

void ImportFilterInfoGui::addInfoLogEntry( const QString& log )
{
  kapp->processEvents();
}

void ImportFilterInfoGui::addErrorLogEntry( const QString& log )
{
  kapp->processEvents();
}


void ImportFilterInfoGui::clear()
{
  setCurrent();
  setOverall();
  setCurrent( QString() );
  setFrom( QString() );
  setTo( QString() );
}

void ImportFilterInfoGui::alert( const QString& message )
{
}

QWidget *ImportFilterInfoGui::parent()
{
  return m_parent;
}
  
