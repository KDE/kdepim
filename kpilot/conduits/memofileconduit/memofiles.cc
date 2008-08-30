/* memofile-conduit.cc			KPilot
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

#include "memofiles.h"

#include "options.h"

#include "memofile.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QtCore/QTextStream>

QString Memofiles::FIELD_SEP = CSL1("\t");

Memofiles::Memofiles (MemoCategoryMap & categories, PilotMemoInfo &appInfo,
	QString & baseDirectory) :
	_categories(categories), _memoAppInfo(appInfo),
	_baseDirectory(baseDirectory)
{
	FUNCTIONSETUP;
	_memofiles.clear();
	_memoMetadataFile = _baseDirectory + QDir::separator() + CSL1(".ids");
	_categoryMetadataFile = _baseDirectory + QDir::separator() + CSL1(".categories");
	_memofiles.setAutoDelete(true);

	_ready = ensureDirectoryReady();

	_metadataLoaded = loadFromMetadata();
}

Memofiles::~Memofiles()
{
	FUNCTIONSETUP;
}

void Memofiles::load (bool loadAll)
{
	FUNCTIONSETUP;

	DEBUGKPILOT << ": now looking at all memofiles in your directory.";

	// now go through each of our known categories and look in each directory
	// for that category for memo files
	MemoCategoryMap::ConstIterator it;
	int counter = -1;

	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		int category = it.key();
		QString categoryName = it.value();
		QString categoryDirname = _baseDirectory + QDir::separator() + categoryName;

		QDir dir = QDir(categoryDirname);
		if (! dir.exists() ) {
			DEBUGKPILOT << ": category directory: [" << categoryDirname
				<< "] does not exist. skipping.";
			continue;
		}


		QStringList entries = dir.entryList(QDir::Files);
		QString file;
		for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
			file = *it;
			QFileInfo info(dir, file);

			if(info.isFile() && info.isReadable()) {
				Memofile * memofile = find(categoryName, file);
				if (NULL == memofile) {
					memofile = new Memofile(category, categoryName, file, _baseDirectory);
					memofile->setModified(true);
					_memofiles.append(memofile);
					DEBUGKPILOT
						<< ": looks like we did not know about this one until now. "
						<< "created new memofile for category: ["
						<< categoryName << "], file: [" << file << ']';

				}

				counter++;

				// okay, we should have a memofile for this file now.  see if we need
				// to load its text...
				if (memofile->isModified() || loadAll) {
					DEBUGKPILOT
					<< ": now loading text for: [" << info.filePath() << ']';
					memofile->load();
				}
			} else {
				DEBUGKPILOT << ": could not read file: [" << info.filePath() << "]. skipping it.";

			}
		} // end of iterating through files in this directory

	} // end of iterating through our categories/directories

	DEBUGKPILOT << ": looked at: [" << counter << "] files from your directories.";


	// okay, now we've loaded everything from our directories.  make one last
	// pass through our loaded memofiles and see if we need to mark any of them
	// as deleted (i.e. we created a memofile object from our metadata, but
	// the file is now gone, so it's deleted.
	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if (! memofile->fileExists()) {
			memofile->setDeleted( true );
		}
	}
}

/**
* Make sure that our directory is ready to synchronize with our
* Palm's database.  This means we need to make sure that the directory
* that our user has specified for storing his/her memos exists, as well
* as a directory inside that directory for each of his/her memo categories.
*/
bool Memofiles::ensureDirectoryReady()
{
	FUNCTIONSETUP;

	if (!checkDirectory(_baseDirectory))
		return false;

	int failures = 0;
	// now make sure that a directory for each category exists.
	QString _category_name;
	QString dir;

	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		_category_name = it.value();
		dir = _baseDirectory + QDir::separator() + _category_name;

		DEBUGKPILOT << ": checking directory: [" << dir		<< ']';

		if (!checkDirectory(dir))
			failures++;
	}

	return failures == 0;
}

