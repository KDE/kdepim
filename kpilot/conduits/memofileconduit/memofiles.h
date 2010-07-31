#ifndef _MEMOFILE_MEMOFILES_H
#define _MEMOFILE_MEMOFILES_H
/* memofiles.h			KPilot
**
** Copyright (C) 2004-2007 by Jason 'vanRijn' Kasper
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"
#include <tqmap.h>

#include "memofile.h"

typedef TQMap<int, TQString> MemoCategoryMap;

class Memofile;

class Memofiles {

public:

	Memofiles (MemoCategoryMap & categories, PilotMemoInfo &appInfo, 
		TQString & baseDirectory, CUDCounter &fCtrHH);
	~Memofiles();

	void load(bool loadAll);
	void save();
	void eraseLocalMemos();
	void setPilotMemos (TQPtrList<PilotMemo> & memos);
	void addModifiedMemo (PilotMemo * memo);
	void deleteMemo (PilotMemo * memo);

	bool isFirstSync();
	bool isReady() { return _ready; };

	TQPtrList<Memofile> getModified();
	TQPtrList<Memofile> getAll() { return _memofiles; } ;
	Memofile * find (const TQString & category, const TQString & filename);
	Memofile * find (recordid_t id);

	MemoCategoryMap readCategoryMetadata();
	void setCategories(MemoCategoryMap map) { _categories = map; } ;

	static TQString FIELD_SEP;
	static TQString sanitizeName(TQString name);

	int count() { return _memofiles.count(); }

private:

	MemoCategoryMap _categories;
	PilotMemoInfo &_memoAppInfo;
	TQString & _baseDirectory;
	CUDCounter &_cudCounter;
	TQPtrList<Memofile> _memofiles;

	bool  loadFromMetadata();
	bool  ensureDirectoryReady();
	bool  checkDirectory(TQString & dir);
	bool  saveMemoMetadata();
	bool  saveCategoryMetadata();
	bool  saveMemos();
	bool  folderRemove(const TQDir & dir);

	TQString filename(PilotMemo * memo);


	TQString _categoryMetadataFile;
	TQString _memoMetadataFile;

	bool _metadataLoaded;
	bool _ready;

};
#endif //MEMOFILES_H

