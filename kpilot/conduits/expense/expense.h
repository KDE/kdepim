#ifndef _KPILOT_EXPENSE_H
#define _KPILOT_EXPENSE_H
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


#include "plugin.h"


class PilotRecord;
class PilotDatabase;



class ExpenseConduit : public ConduitAction
{
public:
	ExpenseConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~ExpenseConduit();
  

	virtual void exec();
};


// $Log$
// Revision 1.3  2001/03/15 21:10:07  molnarc
//
//
// CJM - now it saves a csv file to the path in kpilotrc if
//       the path exists. csv file needs some work, but its
//       a start.
//
// Revision 1.2  2001/03/09 09:46:14  adridg
// Large-scale #include cleanup
//
// Revision 1.1  2001/03/04 21:47:04  adridg
// New expense conduit, non-functional but it compiles
//
#endif