bool Memofiles::checkDirectory(QString & dir)
{
	FUNCTIONSETUP;
	// make sure that the directory we're asked to write to exists
	QDir d(dir);
	QFileInfo fid( dir );

	if ( ! fid.isDir() ) {

		DEBUGKPILOT << ": directory: [" << dir << "] does not exist. creating....";

		if (!d.mkdir(dir)) {

			DEBUGKPILOT << ": could not create directory: [" << dir << "].  this will not end well.";
			return false;
		} else {
			DEBUGKPILOT << ": directory created: [" << dir << ']';

		}
	} else {
		DEBUGKPILOT << ": directory already existed: [" << dir << ']';
	}

	return true;

}

void Memofiles::eraseLocalMemos ()
{
	FUNCTIONSETUP;

	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		QString dir = _baseDirectory + QDir::separator() + it.value();

		if (!folderRemove(QDir(dir))) {
			DEBUGKPILOT << ": could not erase all local memos from: ["<< dir << ']';
		}
	}
	QDir d(_baseDirectory);
	d.remove(_memoMetadataFile);

	ensureDirectoryReady();

	_memofiles.clear();
}

void Memofiles::setPilotMemos (Q3PtrList<PilotMemo> & memos)
{
	FUNCTIONSETUP;

	PilotMemo * memo;

	_memofiles.clear();

	for ( memo = memos.first(); memo; memo = memos.next() ) {
		addModifiedMemo(memo);
	}

	DEBUGKPILOT << ": set: [" << _memofiles.count() << "] from Palm to local.";

}

bool Memofiles::loadFromMetadata ()
{
	FUNCTIONSETUP;

	_memofiles.clear();

	QFile f( _memoMetadataFile );
	if ( !f.open( QIODevice::ReadOnly ) ) {
		DEBUGKPILOT << ": ooh, bad.  could not open your memo-id file for reading.";
		return false;
	}

	QTextStream t( &f );
	Memofile * memofile;

	while ( !t.atEnd() ) {
		QString data = t.readLine();
		int errors = 0;
		bool ok;

		QStringList fields = data.split( FIELD_SEP, QString::SkipEmptyParts );
		if ( fields.count() >= 4 ) {
			int id = fields[0].toInt( &ok );
			if ( !ok )
				errors++;
			int category = fields[1].toInt( &ok );
			if ( !ok )
				errors++;
			uint lastModified = fields[2].toInt( &ok );
			if ( !ok )
				errors++;
			uint size = fields[3].toInt( &ok );
			if ( !ok )
				errors++;
			QString filename = fields[4];
			if ( filename.isEmpty() )
				errors++;

			if (errors <= 0) {
				memofile = new Memofile(id, category, lastModified, size,
				                        _categories[category], filename, _baseDirectory);
				_memofiles.append(memofile);
			}
		} else {
			errors++;
		}

		if (errors > 0) {
			DEBUGKPILOT << ": error: could not understand this line: [" << data << ']';
		}
	}

	DEBUGKPILOT << ": loaded: [" << _memofiles.count() << "] memofiles.";

	f.close();
	return true;
}

Memofile * Memofiles::find (recordid_t id)
{

	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if ( memofile->id() == id) {
			return memofile;
		}
	}

	return NULL;

}

Memofile * Memofiles::find (const QString & category, const QString & filename)
{

	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if ( memofile->getCategoryName() == category &&
		        memofile->getFilename() == filename ) {
			return memofile;
		}
	}

	return NULL;

}

void Memofiles::deleteMemo(PilotMemo * memo)
{
	FUNCTIONSETUP;
	if (! memo->isDeleted())
		return;

	Memofile * memofile = find(memo->id());
	if (memofile) {
		memofile->deleteFile();
		_memofiles.remove(memofile);
		//_cudCounter.deleted();
	}
}


