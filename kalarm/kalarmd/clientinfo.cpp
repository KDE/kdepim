/*
 *  clientinfo.cpp  -  client application information
 *  Program:  KAlarm's alarm daemon (kalarmd)
 *  Copyright (C) 2001, 2004 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "adcalendar.h"
#include "clientinfo.h"

TQMap<TQCString, ClientInfo*> ClientInfo::mClients;


ClientInfo::ClientInfo(const TQCString& appName, const TQString& title,
                       const TQCString& dcopObj, const TQString& calendar, bool startClient)
	: mAppName(appName),
	  mTitle(title),
	  mDcopObject(dcopObj),
	  mCalendar(new ADCalendar(calendar, appName)),
	  mStartClient(startClient)
{
	mClients[mAppName] = this;
}

ClientInfo::ClientInfo(const TQCString& appName, const TQString& title,
                       const TQCString& dcopObj, ADCalendar* calendar, bool startClient)
	: mAppName(appName),
	  mTitle(title),
	  mDcopObject(dcopObj),
	  mCalendar(calendar),
	  mStartClient(startClient)
{
	mClients[mAppName] = this;
}

ClientInfo::~ClientInfo()
{
	delete mCalendar;
	mClients.remove(mAppName);
}

/******************************************************************************
* Set a new calendar for the specified client application.
*/
ADCalendar* ClientInfo::setCalendar(const TQString& url)
{
	if (url != mCalendar->urlString())
	{
		delete mCalendar;
		mCalendar = new ADCalendar(url, mAppName);
	}
	return mCalendar;
}

/******************************************************************************
* Return the ClientInfo object for the specified client application.
*/
ClientInfo* ClientInfo::get(const TQCString& appName)
{
	if (appName.isEmpty())
		return 0;
	TQMap<TQCString, ClientInfo*>::ConstIterator it = mClients.find(appName);
	if (it == mClients.end())
		return 0;
	return it.data();
}

/******************************************************************************
* Return the ClientInfo object for client which owns the specified calendar.
*/
ClientInfo* ClientInfo::get(const ADCalendar* cal)
{
	for (ClientInfo::ConstIterator it = ClientInfo::begin();  it != ClientInfo::end();  ++it)
		if (it.data()->calendar() == cal)
			return it.data();
	return 0;
}

/******************************************************************************
* Delete all clients.
*/
void ClientInfo::clear()
{
	TQMap<TQCString, ClientInfo*>::Iterator it;
	while ((it = mClients.begin()) != mClients.end())
		delete it.data();
}

/******************************************************************************
* Delete the client with the specified name.
*/
void ClientInfo::remove(const TQCString& appName)
{
	TQMap<TQCString, ClientInfo*>::Iterator it = mClients.find(appName);
	if (it != mClients.end())
		delete it.data();
}
