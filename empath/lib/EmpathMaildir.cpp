/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMaildir.h"
#endif

// System includes
#include <sys/file.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

// Qt includes
#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qstringlist.h>

// KDE includes
#include <kapp.h>
#include <klocale.h>

// Local includes
#include "EmpathMaildir.h"
#include "EmpathFolderList.h"
#include "EmpathMessageList.h"
#include "Empath.h"
#include "EmpathTask.h"
#include "EmpathMailbox.h"

EmpathMaildir::EmpathMaildir(const QString & basePath, const EmpathURL & url)
	:	seq_(0),
		url_(url),
		basePath_(basePath)
{
	path_ = basePath + "/" + url.folderPath();
	empathDebug("ctor - path_ == " + path_);
}

EmpathMaildir::~EmpathMaildir()
{
	empathDebug("dtor");
}

	void
EmpathMaildir::sync(const EmpathURL & url, bool ignoreMtime)
{
	empathDebug("sync(" + url.asString() + ") called");
	QTime realBegin(QTime::currentTime());
	
	// First make sure any new mail that has arrived goes into cur.
	_markNewMailAsSeen();
	
	// Get a pointer to the folder related to us.
	EmpathFolder * f(empath->folder(url));
	
	if (f == 0) {
		empathDebug("sync: Couldn't find folder !");
		return;
	}
	
	if (f->messageList().count() == 0) {
		_readIndex();
	}

	if (!ignoreMtime) {
	
		// Check the mtime of cur. If it's not changed, we can return now.
		QFileInfo fiDir(path_ + "/cur/");
		QFileInfo fiIndex(path_ + "/.empathIndex");
		
		if (fiDir.lastModified() < fiIndex.lastModified()) {
			empathDebug("sync: Not modified");
			return;
		}
	}

	EmpathTask * t(empath->addTask(i18n("Reading mailbox")));
	CHECK_PTR(t);

	// Get the dir listing.
	QDir d(path_ + "/cur/",
		QString::null, QDir::Name | QDir::IgnoreCase,
		QDir::NoSymLinks | QDir::Files);

	QStringList fileList(d.entryList());
	empathDebug("There are " +
		QString().setNum(fileList.count()) + " files in cur");
	
	// Look through the file list.
	// 
	// Any files that we don't recognise must be parsed and added to the
	// folder's message list.
	// 
	// Any files that have disappeared (are in the message list but not in the
	// dir entry list) must be removed from the message list.
	
	QStringList::ConstIterator it(fileList.begin());
	
	t->setMax(fileList.count());
	
	QString s;
	QRegExp re_flags(":2,[A-Za-z]*$");
	
	QTime begin(QTime::currentTime());
	
	for (; it != fileList.end(); ++it) {
		
		QTime now(QTime::currentTime());
		
		if (begin.msecsTo(now) > 100) {
			kapp->processEvents();
			begin = now;
		}
		
		s = *it;
	
		RMM::MessageStatus status(RMM::MessageStatus(0));
		
		int i = s.find(re_flags);
		QString flags;
		
		if (i != -1) {
			
			flags = s.right(s.length() - i - 3);
			
			empathDebug("FLAGS: " + flags);
			
			status = (RMM::MessageStatus)
				(	(flags.contains('S') ? RMM::Read	: 0)	|
					(flags.contains('R') ? RMM::Replied	: 0)	|
					(flags.contains('F') ? RMM::Marked	: 0));
		}
		
		// Remove the flags from the end of the filename.
		s.replace(re_flags, QString::null);
		
		EmpathIndexRecord * rec = f->messageList()[s];
		
		// If the record exists, we tag it to say it's still here. Later, we'll
		// look for all untagged records. In this way, we'll be able to
		// efficiently find all records that no longer have a matching file,
		// and therefore which have disappeared.
		
		if (rec) {
		
			rec->tag(true);
			rec->setStatus(status);
		
		} else {	
			// New file to add.
			
			// Read the file to an RMessage
			RMessage m(_messageData(s));
			
			// Create an index record.
			EmpathIndexRecord * ir = new EmpathIndexRecord(s, m);
			CHECK_PTR(ir);
			
			ir->tag(true);
			ir->setStatus(status);
			
			f->messageList().insert(s, ir);
			
			empathDebug("New message in index with id \"" + s + "\"");
		}
		
		t->doneOne();
	}
	
//	kapp->processEvents();
	
	// Now go through the index, looking for untagged records.
	// Anything that wasn't tagged is no longer in the dir, and has been
	// deleted or moved. We therefore remove it from the index.
	
	EmpathIndexIterator iit(f->messageList());
	
	for (; iit.current(); ++iit)
		if (!iit.current()->isTagged()) {
			
			empathDebug("Message with id \"" +
				iit.currentKey() + "\" has gone.");
			
			if (!f->messageList().remove(iit.currentKey())) {
				empathDebug("Duh.. couldn't remove index item \"" +
					iit.currentKey() + "\"");
			}
		} else {
			empathDebug("Message with id \"" +
				iit.currentKey() + "\" seems to still exist");
		}
	
	
	t->done();
	
	_writeIndex();
	empathDebug("sync took " +
		QString().setNum(realBegin.msecsTo(QTime::currentTime())) + " ms");
	empathDebug("sync done");
}

	bool
