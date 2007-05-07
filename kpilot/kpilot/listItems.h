/* listItems.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines a subclasse of QListBoxText that carries
** additional information useful for Pilot records. In particular it
** carries an int (for the pilot's uid?) and a void * (for pilotrecord?)
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_LISTITEMS_H
#define _KPILOT_LISTITEMS_H

#include <q3listview.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <pi-dlp.h>
#include <Q3ListBoxText>

class PilotListItem : public Q3ListBoxText
{
public:
	PilotListItem(const QString &text, recordid_t pilotid=0, void *r=0);
	virtual ~PilotListItem();
	recordid_t id() const {return fid;};
	const void *rec() const {return fr;};


protected:
	recordid_t fid;
	void *fr;

#ifdef DEBUG
public:
	static void counts();
private:
	static int crt,del,bal,count;
#endif
};

class PilotCheckListItem : public Q3CheckListItem
{
public:
	PilotCheckListItem( Q3ListView * parent, const QString & text, recordid_t pilotid=0, void *r=0);
	virtual ~PilotCheckListItem();
	recordid_t id() const {return fid;};
	const void  *rec() const {return fr;};
protected:
	virtual void stateChange ( bool );
	recordid_t fid;
	void *fr;
#ifdef DEBUG
public:
	static void counts();
private:
	static int crt, del, bal, count;
#endif
};

struct PilotListViewItemData
{
	int valCol;
	bool valOk;
	unsigned long val;
};

class PilotListViewItem : public Q3ListViewItem
{
public:
	PilotListViewItem( Q3ListView * parent,
		QString label1, QString label2 = QString::null,
		QString label3 = QString::null, QString label4 = QString::null,
		recordid_t pilotid=0, void *r=0);
	virtual ~PilotListViewItem();
	recordid_t id() const {return fid;};
	const void  *rec() const {return fr;};
public:
	void setNumericCol(int col, bool numeric);
	int compare( Q3ListViewItem *i, int col, bool ascending ) const;
protected:
	Q3ValueList<int> numericCols;
	recordid_t fid;
	void *fr;
	// Caching to make sorting faster:
	PilotListViewItemData*d;
	unsigned long colValue(int col, bool *ok) const;
#ifdef DEBUG
public:
	static void counts();
private:
	static int crt, del, bal, count;
#endif
};


#endif
