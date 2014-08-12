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

using namespace MailImporter;

FilterInfoGui::FilterInfoGui()
{
}

FilterInfoGui::~FilterInfoGui()
{
}
void FilterInfoGui::setStatusMessage( const QString &status )
{
    Q_UNUSED( status );
}

void FilterInfoGui::setFrom( const QString &from )
{
    Q_UNUSED( from );
}

void FilterInfoGui::setTo( const QString &to )
{
    Q_UNUSED( to );
}

void FilterInfoGui::setCurrent( const QString &current )
{
    Q_UNUSED( current );
}

void  FilterInfoGui::setCurrent( int percent )
{
    Q_UNUSED( percent );
}

void  FilterInfoGui::setOverall( int percent )
{
    Q_UNUSED( percent );
}

void FilterInfoGui::addErrorLogEntry( const QString &log )
{
    Q_UNUSED( log );
}

void FilterInfoGui::addInfoLogEntry( const QString &log )
{
    Q_UNUSED( log );
}

void FilterInfoGui::clear()
{
}

void FilterInfoGui::alert( const QString &message )
{
    Q_UNUSED( message );
}

QWidget *FilterInfoGui::parent() 
{ 
    return 0;
}
