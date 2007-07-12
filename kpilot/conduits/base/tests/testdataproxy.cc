/* hhdataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "testdataproxy.h"

TestDataProxy::TestDataProxy() {}

/*
	* Implement virtual methods to be able to instantiate this. The testclass 
	* will only test the non-virtual methods of DataProxy end Record.
	*/
bool TestDataProxy::isOpen() const
{
	return true;
}

bool TestDataProxy::commit()
{
	return true;
}

bool TestDataProxy::rollback()
{
	return true;
}

void TestDataProxy::loadAllRecords()
{
}