void Memofiles::addModifiedMemo (PilotMemo * memo)
{
	FUNCTIONSETUP;

	if (memo->isDeleted()) {
		deleteMemo(memo);
		return;
	}

	QString debug = CSL1(": adding a PilotMemo. id: [")
	                + QString::number(memo->id()) + CSL1("], title: [")
	                + memo->getTitle() + CSL1("]. ");

	Memofile * memofile = find(memo->id());

	if (NULL == memofile) {
		//_cudCounter.created();
		debug += CSL1(" new from pilot.");
	} else {
		// we have found a local memofile that was modified on the palm.  for the time
		// being (until someone complains, etc.), we will always overwrite changes to
		// the local filesystem with changes to the palm (palm overrides local).  at
		// some point in the future, we should probably honor a user preference for
		// this...
		//_cudCounter.updated();
		_memofiles.remove(memofile);
		debug += CSL1(" modified from pilot.");
	}

	memofile = new Memofile(memo, _categories[memo->category()], filename(memo), _baseDirectory);
	memofile->setModifiedByPalm(true);
	_memofiles.append(memofile);

}

Q3PtrList<Memofile> Memofiles::getModified ()
{
	FUNCTIONSETUP;

	Q3PtrList<Memofile> modList;
	modList.clear();

	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if ( memofile->isModified() && ! memofile->isModifiedByPalm() ) {
			modList.append(memofile);
		}
	}

	DEBUGKPILOT << ": found: [" << modList.count() << "] memofiles modified on filesystem.";

	return modList;
}

void Memofiles::save()
{
	FUNCTIONSETUP;

	saveCategoryMetadata();
	saveMemos();
	// this needs to be done last, because saveMemos() might change
	// attributes of the Memofiles
	saveMemoMetadata();

}

bool Memofiles::saveMemoMetadata()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << ": saving memo metadata to file: [" << _memoMetadataFile << ']';

	QFile f( _memoMetadataFile );
	QTextStream stream(&f);

	if( !f.open(QIODevice::WriteOnly) ) {
		DEBUGKPILOT << ": ooh, bad.  could not open your memo-id file for writing.";
		return false;
	}

	Memofile * memofile;

	// each line looks like this, but FIELD_SEP is the separator instead of ","
	// id,category,lastModifiedTime,filesize,filename
	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		// don't save deleted memos to our id file
		if (! memofile->isDeleted()) {
			stream  << memofile->id() << FIELD_SEP
				<< memofile->category() << FIELD_SEP
				<< memofile->lastModified() << FIELD_SEP
				<< memofile->size() << FIELD_SEP
				<< memofile->filename()
				<< endl;
		}
	}

	f.close();

	return true;

}

MemoCategoryMap Memofiles::readCategoryMetadata()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << "Reading categories from file [" << _categoryMetadataFile << ']';

	MemoCategoryMap map;
	map.clear();

	QFile f( _categoryMetadataFile );
	QTextStream stream(&f);

	if( !f.open(QIODevice::ReadOnly) ) {
		DEBUGKPILOT << "Could not open your categories file for reading.";
		return map;
	}


	while ( !stream.atEnd() ) {
		QString data = stream.readLine();
		int errors = 0;
		bool ok;

		QStringList fields = data.split( FIELD_SEP, QString::SkipEmptyParts );
		if ( fields.count() >= 2 ) {
			int id = fields[0].toInt( &ok );
			if ( !ok )
				errors++;
			QString categoryName = fields[1];
			if ( categoryName.isEmpty() )
				errors++;

			if (errors <= 0) {
				map[id] = categoryName;
			}
		} else {
			errors++;
		}

		if (errors > 0) {
			DEBUGKPILOT << ": error: could not understand this line: [" << data << ']';
		}
	}

	DEBUGKPILOT << ": loaded: [" << map.count() << "] categories.";

	f.close();

	return map;
}

