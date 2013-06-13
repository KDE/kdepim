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


class FilterInfo::Private
{
public:
    Private()
        : m_removeDupMsg( false ),
          m_filterInfoGui( 0 )
    {
    }
    ~Private()
    {
        delete m_filterInfoGui;
        m_filterInfoGui = 0;
    }
    Akonadi::Collection m_rootCollection;
    bool m_removeDupMsg;
    FilterInfoGui *m_filterInfoGui;
    static bool s_terminateASAP;

};


bool FilterInfo::Private::s_terminateASAP = false;

FilterInfo::FilterInfo()
    : d( new Private )
{
    Private::s_terminateASAP = false;
}

FilterInfo::~FilterInfo()
{
    delete d;
}

void FilterInfo::setFilterInfoGui( FilterInfoGui *filterinfogui )
{
    delete d->m_filterInfoGui;
    d->m_filterInfoGui = filterinfogui;
}

void FilterInfo::setStatusMessage( const QString &status )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setStatusMessage( status );
}

void FilterInfo::setFrom( const QString &from )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setFrom( from );
}

void FilterInfo::setTo( const QString &to )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setTo(to );
}

void FilterInfo::setCurrent( const QString &current )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setCurrent( current );
}

void  FilterInfo::setCurrent( int percent )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setCurrent( percent );
}

void  FilterInfo::setOverall( int percent )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->setOverall(percent );
}

void FilterInfo::addInfoLogEntry( const QString &log )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->addInfoLogEntry( log );
}

void FilterInfo::addErrorLogEntry( const QString &log )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->addErrorLogEntry( log );
}


void FilterInfo::clear()
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->clear();
}

void FilterInfo::alert( const QString &message )
{
    if ( d->m_filterInfoGui )
        d->m_filterInfoGui->alert( message );
}

void FilterInfo::terminateASAP()
{
    Private::s_terminateASAP = true;
}

bool FilterInfo::shouldTerminate() const
{
    return Private::s_terminateASAP;
}

Akonadi::Collection FilterInfo::rootCollection() const
{
    return d->m_rootCollection;
}

void FilterInfo::setRootCollection( const Akonadi::Collection &collection )
{
    d->m_rootCollection = collection;
}

void FilterInfo::setRemoveDupMessage( bool removeDupMessage )
{
    d->m_removeDupMsg = removeDupMessage;
}

bool FilterInfo::removeDupMessage() const
{
    return d->m_removeDupMsg;
}

QWidget* FilterInfo::parent()
{
    if ( d->m_filterInfoGui )
        return d->m_filterInfoGui->parent();
    return 0;
}

