// setupDialog.h for knotes-conduit
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


class KNotesOptions : public setupDialog
{
	Q_OBJECT

friend class KNotesConduit;
public:
	KNotesOptions(QWidget *parent);

protected:
	static const QString KNotesGroup;
};

#endif
