#ifndef CCONDUITSETUP_H
#define CCONDUITSETUP_H
/* conduitSetup.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the widget for setting up (ie. installing and activating)
** external conduits in KPilot.
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

/* Library Includes */
#include <kdialogbase.h>

class QPushButton;
class QListBox;
class QLabel;

class QListViewItem;
class KProcess;
class ListCategorizer;

class CConduitSetup : public KDialogBase
{
	Q_OBJECT

public:
	CConduitSetup(QWidget *parent, const char * name = 0);
	virtual ~CConduitSetup();


protected:
	QString findExecPath(const QListViewItem *) const;
	void writeInstalledConduits();
	void fillLists();

	void conduitExecuted(QListViewItem *);

protected slots:
	virtual void slotUser1();
	virtual void slotUser2();
	virtual void slotUser3();
	void conduitSelected(QListViewItem *);
	void setupDone(KProcess *);
	void slotOk();

private:
	void warnNoExec(const QListViewItem *);
	void warnSetupRunning();

	ListCategorizer *categories;
	QListViewItem *active,*available;
	KProcess *conduitSetup;
	QStringList conduitPaths;
} ;

// $Log$
// Revision 1.13  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.12  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.11  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.10  2001/02/08 17:59:34  adridg
// Removed spurious #ifdefs, and the #define that goes with it. Make KPilot exit consistently after user-requested setup actions.
//
// Revision 1.9  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