EmpathMaildir::mark(const EmpathURL & message, RMM::MessageStatus msgStat)
{
	empathDebug("mark(" + message.asString() + ") called");
	
	QRegExp re_flags(":2,[A-Za-z]*$");
	QDir d(path_ + "/cur/", message.messageID() + "*");
	
	if (d.count() != 1) {
		empathDebug("Couldn't mark message");
		return false;
	}
	
	QString filename(d[0]);
	
	QString statusFlags;
	statusFlags +=	(msgStat & RMM::Read	? "S" : "");
	statusFlags	+=	(msgStat & RMM::Marked	? "F" : "");
	statusFlags	+=	(msgStat & RMM::Trashed	? "T" : "");
	statusFlags	+=	(msgStat & RMM::Replied	? "R" : "");
	
	QString newFilename(filename);
	
	if (!newFilename.contains(":2,"))
		newFilename += ":2," + statusFlags;
	else
		newFilename.replace(QRegExp(":2,.*"), ":2," + statusFlags);
	
	bool retval =
		d.rename(path_ + "/cur/" + filename, path_ + "/cur/" + newFilename);
	
	if (!retval) {
		empathDebug("Couldn't mark message");
	}
	return retval;
}

	QString
EmpathMaildir::writeMessage(RMessage & m)
{
	empathDebug("writeMessage called");
	return _write(m);
}

	Q_UINT32
EmpathMaildir::sizeOfMessage(const QString & id)
{
	QDir d(path_ + "/cur/", id + "*");
	if (d.count() != 1) return 0;
	QFileInfo fi(path_ + "/cur/" + d[0]);	
	return fi.size();
}

	QString
EmpathMaildir::plainBodyOfMessage(const QString &)
{
	return QString::null;
}

	REnvelope *
EmpathMaildir::envelopeOfMessage(const QString &)
{
	REnvelope * e = 0;
	return e;
}

	RBodyPart::PartType
EmpathMaildir::typeOfMessage(const QString &)
{
	return RBodyPart::Basic;
}

	RMessage *
EmpathMaildir::message(const QString & id)
{
	QCString s = _messageData(id);
	
	if (s.isEmpty()) {
		empathDebug("Couldn't load data for \"" + id + "\"");
		return 0;
	}
	
	RMessage * m = new RMessage(s);
	CHECK_PTR(m);
	return m;
}

	bool
