/* listItem.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003 by Reinhold Kainhofer
**
** Program description
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *listitems_id =
	"$Id$";

#include "options.h"


#include <qstring.h>
#include <qlistbox.h>
#include <qlistview.h>


#ifndef _KPILOT_LISTITEMS_H
#include "listItems.h"
#endif

#ifdef DEBUG
/* static */ int PilotListItem::crt = 0;
/* static */ int PilotListItem::del = 0;
/* static */ int PilotListItem::count = 0;

/* static */ void PilotListItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname
		<< ": created=" << crt << " deletions=" << del << endl;
}
#endif

PilotListItem::PilotListItem(const QString & text,
	recordid_t pilotid, void *r) :
	QListBoxText(text),
	fid(pilotid),
	fr(r)
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
	(void) listitems_id;
}

PilotListItem::~PilotListItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}




#ifdef DEBUG
/* static */ int PilotCheckListItem::crt = 0;
/* static */ int PilotCheckListItem::del = 0;
/* static */ int PilotCheckListItem::count = 0;

/* static */ void PilotCheckListItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname
		<< ": created=" << crt << " deletions=" << del << endl;
}
#endif

PilotCheckListItem::PilotCheckListItem(QListView * parent, const QString & text, recordid_t pilotid, void *r) :
	QCheckListItem(parent, text, QCheckListItem::CheckBox),
	fid(pilotid),
	fr(r)
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
	(void) listitems_id;
}

PilotCheckListItem::~PilotCheckListItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}

void PilotCheckListItem::stateChange ( bool on)
{
	// FUNCTIONSETUP;
	QCheckListItem::stateChange(on);

}




#ifdef DEBUG
/* static */ int PilotListViewItem::crt = 0;
/* static */ int PilotListViewItem::del = 0;
/* static */ int PilotListViewItem::count = 0;

/* static */ void PilotListViewItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << fname
		<< ": created=" << crt << " deletions=" << del << endl;
}
#endif

PilotListViewItem::PilotListViewItem( QListView * parent,
	QString label1, QString label2, QString label3, QString label4,
	recordid_t pilotid, void *r):
	QListViewItem(parent, label1, label2, label3, label4,
		QString::null, QString::null, QString::null, QString::null),
	fid(pilotid),
	fr(r),
	d(new PilotListViewItemData)
{
	// FUNCTIONSETUP;
	if (d) d->valCol=-1;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
	(void) listitems_id;
}

PilotListViewItem::~PilotListViewItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}
void PilotListViewItem::setNumericCol(int col, bool numeric)
{
	// FUNCTIONSETUP;
	if (numeric)
	{
		if (!numericCols.contains(col))
			numericCols.append(col);
	}
	else
	{
		if (numericCols.contains(col))
			numericCols.remove(col);
	}
}

unsigned long PilotListViewItem::colValue(int col, bool *ok) const
{
//	FUNCTIONSETUP;
/*	if (!d)
	{
		d=new PilotListViewItemData;
		d->valCol=-1;
	}*/
	if (d->valCol!=col)
	{
		// Use true for ascending for now...
		d->val=key(col, true).toULong(&d->valOk);
		d->valCol=col;
	}
	if (ok) (*ok)=d->valOk;
	return d->val;
}

int PilotListViewItem::compare( QListViewItem *i, int col, bool ascending ) const
{
// 	FUNCTIONSETUP;
	PilotListViewItem*item=dynamic_cast<PilotListViewItem*>(i);
/*#ifdef DEBUG
	DEBUGKPILOT<<"Item of dyn cast: "<<item<<endl;
#endif*/
	if (item && numericCols.contains(col))
	{
/*#ifdef DEBUG
	DEBUGKPILOT<<"Comparing: col "<<col<<", Ascending: "<<ascending<<endl;
#endif*/
		bool ok1, ok2;
		/// Do the toULong call just once if the sorting column changed:
		unsigned long l1=colValue(col, &ok1);
		unsigned long l2=item->colValue(col, &ok2);
/*#ifdef DEBUG
	DEBUGKPILOT<<"l1="<<l1<<"(ok: "<<ok1<<"), l2="<<l2<<"(ok: "<<ok2<<")"<<endl;
#endif*/
		if (ok1 && ok2)
		{
			// Returns -1 if this item is less than i, 0 if they are
			// equal and 1 if this item is greater than i.
			int res=0;
			if (l1<l2) res=-1;
			else if (l1>l2) res=1;
			//else res=0;
/*#ifdef DEBUG
	DEBUGKPILOT<<"RESULT="<<res<<endl;
#endif*/
			return res;
		}
	}
	return QListViewItem::compare(i, col, ascending);
}

