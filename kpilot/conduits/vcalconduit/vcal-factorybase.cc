/* vcal-factory.cc                      KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the vcal-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "vcal-factorybase.moc"


// Configuration keys
//
//
const char *const VCalConduitFactoryBase::syncAction = "SyncAction";
const char *const VCalConduitFactoryBase::nextSyncAction = "NextSyncAction";
const char *const VCalConduitFactoryBase::archive = "SyncArchived";
const char *const VCalConduitFactoryBase::conflictResolution = "ConflictResolution";
const char * const VCalConduitFactoryBase::fullSyncOnPCChange = "FullSyncOnPCChange";
const char * const VCalConduitFactoryBase::calendarType = "CalendarType";

const char * const VCalConduitFactoryBase::calendarFile = "CalFile" ;

//const char * const VCalConduitFactoryBase::firstTime = "FirstTime" ;
//const char * const VCalConduitFactoryBase::deleteOnPilot = "DeleteOnPilot" ;
//const char * const VCalConduitFactoryBase::alwaysFullSync = "AlwaysFullSync";
