#ifndef _EXPENSE_SETUPDIALOG_H
#define _EXPENSE_SETUPDIALOG_H
/* setupDialog.h			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the Expense conduit, a conduit for KPilot that
** synchronises the Pilot's expense application with external files.
** It supports CSV export and (possibly) SQL database export.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qwidget.h>

class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class KConfig;

class ExpenseCSVPage : public QWidget
{
	Q_OBJECT

public:
	ExpenseCSVPage(QWidget *);

	virtual int commitChanges(KConfig &);
	void readSettings(KConfig &);

	typedef enum { PolicyOverwrite,
		PolicyAppend,
		PolicyRotate } RotatePolicy;
	int getPolicy() const;
	void setPolicy(RotatePolicy);

public slots:
	void slotPolicyChanged();
	void slotBrowse();

protected:
	QLineEdit *fCSVFileName;
	QRadioButton *fOverWrite,*fAppend,*fRotate;
	QSpinBox *fRotateNumber;
	QPushButton *fBrowseButton;
} ;

class ExpenseDBPage : public QWidget
{
	Q_OBJECT

public:
	ExpenseDBPage(QWidget *);

	virtual int commitChanges(KConfig&);
	void readSettings(KConfig &);

	typedef enum { PolicyPostgresql,
		PolicyMysql,
		PolicyNone } DBTypePolicy;
	int getPolicy() const;
	void setPolicy(DBTypePolicy);

public slots:
	void slotPolicyChanged();

protected:
	QLineEdit *fDBServer, *fDBlogin, *fDBpasswd, *fDBname, *fDBtable;
	QRadioButton *fpostgresql,*fmysql,*fnone;
} ;



// $Log$
// Revision 1.3  2001/11/25 22:03:44  adridg
// Port expense conduit to new arch. Doesn't compile yet.
//
// Revision 1.2  2001/03/14 16:56:02  molnarc
//
// CJM - Added browse button on csv export tab.
// CJM - Added database export tab and required information.
//
// Revision 1.1  2001/03/04 21:47:04  adridg
// New expense conduit, non-functional but it compiles
//
#endif