EmpathMaildir::removeMessage(const QString & id)
{
	empathDebug("Removing message with id \"" + id + "\"");
	QDir d(path_ + "/cur/", id + "*");
	if (d.count() != 1) return false;
	QFile f(path_ + "/cur/" + d[0]);	
	return f.remove();
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

	QCString
EmpathMaildir::_messageData(const QString & filename)
{
	empathDebug("_messageData (" + filename + ") called"); 
	
	if (filename.length() == 0) {
		empathDebug("WARNING: Must supply filename !");
		return "";
	}

	// Now we need to locate the actual file, by looking for the basename with the
	// flags section appended.
	
	QDir cur(path_ + "/cur/", filename + "*");
	
	if (cur.count() != 1) {
		empathDebug("There appears to be no file with the base name \"" +
				filename + "\"");
		empathDebug("Can't exactly match the filename, giving up.");
		return "";
	}
	
	QFile f(path_ + "/cur/" + cur[0]);	

	if (!f.open(IO_ReadOnly)) {
		empathDebug("Couldn't open mail file " + cur[0] + " for reading.");
		return "";
	}

	Q_UINT32 buflen = 32768;			// 1k block should be OK.
	char * buf = new char[buflen];
	QCString messageBuffer;
	
	while (!f.atEnd()) { // && (f.status() & IO_Ok)) {
		
		// On advice of Trolls, just in case I'm writing to this file
		// at the same time, I flush the file's buffers.
		f.flush();
		
		int bytesRead = f.readBlock(buf, buflen);

		if (bytesRead == -1) {
			empathDebug("A serious error occurred while reading the file.");
			delete [] buf;
			buf = 0;
			f.close();
			return "";
		}
		
		messageBuffer += QCString(buf).left(bytesRead);
	}

	delete [] buf;
	buf = 0;
	f.close();
	return messageBuffer;
}

	void
EmpathMaildir::init()
{
	empathDebug("init() called");
	
	d.setPath(path_);

	if (!d.exists())
		if (!d.mkdir(path_)) {
			empathDebug("Couldn't create " + path_);
		}

	// Go to our dir.
	if (!d.cd(path_)) {
		empathDebug("Can't cd to " + path_);
	}
	
	// Make sure our directory structure is sane.
	_setupDirs();
	
	kapp->processEvents();
	
	// Clear out any temporary files. (see maildir(5))
	_clearTmp();
	
	// Last op may have taken some time.
	kapp->processEvents();
	
	// Mark all new mail as seen by this client. (see maildir(5))
	_markNewMailAsSeen();
	
	// Last op may have taken some time.
	kapp->processEvents();
	
	// Quickly read the index in.
	//_readIndex();
	
	// Tell the index to sync up with what's stored.
	sync(url_);
}
	void
EmpathMaildir::_markNewMailAsSeen()
{
	empathDebug("markNewMailsAsSeen() called");
	
	// Get all mails in 'new' and move them to 'cur' - thus
	// marking them as seen by this MUA.

	empathDebug("Moving all mails in 'new' to 'cur'");
	
	QDir dn(path_ + "/new");
	dn.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Writable);
	
	QStringList l = dn.entryList();
	empathDebug("There are " +
		QString().setNum(l.count()) + " messages to rename");
	
	QValueListConstIterator<QString> it = l.begin();

	for (; it != l.end(); ++it)
		_markAsSeen(*it);
}

	void
EmpathMaildir::_markAsSeen(const QString & name)
{
	QString oldName =
		path_ + "/new/" + name;

	QString newName =
		path_ + "/cur/" + name + ":2,";
			
	empathDebug("Marking \"" + oldName + "\" as seen by MUA");

	// FIXME Use Qt to move the file
	if (::rename(oldName.ascii(), newName.ascii()) != 0) {
		empathDebug(
				"Couldn't rename \"" + oldName +
				"\" to \"" + newName + "\"");
		perror("Ouch");
	}
}

	void
EmpathMaildir::_clearTmp()
{
	// Clear out all files in tmp.
	// Hopefully this is done in a pretty secure fashion.. don't delete
	// anything that's not a file (that means leave symlinks alone too).
	//
	// From maildir(5):
	// 'The reader is also expected to look through the tmp directory and
	// to clean up any old files found there. A file in tmp may be safely
	// removed if it has not been accessed in 36 hours.'
	
	empathDebug("_clearTmp() called");
	
	QDate now = QDate::currentDate();
	QDateTime aTime; // Used for each file's atime.
	
	QDir d(path_ + "/tmp/");
	
	if (d.count() == 0) {
		empathDebug("Can't clear tmp !");
		return;
	}
	
	d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Writable);
	
	QFileInfoListIterator it(*(d.entryInfoList()));
	
	for (; it.current(); ++it) {
		
		aTime = it.current()->lastRead(); // atime of file
		
		// use 48 hours as it's easier and no less safe.
		if (aTime.daysTo(now) < 2) {
			
			empathDebug(
					"Deleting \"" +
					QString(it.current()->filePath()) + "\"");

			d.remove(it.current()->filePath(), false);
		}
	}
}

	bool
