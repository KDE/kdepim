/* memofile-conduit.cc			KPilot
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

#include "memofiles.h"
#include "memofile.h"

QString Memofiles::FIELD_SEP = CSL1("\t");

Memofiles::Memofiles (MemoCategoryMap & categories, struct MemoAppInfo & appInfo, QString & baseDirectory) :
		_categories(categories), _memoAppInfo(appInfo), _baseDirectory(baseDirectory)
{
	FUNCTIONSETUP;
	_memofiles.clear();
	_memoMetadataFile = _baseDirectory + QDir::separator() + CSL1(".ids");
	_categoryMetadataFile = _baseDirectory + QDir::separator() + CSL1(".categories");
	_countNewToLocal = _countModifiedToLocal = _countDeletedToLocal = 0;
	_memofiles.setAutoDelete(true);

	_metadataLoaded = loadFromMetadata();
}

Memofiles::~Memofiles()
{
	FUNCTIONSETUP;
}

void Memofiles::load (bool loadAll)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": now looking at all memofiles in your directory." << endl;
#endif

	// now go through each of our known categories and look in each directory
	// for that category for memo files
	MemoCategoryMap::ConstIterator it;
	int counter = -1;

	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		int category = it.key();
		QString categoryName = it.data();
		QString categoryDirname = _baseDirectory + QDir::separator() + categoryName;

		QDir dir = QDir(categoryDirname);
		if (! dir.exists() ) {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": category directory: [" << categoryDirname
			<< "] doesn't exist. skipping." << endl;
#endif
			continue;
		}


		QStringList entries = dir.entryList(QDir::Files);
		QString file;
		for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
			file = *it;
			QFileInfo info(dir, file);

			if(info.isFile() && info.isReadable()) {
// #ifdef DEBUG
// 				DEBUGCONDUIT << fname
// 				<< ": checking category: [" << categoryName
// 				<< "], file: [" << file << "]." << endl;
// #endif
				Memofile * memofile = find(categoryName, file);
				if (NULL == memofile) {
					memofile = new Memofile(category, categoryName, file, _baseDirectory);
					memofile->setModified(true);
					_memofiles.append(memofile);
#ifdef DEBUG
					DEBUGCONDUIT << fname
					<< ": looks like we didn't know about this one until now. "
					<< "created new memofile for category: ["
					<< categoryName << "], file: [" << file << "]." << endl;
#endif

				}

				counter++;

				// okay, we should have a memofile for this file now.  see if we need
				// to load its text...
				if (memofile->isModified() || loadAll) {
#ifdef DEBUG
					DEBUGCONDUIT << fname
					<< ": now loading text for: [" << info.filePath() << "]." << endl;
#endif
					memofile->load();
				}
			} else {
#ifdef DEBUG
				DEBUGCONDUIT << fname
				<< ": couldn't read file: [" << info.filePath() << "]. skipping it." << endl;
#endif

			}
		} // end of iterating through files in this directory

	} // end of iterating through our categories/directories

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": looked at: [" << counter << "] files from your directories." << endl;
#endif


	// okay, now we've loaded everything from our directories.  make one last
	// pass through our loaded memofiles and see if we need to mark any of them
	// as deleted (i.e. we created a memofile object from our metadata, but
	// the file is now gone, so it's deleted.
	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if (! memofile->fileExists()) {
			memofile->makeDeleted();
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

	// now make sure that a directory for each category exists.
	QString _category_name;
	QString dir;

	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		_category_name = it.data();
		dir = _baseDirectory + QDir::separator() + _category_name;

#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": checking directory: [" << dir		<< "]" << endl;
#endif

		if (!checkDirectory(dir))
			return false;
	}

	return true;
}

bool Memofiles::checkDirectory(QString & dir)
{
	FUNCTIONSETUP;
	// make sure that the directory we're asked to write to exists
	QDir d(dir);
	QFileInfo fid( dir );

	if ( ! fid.isDir() ) {

#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": directory: [" << dir
		<< "] doesn't exist. creating...."
		<< endl;
#endif

		if (!d.mkdir(dir)) {

#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": could not create directory: [" << dir
			<< "].  this won't end well." << endl;
#endif
			return false;
		} else {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": directory created: ["
			<< dir << "]." << endl;
#endif

		}
	} else {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": directory already existed: ["
		<< dir << "]." << endl;
#endif

	}

	return true;

}

void Memofiles::eraseLocalMemos ()
{
	FUNCTIONSETUP;

	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		QString dir = _baseDirectory + QDir::separator() + it.data();

		if (!folderRemove(QDir(dir))) {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": couldn't erase all local memos from: ["
			<< dir << "]." << endl;
#endif
		}
	}
	QDir d(_baseDirectory);
	d.remove(_memoMetadataFile);

	ensureDirectoryReady();

	_memofiles.clear();
}

void Memofiles::setPilotMemos (QPtrList<PilotMemo> & memos)
{
	FUNCTIONSETUP;

	PilotMemo * memo;

	_memofiles.clear();

	for ( memo = memos.first(); memo; memo = memos.next() ) {
		addModifiedMemo(memo);
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": set: ["
	<< _memofiles.count() << "] from Palm to local." << endl;
#endif

}

bool Memofiles::loadFromMetadata ()
{
	FUNCTIONSETUP;

	_memofiles.clear();

	QFile f( _memoMetadataFile );
	if ( !f.open( IO_ReadOnly ) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": ooh, bad.  couldn't open your memo-id file for reading."
		<< endl;
#endif
		return false;
	}

	QTextStream t( &f );
	Memofile * memofile;

	while ( !t.atEnd() ) {
		QString data = t.readLine();
		int errors = 0;
		bool ok;

		QStringList fields = QStringList::split( FIELD_SEP, data );
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
				// #ifdef DEBUG
				// 				DEBUGCONDUIT << fname
				// 				<< ": created memofile from metadata. id: [" << id
				// 				<< "], category: ["
				// 				<< _categories[category] << "], filename: [" << filename << "]."
				// 				<< endl;
				// #endif
			}
		} else {
			errors++;
		}

		if (errors > 0) {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": error: couldn't understand this line: [" << data << "]."
			<< endl;
#endif
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": loaded: [" << _memofiles.count() << "] memofiles."
	<< endl;
#endif

	f.close();

	return (_memofiles.count() > 0);
}

Memofile * Memofiles::find (recordid_t id)
{

	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if ( memofile->getID() == id) {
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

	Memofile * memofile = find(memo->getID());
	if (memofile) {
		memofile->deleteFile();
		_memofiles.remove(memofile);
		_countDeletedToLocal++;
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
	                + QString::number(memo->getID()) + CSL1("], title: [")
	                + memo->getTitle() + CSL1("]. ");

	Memofile * memofile = find(memo->getID());

	if (NULL == memofile) {
		_countNewToLocal++;
		debug += CSL1(" new from pilot.");
	} else {
		// we have found a local memofile that was modified on the palm.  for the time
		// being (until someone complains, etc.), we will always overwrite changes to
		// the local filesystem with changes to the palm (palm overrides local).  at
		// some point in the future, we should probably honor a user preference for
		// this...
		_countModifiedToLocal++;
		_memofiles.remove(memofile);
		debug += CSL1(" modified from pilot.");
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< debug << endl;
#endif

	memofile = new Memofile(memo, _categories[memo->getCat()], filename(memo), _baseDirectory);
	memofile->setModifiedByPalm(true);
	_memofiles.append(memofile);

}

QPtrList<Memofile> Memofiles::getModified ()
{
	FUNCTIONSETUP;

	QPtrList<Memofile> modList;
	modList.clear();

	Memofile * memofile;

	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		if ( memofile->isModified() && ! memofile->isModifiedByPalm() ) {
			modList.append(memofile);
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": found: [" << modList.count() << "] memofiles modified on filesystem." << endl;
#endif

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

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": saving memo metadata to file: ["
	<< _memoMetadataFile << "]" << endl;
#endif

	QFile f( _memoMetadataFile );
	QTextStream stream(&f);

	if( !f.open(IO_WriteOnly) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": ooh, bad.  couldn't open your memo-id file for writing."
		<< endl;
#endif
		return false;
	}

	Memofile * memofile;

	// each line looks like this, but FIELD_SEP is the separator instead of ","
	// id,category,lastModifiedTime,filesize,filename
	for ( memofile = _memofiles.first(); memofile; memofile = _memofiles.next() ) {
		// don't save deleted memos to our id file
		if (! memofile->isDeleted()) {
			stream  << memofile->getID() << FIELD_SEP
			<< memofile->getCat() << FIELD_SEP
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

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": reading categories from file: ["
	<< _categoryMetadataFile << "]" << endl;
#endif

	MemoCategoryMap map;
	map.clear();

	QFile f( _categoryMetadataFile );
	QTextStream stream(&f);

	if( !f.open(IO_ReadOnly) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": ooh, bad.  couldn't open your categories file for reading."
		<< endl;
#endif
		return map;
	}


	while ( !stream.atEnd() ) {
		QString data = stream.readLine();
		int errors = 0;
		bool ok;
	
		QStringList fields = QStringList::split( FIELD_SEP, data );
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
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": error: couldn't understand this line: [" << data << "]."
			<< endl;
#endif
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": loaded: [" << map.count() << "] categories."
	<< endl;
#endif

	f.close();

	return map;
}

bool Memofiles::saveCategoryMetadata()
{
	FUNCTIONSETUP;


#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": saving categories to file: ["
	<< _categoryMetadataFile << "]" << endl;
#endif

	QFile f( _categoryMetadataFile );
	QTextStream stream(&f);

	if( !f.open(IO_WriteOnly) ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": ooh, bad.  couldn't open your categories file for writing."
		<< endl;
#endif
		return false;
	}
	
	MemoCategoryMap::Iterator it;
	for ( it = _categories.begin(); it != _categories.end(); ++it ) {
		stream  << it.key()
		<< FIELD_SEP
		<< it.data()
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
	
#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": local metadata exists: [" << metadataExists
	<< "], metadata loaded: [" << _metadataLoaded
	<< "], returning: [" << ! valid << "]" << endl;
#endif
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
				return FALSE;
		} else {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": deleting file: [" << info.filePath() << "]" << endl;
#endif
			d.remove(info.filePath());
		}
	}
	QString name = d.dirName();
	if(!d.cdUp())
		return FALSE;
#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": removing folder: [" << name << "]" << endl;
#endif
	d.rmdir(name);

	return TRUE;
}

QString Memofiles::filename(PilotMemo * memo)
{
	FUNCTIONSETUP;

	QString filename = memo->getTitle();

	if (filename.isEmpty()) {
		QString text = memo->text();
		int i = text.find(CSL1("\n"));
		if (i > 1) {
			filename = text.left(i);
		}
		if (filename.isEmpty()) {
			filename = CSL1("empty");
		}
		// safety net. we can't save a 
		// filesystem separator as part of a filename, now can we?
		filename.replace('/', CSL1("-"));
	}

	QString category = _categories[memo->getCat()];

	Memofile * memofile = find(category, filename);

	// if we couldn't find a memofile with this filename, or if the
	// memofile that is found is the same as the memo that we're looking
	// at, then use the filename
	if (NULL == memofile || memofile == memo) {
		return filename;
	}

	int uniq = 2;
	QString newfilename;

	// try to find a good filename, but only do this 20 times at the most.
	// if our user has 20 memos with the same filename, he/she is asking
	// for trouble.
	while (NULL != memofile && uniq <=20) {
		newfilename = QString(filename + CSL1(".") + QString::number(uniq++) );
		memofile = find(category, newfilename);
	}

	return newfilename;
}

QString Memofiles::getResults()
{
	QString result;

	if (_countNewToLocal > 0)
		result += i18n("%1 new to filesystem. ").arg(_countNewToLocal);

	if (_countModifiedToLocal > 0)
		result += i18n("%1 changed to filesystem. ").arg(_countModifiedToLocal);

	if (_countDeletedToLocal > 0)
		result += i18n("%1 deleted from filesystem. ").arg(_countDeletedToLocal);

	return result;
}
