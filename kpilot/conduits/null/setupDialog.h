// setupDialog.h for null-conduit
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$



// REVISION HISTORY 
//
// 3.1b12	By Adriaan de Groot; initial revision.
//


#ifndef __NULL_SETUP_H
#define __NULL_SETUP_H

class QLabel;
class QLineEdit;

#include "gsetupDialog.h"


class NullPage : public setupDialogPage
{
	Q_OBJECT

public:
	NullPage(setupDialog *,KConfig *);

	virtual int commitChanges(KConfig *);

	virtual const char *tabName();

private:
	QLabel *textFieldLabel;
	QLineEdit *textField;
	QLabel *generalLabel;
} ;

class NullOptions : public setupDialog
{
	Q_OBJECT

public:
	NullOptions(QWidget *parent);

	static const char *configGroup();
	virtual const char *groupName();
};

#endif
