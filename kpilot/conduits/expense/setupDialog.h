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

#ifndef __EXPENSE_SETUP_H
#define __EXPENSE_SETUP_H

class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class KConfig;

#include "gsetupDialog.h"

class ExpenseCSVPage : public setupDialogPage
{
	Q_OBJECT

public:
	ExpenseCSVPage(setupDialog *,KConfig& );

	virtual int commitChanges(KConfig&);

	typedef enum { PolicyOverwrite,
		PolicyAppend,
		PolicyRotate } RotatePolicy;
	int getPolicy() const;

public slots:
	void slotPolicyChanged();
	void slotBrowse();

protected:
	QLineEdit *fCSVFileName;
	QRadioButton *fOverWrite,*fAppend,*fRotate;
	QSpinBox *fRotateNumber;
	QPushButton *fBrowseButton;
} ;

class ExpenseDBPage : public setupDialogPage
{
	Q_OBJECT

public:
	ExpenseDBPage(setupDialog *,KConfig& );

	virtual int commitChanges(KConfig&);

	typedef enum { PolicyPostgresql,
		PolicyMysql,
		PolicyNone } DBTypePolicy;
	int getPolicy() const;

public slots:
	void slotPolicyChanged();

protected:
	QLineEdit *fDBServer, *fDBlogin, *fDBpasswd, *fDBname, *fDBtable;
	QRadioButton *fpostgresql,*fmysql,*fnone;
} ;

class ExpenseOptions : public setupDialog
{
	Q_OBJECT

friend class ExpenseConduit;
public:
	ExpenseOptions(QWidget *parent);


protected:
	static const char *ExpenseGroup;
};

#endif


// $Log$
// Revision 1.1  2001/03/04 21:47:04  adridg
// New expense conduit, non-functional but it compiles
//