EmpathMaildir::_setupDirs()
{
	QDir d(path_);
	
	if (!d.exists() && !d.mkdir(path_)) {
		empathDebug("Couldn't create " + path_);
		return false;
	}
	
	if (!d.exists(path_ + "/cur") && !d.mkdir(path_ + "/cur")) {
		empathDebug("Couldn't create " + path_ + "/cur");
		return false;
	}
	
	if (!d.exists(path_ + "/new") && !d.mkdir(path_ + "/new")) {
		empathDebug("Couldn't create " + path_ + "/new");
		return false;
	}
	
	if (!d.exists(path_ + "/tmp") && !d.mkdir(path_ + "/tmp")) {
		empathDebug("Couldn't create " + path_ + "/tmp");
		return false;
	}
	
	return true;
}

	QString
EmpathMaildir::_write(RMessage & msg)
{
	empathDebug("_write() called");

	// Here's the procedure for writing a new mail to the user's Maildir.
	// This procedure follows the advice of D. J. Bernstein, in the
	// documentation provided with qmail.
	// 
	// * Generate a unique filename for this file.
	// 
	// * Get the full path for the filename.
	// 
	// * Stat the (full) filename. We need to get ENOENT to be sure the filename
	//   is available for use.
	// 
	// * Do a last check to see if the filename has become available.
	// 
	// * If it's still not available, give up.
	// 
	// * Dump the message to the file.
	// 
	// * Flush all output to the file.
	// 
	// * Try to close the file.
	// 
	// * Try to link file to the 'new' dir.
	// 
	// At this point, if we didn't hit trouble, the message is DELIVERED.
	// 
	// Mark message as read by MUA by moving to cur.
	// Hand back the filename.
	

	// Generate a unique filename for this file.
	QString canonName = _generateUnique();

	// Generate the flags for this filename
	QString flags = _generateFlagsString(msg.status());
	
	// Get the full path for the filename.
	QString path = path_ + "/tmp/" + canonName;
	
	// This is the file we'll be writing to.
	QFile f(path);

	// Stat the filename. If we don't get ENOENT (error - no such entry)
	// then that filename's already taken. There is no good reason why
	// this should be so.
	// TODO: Perhaps we could 'fix' this so that if we can't get a unique
	// filename, we simply increment a number (our process ID, or the
	// date, or our sequence number) and try again ?
	// 
	empathDebug("stat()ing " + path);
	
	if (f.exists()) {
		
		empathDebug("Couldn't get a free filename for writing mail file - timing out for 2 seconds.");

		// D. J. Bernstein explains that in this particular circumstance,
		// we should sleep for a couple of seconds and try again. I sleep
		// using usleep, a few times, instead. This allows me to process
		// any app events without stalling the interface.

		for (int i = 0 ; i < 20 ; i++) {
			usleep(100);
			kapp->processEvents();
		}
	
		// If it's still there. Give up.
		// FIXME: We need to tell the user that we couldn't write their file.

		if (f.exists())
			return QString::null;
	}

	if (!f.open(IO_WriteOnly)) {
		empathDebug("Couldn't open mail file in ~/Maildir/tmp/ for writing. This is odd, since we could stat it as being ENOENT - must be permissions problem");
		return QString::null;
	}

	empathDebug(
			"Opened output file \"" + path + "\" OK");
	
	// This DataStream will be used to write the data to, which will
	// hit the file later.
	QDataStream outputStream(&f);
	
	// Dump the whole message at once to the stream which is attached
	// to the file.
	empathDebug("Writing message to output stream");
	outputStream.writeRawBytes(msg.asString(), msg.asString().length());

	// Flush all output to the file.
	empathDebug("Flushing output stream");
	f.flush();

	// If flush() failed, give up.
	if (f.status() != IO_Ok) {
		empathDebug("Couldn't flush() file");
		f.close();
		f.remove();
		return QString::null;
	}

	// Try to close the file. Should close OK since we fsync()ed it,
	// but this might be over NFS, which could go down any second.
	
	f.close();

	// If close() failed, give up.
	if (f.status() != IO_Ok) {
		empathDebug("Couldn't close() file");
		f.close();
		f.remove();
		return QString::null;
	}

	empathDebug("Trying to link \"" + path +
		"\" to \"" + path_ + "/new/" + canonName);
	
	// Try to link file to the 'new' dir.
	
	// FIXME Does Qt link yet ?
	
	QString linkTarget(path_ + "/new/" + canonName);
	
	empathDebug(path.ascii());
	empathDebug(linkTarget.ascii());
	
	if (::link(path.ascii(), linkTarget.ascii()) != 0) {
		empathDebug("Couldn't successfully link mail file - giving up");
		perror("link");
		f.close();
		f.remove();
		return QString::null;
	}
	
	// Got here = DELIVERED ! Wow.
	
	empathDebug("Delivery completed :)");
	
	_markAsSeen(canonName);
	
	empathDebug("Finished writing new mail. Delivery and marking as seen successful.");

	// Hand back the filename. The path to the file is not relevant.
	return canonName;
}

	QString
