// setupDialog.h for null-conduit
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 



// REVISION HISTORY 
//
// 3.1b12	By Adriaan de Groot; initial revision.
//


#ifndef __NULL_SETUP_H
#define __NULL_SETUP_H

#include <qtabdlg.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <kfm.h>

class NullOptions : public QTabDialog
{
	Q_OBJECT

public:
	NullOptions();
	~NullOptions();

	static const char *groupName() const;

public slots:
	void commitChanges();
	void cancelChanges();

private:
	QLabel *textFieldLabel;
	QLineEdit *textField;
	QLabel *generalLabel;


	void setupWidget();
};

#endif
