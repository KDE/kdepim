/* memofile.cc			KPilot
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

#include "memofile.h"

#include <QtCore/QTextStream>
#include <QtCore/QDateTime>

Memofile::Memofile(PilotMemo * memo, QString categoryName, QString fileName, QString baseDirectory) :
		PilotMemo(memo,memo->text()), _categoryName(categoryName), _filename(fileName),  _baseDirectory(baseDirectory)
{
	_lastModified = 0;
	_size = 0;
	_modified = _modifiedByPalm = false;
}

Memofile::Memofile(recordid_t id, int category, uint lastModifiedTime, uint size,
                   QString categoryName, QString fileName, QString baseDirectory) :
		PilotMemo(),  _categoryName(categoryName),
		_filename(fileName),_baseDirectory(baseDirectory)
{
	setID(id);
	PilotRecordBase::setCategory(category);
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
	PilotRecordBase::setCategory(category);
	_modified = true;
	_modifiedByPalm = false;
	_lastModified = 0;
	_size = 0;
}

bool Memofile::load()
{
	FUNCTIONSETUP;
	if (filename().isEmpty())
	{
		DEBUGKPILOT << ": I was asked to load, but have no filename to load.";
		return false;
	}

	QFile f( filenameAbs() );
	if ( !f.open( QIODevice::ReadOnly ) )
	{
		DEBUGKPILOT << ": Couldn't open file: [" << filenameAbs() << "] to read.";
		return false;
	}

	QTextStream ts( &f );

	QString text,title,body;
	title = filename();
	body = ts.readAll();

	// funky magic.  we want the text of the memofile to have the filename
	// as the first line....
	if (body.startsWith(title)) {
		text = body;
	} else {
		DEBUGKPILOT << ": text of your memofile: [" << filename()
 			<< "] didn't include the filename as the first line.  fixing it...";
		text = title + CSL1("\n") + body;
	}
	
	// check length of text.  if it's over the allowable length, warn user.
	// NOTE: We don't need to truncate this here, since PilotMemo::setText()
	// does it for us.
	int _len = text.length();
	int _maxlen = PilotMemo::MAX_MEMO_LEN;
	if (_len > _maxlen) {
		DEBUGKPILOT << ": memofile: [" << filename()
		 			<< "] length: [" << _len << "] is over maximum: ["
		 			<< _maxlen << "] and will be truncated to fit.";
	}

	setText(text);
	f.close();

	return true;
}

void Memofile::setID(recordid_t i)
{
	if (i != id())
		_modifiedByPalm = true;

	PilotMemo::setID(i);
}

bool Memofile::save()
{
	bool result = true;

	if ((isModified() && isLoaded()) || _modifiedByPalm) {
		result = saveFile();
	}

	return result;
}

bool Memofile::deleteFile()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << ": deleting file: [" << filenameAbs() << "].";
	return QFile::remove(filenameAbs());
}

bool Memofile::saveFile()
{
	FUNCTIONSETUP;

	if (filename().isEmpty()) {
		DEBUGKPILOT << ": I was asked to save, but have no filename to save to.";
		return false;
	}

	DEBUGKPILOT << ": saving memo to file: [" << filenameAbs() << ']';


	QFile f( filenameAbs() );
	if ( !f.open( QIODevice::WriteOnly ) ) {
		DEBUGKPILOT << ": Couldn't open file: [" << filenameAbs() << "] to write your memo to.  "
			<< "This won't end well.";
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
	// first, check to see if this file is deleted....
	if (!fileExists()) {
		return true;
	}

	bool modByTimestamp = false;
	bool modBySize = false;

	if (_lastModified > 0)
		modByTimestamp = isModifiedByTimestamp();

	if (_size > 0)
		modBySize = isModifiedBySize();

	bool ret = _modified || modByTimestamp || modBySize;

	return ret;
}

bool Memofile::isModifiedByTimestamp()
{
	if (_lastModified <=0) {
		return true;
	}

	uint lastModifiedTime = getFileLastModified();
	if ( lastModifiedTime != _lastModified) {
		return true;
	}

	return false;
}

bool Memofile::isModifiedBySize()
{
	if (_size <=0) {
		return true;
	}

	uint size = getFileSize();
	if ( size != _size) {
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
