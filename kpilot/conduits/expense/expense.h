/* expense.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _EXPENSE_CONDUIT_H
#define _EXPENSE_CONDUIT_H

#include "baseConduit.h"

class PilotRecord;


class ExpenseConduit : public BaseConduit
{
public:
	ExpenseConduit(eConduitMode mode);
	virtual ~ExpenseConduit();
  
	virtual void doSync();
	virtual QWidget* aboutAndSetup();

	virtual const char* dbInfo();
	virtual void doTest();

protected:
};

#endif

// $Log:$
