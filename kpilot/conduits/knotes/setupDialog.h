/* setupDialog.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the KNotes conduit, a conduit for KPilot that
** synchronises the Pilot's memo pad application with KNotes.
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

#ifndef __KNOTES_SETUP_H
#define __KNOTES_SETUP_H

class QLabel;
class QLineEdit;
class QCheckBox;
class KConfig;

#include "gsetupDialog.h"

class KNotesGeneralPage : public setupDialogPage
{
	Q_OBJECT

public:
	KNotesGeneralPage(setupDialog *,KConfig& );

	virtual int commitChanges(KConfig&);

protected:
	QCheckBox *fDeleteNoteForMemo;
} ;

class KNotesOptions : public setupDialog
{
	Q_OBJECT

friend class KNotesConduit;
friend class KNotesGeneralPage;
public:
	KNotesOptions(QWidget *parent);

protected:
	static const QString KNotesGroup;
};

#endif


// $Log:$
