/* setupDialog.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the NULL conduit, a conduit for KPilot that
** does nothing except add a log message to the Pilot's HotSync log.
** It is also intended as a programming example.
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

#ifndef __NULL_SETUP_H
#define __NULL_SETUP_H

class QLabel;
class QLineEdit;

#include "gsetupDialog.h"


class NullPage : public setupDialogPage
{
	Q_OBJECT

public:
	NullPage(setupDialog *,KConfig&);

	virtual int commitChanges(KConfig&);

private:
	QLabel *textFieldLabel;
	QLineEdit *textField;
	QLabel *generalLabel;
	QLabel *dbLabel;
	QLineEdit *dbField;
} ;

class NullOptions : public setupDialog
{
	Q_OBJECT

friend class NullConduit;
public:
	NullOptions(QWidget *parent);

protected:
	static const QString NullGroup;
};

#endif


// $Log:$