EmpathMaildir::_generateUnique()
{
	QString unique;

	unique = QString().setNum(empath->startTime());
	unique += '.';
	unique += QString().setNum(empath->processID());
	unique += '_';
	unique += QString().setNum(seq_);
	unique += '.';
	unique += empath->hostName();

	++seq_;

	return unique;
}

	QString
EmpathMaildir::_generateFlagsString(RMM::MessageStatus s)
{
	QString flags;
	
	flags += s & RMM::Read		? "S" : "";
	flags += s & RMM::Marked	? "F" : "";
	flags += s & RMM::Trashed	? "T" : "";
	flags += s & RMM::Replied	? "R" : "";
	
	return flags;
}
	
	void
EmpathMaildir::_readIndex()
{
	empathDebug("_readIndex() called");
	QTime realBegin(QTime::currentTime());
	QTime begin(realBegin);
	QTime now;

	QFile indexFile(path_ + "/.empathIndex");
	
	if (!indexFile.open(IO_ReadOnly)) {
		empathDebug("Couldn't read index file \"" + indexFile.name() + "\"");
		return;
	}
	
	QFileInfo fi(indexFile);
	mtime_ = fi.lastModified();
		
	// Get a pointer to the folder related to us.
	EmpathFolder * f(empath->folder(url_));
	
	if (f == 0) {
		empathDebug("_readIndex: Couldn't find folder !");
		return;
	}
	
	QRegExp re_flags(":2,[A-Za-z]*$");

	QDataStream indexStream(&indexFile);
	
	EmpathIndexRecord * r;
	
	while (!indexStream.eof()) {
		
		now = QTime::currentTime();
		if (begin.msecsTo(now) > 100) {
			begin = now;
			kapp->processEvents();
		}

		r = new EmpathIndexRecord;
		CHECK_PTR(r);
		indexStream >> *r;
#if 0	
		int flagsStart = r->id().find(re_flags);

		if (!flagsStart) flagsStart = r->id().length();

		QString flags = r->id().right(r->id().length() - flagsStart + 3);
	
		r->setStatus(
			(RMM::MessageStatus)
			(	(flags.contains('S') ? RMM::Read	: 0)	|
				(flags.contains('R') ? RMM::Replied	: 0)	|
				(flags.contains('F') ? RMM::Marked	: 0)	));
#endif
		empathDebug("Inserting the new index record.");
		
		if (r->id().isEmpty()) {
			empathDebug("Not really - no filename !");
			delete r;
			continue;
		}

		f->messageList().insert(r->id(), r);
	}

	indexFile.close();
	
	empathDebug("readIndex took " +
		QString().setNum(realBegin.msecsTo(QTime::currentTime())) + " ms");
}

	void
EmpathMaildir::_writeIndex()
{
	empathDebug("_writeIndex() called");

	// Get a pointer to the folder related to us.
	EmpathFolder * f(empath->folder(url_));
	
	if (f == 0) {
		empathDebug("writeIndex: Couldn't find folder !");
		return;
	}
	
	QFile indexFile(path_  + "/.empathIndex");
	
	if (!indexFile.open(IO_WriteOnly)) {
		empathDebug("Couldn't write to index \"" + indexFile.name() + "\"");
		return;
	}
	
	QDataStream indexStream(&indexFile);
	
	empathDebug("Writing " +
		QString().setNum(f->messageList().count()) + " entries");

	EmpathIndexIterator it(f->messageList());
	
	for (; it.current(); ++it) {
		
		indexStream << *it.current();
	}
	
	indexFile.close();
	
	mtime_ = QDateTime::currentDateTime();
}

