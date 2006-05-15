/* memofile.cc			KPilot
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

#include "memofile.h"

Memofile::Memofile(PilotMemo * memo, QString categoryName, QString fileName, QString baseDirectory) :
		PilotMemo(memo->text()), _categoryName(categoryName), _filename(fileName),  _baseDirectory(baseDirectory)
{
	_lastModified = 0;
	_size = 0;
	setAttrib(memo->getAttrib());
	setCat(memo->getCat());
	setID(memo->getID());
	_modified = _modifiedByPalm = false;
}

Memofile::Memofile(recordid_t id, int category, uint lastModifiedTime, uint size,
                   QString categoryName, QString fileName, QString baseDirectory) :
		PilotMemo(),  _categoryName(categoryName),
		_filename(fileName),_baseDirectory(baseDirectory)
{
	setID(id);
	setCat(category);
	_lastModified = lastModifiedTime;
	_size = size;
	_modified = _modifiedByPalm = false;
}

Memofile::Memofile(int category, QString categoryName, QString fileName, QString baseDirectory) :
		PilotMemo(),
		_categoryName(categoryName), _filename(fileName), _baseDirectory(baseDirectory)
{
	setID(0);
	_new = true;
	setCat(category);
	_modified = true;
	_modifiedByPalm = false;
	_lastModified = 0;
	_size = 0;
}

bool Memofile::load()
{
	FUNCTIONSETUP;
	if (filename().isEmpty()) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": I was asked to load, but have no filename to load.  "
		<< endl;
#endif
		return false;
	}

	QFile f( filenameAbs() );
	if ( !f.open( IO_ReadOnly ) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": Couldn't open file: [" << filenameAbs() << "] to read.  "
		<< endl;
#endif
		return false;
	}

	QTextStream ts( &f );

	QString text,title,body;
	title = filename();
	body = ts.read();

	// funky magic.  we want the text of the memofile to have the filename
	// as the first line....
	if (body.startsWith(title)) {
		text = body;
	} else {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": text of your memofile: [" << filename()
 		<< "] didn't include the filename as the first line.  fixing it..." << endl;
#endif
		text = title + CSL1("\n") + body;
	}

	setText(text);
	f.close();

	return true;
}

void Memofile::setID(recordid_t id)
{
	if (id != getID())
		_modifiedByPalm = true;

	PilotMemo::setID(id);

}

bool Memofile::save()
{
	bool result = false;

	if ((isModified() && isLoaded()) || _modifiedByPalm) {
		result = saveFile();
	}

	return result;
}

bool Memofile::deleteFile()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": deleting file: [" << filenameAbs() << "]." << endl;
#endif
	return QFile::remove(filenameAbs());

}

bool Memofile::saveFile()
{
	FUNCTIONSETUP;

	if (filename().isEmpty()) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": I was asked to save, but have no filename to save to.  "
		<< endl;
#endif
		return false;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": saving memo to file: ["
	<< filenameAbs() << "]" << endl;
#endif


	QFile f( filenameAbs() );
	if ( !f.open( IO_WriteOnly ) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": Couldn't open file: [" << filenameAbs() << "] to write your memo to.  "
		<< "This won't end well." << endl;
#endif
		return false;
	}

	QTextStream stream(&f);
	stream  << text() << endl;
	f.close();

	_lastModified = getFileLastModified();
	_size = getFileSize();

	return true;

}

bool Memofile::isModified(void)
{
	FUNCTIONSETUP;
	// first, check to see if this file is deleted....
	if (!fileExists()) {
#ifdef DEBUG
		DEBUGCONDUIT << "isModified: our file doesn't exist. returning true." << endl;
#endif
		return true;
	}

	bool modByTimestamp = false;
	bool modBySize = false;

	if (_lastModified > 0)
		modByTimestamp = isModifiedByTimestamp();

	if (_size > 0)
		modBySize = isModifiedBySize();

	bool ret = _modified || modByTimestamp || modBySize;
#ifdef DEBUG
	if (ret) {
		DEBUGCONDUIT <<"isModified: " << toString() << " _modified: ["
		<< _modified << "], modByTimestamp: ["
		<< modByTimestamp << "] modBySize: [" << ret
		<< modBySize << "] returning: [" << ret
		<< "]." << endl;
	}
#endif
	return ret;
}

bool Memofile::isModifiedByTimestamp()
{
	FUNCTIONSETUP;

	if (_lastModified <=0) {
#ifdef DEBUG
		DEBUGCONDUIT <<"isModifiedByTimestamp: lastModified is <=0, returning true" << endl;
#endif
		return true;
	}

	uint lastModifiedTime = getFileLastModified();
	if ( lastModifiedTime != _lastModified) {
#ifdef DEBUG
		DEBUGCONDUIT <<"isModifiedByTimestamp: file : [" << filename()
		<< "] was modified: [" << lastModifiedTime
		<< "], which is not my: [" << _lastModified
		<< "]." << endl;
#endif
		return true;
	}

	return false;
}

bool Memofile::isModifiedBySize()
{
	FUNCTIONSETUP;

	if (_size <=0) {
#ifdef DEBUG
		DEBUGCONDUIT <<"isModifiedBySize: size is <=0, returning true" << endl;
#endif
		return true;
	}

	uint size = getFileSize();
	if ( size != _size) {
#ifdef DEBUG
		DEBUGCONDUIT <<"isModifiedBySize: file : [" << filename()
		<< "] was modified: [" << size
		<< "], which is not my: [" << _size
		<< "]." << endl;
#endif
		return true;
	}

	return false;
}

uint Memofile::getFileLastModified()
{
	QFileInfo f = QFileInfo(filenameAbs());
	uint lastModifiedTime = f.lastModified().toTime_t();
	return lastModifiedTime;
}

uint Memofile::getFileSize()
{
	QFileInfo f = QFileInfo(filenameAbs());
	uint size = f.size();
	return size;
}

// bool Memofile::operator==( const PilotMemo &p ) const
// {
// 	FUNCTIONSETUP;
//
// 	bool equals = false;
//
// 	if (getID() > 0)
// 	{
// 		equals = p.getID()==getID();
// 	}
//
// 	return equals;
// }
