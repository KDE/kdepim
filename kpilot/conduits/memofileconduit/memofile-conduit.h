#ifndef _MEMOFILE_MEMOFILE_CONDUIT_H
#define _MEMOFILE_MEMOFILE_CONDUIT_H
/* memofile-conduit.h			KPilot
**
** Copyright (C) 2004-2004 by Jason 'vanRijn' Kasper
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <pi-memo.h>

#include "plugin.h"

#include "memofiles.h"

class PilotMemo;

class MemofileConduit : public ConduitAction
{
Q_OBJECT
public:
	MemofileConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~MemofileConduit();

	QString getResults();

	
protected:
	virtual bool exec();
	

protected slots:
	void process();
	
private:
	// configuration settings...
	QString	_DEFAULT_MEMODIR;
	QString	_memo_directory;
	bool	_sync_private;

	int _countDeletedToPilot;
	int _countModifiedToPilot;
	int _countNewToPilot;

	struct MemoAppInfo	fMemoAppInfo;
	QPtrList<PilotMemo> fMemoList;

	// our categories
	MemoCategoryMap fCategories;

	Memofiles * _memofiles;

	
	bool	readConfig();
	void	getAppInfo();
	QString	getCategoryName(int category);

	bool	initializeFromPilot();
	bool	loadPilotCategories();

	void 	listPilotMemos();
	
	void	getAllFromPilot();
	void	getModifiedFromPilot();
	
	bool	copyHHToPC();
	bool	copyPCToHH();
	bool	sync();
	
	int 	writeToPilot(Memofile * memofile);
	void	deleteFromPilot(Memofile* memofile);

	void 	cleanup();
	
};

#endif
