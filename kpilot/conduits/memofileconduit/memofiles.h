#ifndef _MEMOFILE_MEMOFILES_H
#define _MEMOFILE_MEMOFILES_H
/* memofiles.h			KPilot
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

#include "plugin.h"
#include <qmap.h>

#include "memofile.h"

typedef QMap<int, QString> MemoCategoryMap;

class Memofile;

class Memofiles {

public:

	Memofiles (MemoCategoryMap & categories, struct MemoAppInfo & appInfo, QString & baseDirectory);
	~Memofiles();
	
	void load(bool loadAll);
	void save();
	void eraseLocalMemos();
	void setPilotMemos (QPtrList<PilotMemo> & memos);
	void addModifiedMemo (PilotMemo * memo);
	void deleteMemo (PilotMemo * memo);

	QString getResults();
		
	bool isFirstSync();
	
	QPtrList<Memofile> getModified();
	QPtrList<Memofile> getAll() { return _memofiles; } ;
	Memofile * find (const QString & category, const QString & filename);
	Memofile * find (recordid_t id);

	static QString FIELD_SEP;

private:
   
	MemoCategoryMap & _categories;
	struct MemoAppInfo & _memoAppInfo;
	QString & _baseDirectory;
	QPtrList<Memofile> _memofiles;

	bool  loadFromMetadata();
	bool  ensureDirectoryReady();
	bool  checkDirectory(QString & dir);
	bool  saveMemoMetadata();
	bool  saveCategoryMetadata();
	bool  saveMemos();
	bool  folderRemove(const QDir & dir);

	QString filename(PilotMemo * memo);


	QString _categoryMetadataFile;
	QString _memoMetadataFile;

	int _countDeletedToLocal;
	int _countModifiedToLocal;
	int _countNewToLocal;
	
	bool _metadataLoaded;
	
};
#endif //MEMOFILES_H

