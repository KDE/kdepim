#ifndef _MEMOFILE_MEMOFILE_H
#define _MEMOFILE_MEMOFILE_H
/* memofile.h			KPilot
**
** Copyright (C) 2004-2007 by Jason 'vanRijn' Kasper
**
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


#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//

#include <time.h>  // required by pilot-link includes

#include <pi-memo.h>

#include <tqfile.h>
#include <tqdir.h>
#include <tqtextstream.h>
#include <tqtextcodec.h>

#include "pilotMemo.h"

#include "memofiles.h"

/**
 * Class that represents our filesystem memo.
 */
class Memofile : public PilotMemo
{
	public:
		Memofile(PilotMemo * memo, TQString categoryName, TQString fileName, TQString baseDirectory);
		Memofile(recordid_t id, int category, uint lastModifiedTime, uint size, TQString categoryName, TQString filename, TQString baseDirectory);
		Memofile(int category, TQString categoryName, TQString fileName, TQString baseDirectory);

		uint lastModified() const { return _lastModified; } ;
		uint size() const { return _size; } ;

		void setModifiedByPalm(bool mod) { _modifiedByPalm = mod; } ;
		void setModified(bool modified) { _modified = modified; } ;

		bool isModified(void);
		bool isModifiedByPalm() { return _modifiedByPalm; } ;
		bool isLoaded(void) { return (! text().isEmpty()); } ;
		bool isNew(void) { return _new; } ;

		bool load();

		bool fileExists() { return TQFile::exists(filenameAbs()); } ;

		void setID(recordid_t id);

		bool save();
		bool deleteFile();

		TQString toString() {
			return CSL1("id: [") + TQString::number(id())
					+ CSL1("], category:[") + _categoryName
					+ CSL1("], filename: [") + _filename + CSL1("]");
		} ;
		const TQString & getCategoryName() { return _categoryName; } ;
		const TQString & getFilename() { return _filename; } ;
		const TQString & filename() { return _filename; } ;

	private:
		bool saveFile();
		bool isModifiedByTimestamp();
		bool isModifiedBySize();

		TQString filenameAbs() { return dirName() + filename(); } ;
		TQString dirName() { return _baseDirectory + TQDir::separator() + _categoryName + TQDir::separator(); } ;
		bool setCategory(const TQString &label);
		uint getFileLastModified();
		uint getFileSize();

		bool _modifiedByPalm;
		bool _modified;
		bool _new;
		uint _lastModified;
		uint _size;

		TQString _categoryName;
		TQString _filename;
		TQString _baseDirectory;
} ;

#endif
