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

#include "filterinfogui.h"
#include <KMessageBox>
#include <KApplication>

using namespace MailImporter;

FilterInfoGui::FilterInfoGui()
{
}

FilterInfoGui::~FilterInfoGui()
{
}
void FilterInfoGui::setStatusMessage( const QString& status )
{
}

void FilterInfoGui::setFrom( const QString& from )
{
}

void FilterInfoGui::setTo( const QString& to )
{
}

void FilterInfoGui::setCurrent( const QString& current )
{
}

void  FilterInfoGui::setCurrent( int percent )
{
}

void  FilterInfoGui::setOverall( int percent )
{
}

void FilterInfoGui::addErrorLogEntry( const QString& log )
{
}

void FilterInfoGui::addInfoLogEntry( const QString& log )
{
}

void FilterInfoGui::clear()
{
}

void FilterInfoGui::alert( const QString& message )
{
}

QWidget *FilterInfoGui::parent() 
{ 
  return 0; 
}
