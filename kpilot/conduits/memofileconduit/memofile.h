#ifndef _MEMOFILE_MEMOFILE_H
#define _MEMOFILE_MEMOFILE_H
/* memofile.h			KPilot
**
** Copyright (C) 2004-2004 by Jason 'vanRijn' Kasper
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include "pilotMemo.h"

#include "memofiles.h"

/**
 * Class that represents our filesystem memo.
 */
class Memofile : public PilotMemo
{
	public:
		Memofile(PilotMemo * memo, QString categoryName, QString fileName, QString baseDirectory);
		Memofile(recordid_t id, int category, uint lastModifiedTime, uint size, QString categoryName, QString filename, QString baseDirectory);
		Memofile(int category, QString categoryName, QString fileName, QString baseDirectory);
		
		uint lastModified() const { return _lastModified; } ;
		uint size() const { return _size; } ;

		void setModifiedByPalm(bool mod) { _modifiedByPalm = mod; } ;
		void setModified(bool modified) { _modified = modified; } ;
		
		bool isModified(void);
		bool isModifiedByPalm() { return _modifiedByPalm; } ;
		bool isLoaded(void) { return (! text().isEmpty()); } ;
		bool isNew(void) { return _new; } ;
		
		bool load();

		bool fileExists() { return QFile::exists(filenameAbs()); } ;

		void setID(recordid_t id);

		bool save();
		bool deleteFile();

		QString toString() {
			return CSL1("id: [") + QString::number(getID())
					+ CSL1("], category:[") + _categoryName
					+ CSL1("], filename: [") + _filename + CSL1("]");
		} ;
		const QString & getCategoryName() { return _categoryName; } ;
		const QString & getFilename() { return _filename; } ;
		const QString & filename() { return _filename; } ;

	private:
		bool saveFile();
		bool isModifiedByTimestamp();
		bool isModifiedBySize();
		
		QString filenameAbs() { return dirName() + filename(); } ;
		QString dirName() { return _baseDirectory + QDir::separator() + _categoryName + QDir::separator(); } ;
		bool setCategory(const QString &label);
		uint getFileLastModified();
		uint getFileSize();
		
		bool _modifiedByPalm;
		bool _modified;
		bool _new;
		uint _lastModified;
		uint _size;

		QString _categoryName;
		QString _filename;
		QString _baseDirectory;
} ;

#endif