bool Memofiles::saveCategoryMetadata()
{
	FUNCTIONSETUP;


	DEBUGKPILOT << ": saving categories to file: [" << _categoryMetadataFile << ']';

	QFile f( _categoryMetadataFile );
	QTextStream stream(&f);

	if( !f.open(QIODevice::WriteOnly) ) {
		DEBUGKPILOT << ": ooh, bad.  could not open your categories file for writing.";
		return false;
	}

	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		stream  << it.key()
			<< FIELD_SEP
			<< it.value()
			<< endl;
	}

	f.close();

	return true;
}

bool Memofiles::saveMemos()
{
	FUNCTIONSETUP;

	Memofile * memofile;
	bool result = true;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if (memofile->isDeleted()) {
			_memofiles.remove(memofile);
		} else {
			result = memofile->save();
			// Fix prompted by Bug #103922
			// if we weren't able to save the file, then remove it from the list.
			// if we don't do this, the next sync will think that the user deliberately
			// deleted the memofile and will then delete it from the Pilot.
			// TODO -- at some point, we should probably tell the user that this
			//        did not work, but that will require a String change.
			//        Also, this is a partial fix since at this point
			//        this memo will never make its way onto the PC, but at least
			//        we won't delete it from the Pilot erroneously either.  *sigh*
			if (!result) {
				DEBUGKPILOT
					<< ": unable to save memofile: ["
					<< memofile->filename()
					<< "], now removing it from the metadata list.";
				_memofiles.remove(memofile);
			}
		}
	}
	return true;
}

bool Memofiles::isFirstSync()
{
	FUNCTIONSETUP;
	bool metadataExists = QFile::exists(_memoMetadataFile) &&
	                      QFile::exists(_categoryMetadataFile);

	bool valid = metadataExists && _metadataLoaded;

	DEBUGKPILOT
	<< ": local metadata exists: [" << metadataExists
	<< "], metadata loaded: [" << _metadataLoaded
	<< "], returning: [" << ! valid << ']';
	return ! valid;
}



bool Memofiles::folderRemove(const QDir &_d)
{
	FUNCTIONSETUP;

	QDir d = _d;

	QStringList entries = d.entryList();
	for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		if(*it == CSL1(".") || *it == CSL1(".."))
			continue;
		QFileInfo info(d, *it);
		if(info.isDir()) {
			if(!folderRemove(QDir(info.filePath())))
				return false;
		} else {
			DEBUGKPILOT
			<< ": deleting file: [" << info.filePath() << ']';
			d.remove(info.filePath());
		}
	}
	QString name = d.dirName();
	if(!d.cdUp())
		return false;
	DEBUGKPILOT
	<< ": removing folder: [" << name << "]";
	d.rmdir(name);

	return true;
}

QString Memofiles::filename(PilotMemo * memo)
{
	FUNCTIONSETUP;

	QString filename = memo->getTitle();

	if (filename.isEmpty()) {
		QString text = memo->text();
		int i = text.indexOf('\n');
		if (i > 1) {
			filename = text.left(i);
		}
		if (filename.isEmpty()) {
			filename = CSL1("empty");
		}
	}

	filename = sanitizeName(filename);

	QString category = _categories[memo->category()];

	Memofile * memofile = find(category, filename);

	// if we couldn't find a memofile with this filename, or if the
	// memofile that is found is the same as the memo that we're looking
	// at, then use the filename
	if (NULL == memofile || memofile == memo) {
		return filename;
	}

	int memonameindex = 2;
	QString newfilename;

	// try to find a good filename, but only do this 20 times at the most.
	// if our user has 20 memos with the same filename, he/she is asking
	// for trouble.
	while (NULL != memofile && memonameindex <=20) {
		newfilename = QString(filename + CSL1(".") + QString::number(memonameindex++) );
		memofile = find(category, newfilename);
	}

	return newfilename;
}

QString Memofiles::sanitizeName(QString name)
{
	QString clean = name;
	// safety net. we can't save a
	// filesystem separator as part of a filename, now can we?
	clean.replace('/', CSL1("-"));
	return clean;
}

