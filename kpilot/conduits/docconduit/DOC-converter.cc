// doc-converter.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The doc converter synchronizes text files on the PC with DOC databases on the Palm
//




#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <unistd.h>
//
//#include <qtimer.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qsortedlist.h>


#include <pilotDatabase.h>
#include <pilotLocalDatabase.h>
#include <pilotSerialDatabase.h>

#include "pilotDOCHead.h"
#include "pilotDOCEntry.h"
#include "pilotDOCBookmark.h"

#include "DOC-converter.moc"
#include "DOC-converter.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *doc_converter_id = "$Id: $";

#define min(a,b) (a<b)?(a):(b)



/****************************************************************************************************
 *  various bookmark classes. Most important is the bmkList  findMatches(QString) function, 
 *  which needs to return a list of all bookmarks found for the given bookmark expression.
 *  A bookmark usually consists of a bookmark text and an offset into the text document.
 ****************************************************************************************************/


bool docBookmark::compare_pos=true;

bool operator< ( const docBookmark &s1, const docBookmark &s2)
{
	if (docBookmark::compare_pos) { return s1.position<s2.position;}
	else {return s2.bmkName<s2.bmkName;}
};

bool operator== ( const docBookmark &s1, const docBookmark &s2)
{
	return (s1.position==s2.position) && (s1.bmkName==s2.bmkName);
};


int docMatchBookmark::findMatches(QString doctext, bmkList &fBookmarks) {
	FUNCTIONSETUP;
//	bmkList res;
	int pos = 0, nr=0;
#ifdef DEBUG
	DEBUGCONDUIT<<"Finding matches of "<<pattern<<endl;
#endif

	while (pos >= 0) {
		pos = doctext.find(pattern, pos);
#ifdef DEBUG
		DEBUGCONDUIT<<"Result of search: pos="<<pos<<endl;
#endif
		if (pos >= 0)
		{
			fBookmarks.append(new docBookmark(pattern, pos));
			pos++;
			nr++;
		}
	}
	return nr;
};



int docRegExpBookmark::findMatches(QString doctext, bmkList &fBookmarks) {
//	bmkList res;
	QRegExp rx(pattern);
	int pos = 0, nr=0;

	while (pos>=0) 	{
		pos=rx.search(doctext, pos);
		if (pos > -1) {
			fBookmarks.append(new docBookmark(rx.cap(0), pos));
			pos++;
			nr++;
		}
	}
	return nr;
};









/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/


DOCConverter::DOCConverter(QObject *parent, const char *name):QObject(parent,name) {
	FUNCTIONSETUP;
	docdb=0L;
	eSortBookmarks=eSortNone;
	fBookmarks.setAutoDelete( TRUE );
	(void) doc_converter_id;
}



DOCConverter::~DOCConverter() {
	FUNCTIONSETUP;
}





/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/



void DOCConverter::setDOCpath(QString path, QString file) {
	QDir dr(path);
	QFileInfo pth(dr, file);
	if (!file.isEmpty())
		 docfilename = pth.absFilePath();
};



void DOCConverter::setDOCpath(QString filename) {
	if (!filename.isEmpty()) docfilename = filename;
};



void DOCConverter::setPDB(PilotDatabase * dbi) {
	if (dbi) docdb = dbi;
};

 
QString DOCConverter::readText() {
	FUNCTIONSETUP;
	if (docfilename.isEmpty()) return QString();
	QFile docfile(docfilename);
	if (!docfile.open(IO_ReadOnly))
	{
		emit logError(i18n("Unable to open text file %1 for reading.").arg(docfilename));
		return QString();
	}

	QTextStream docstream(&docfile);

	QString doc = docstream.read();
	docfile.close();
	return doc;
}

int DOCConverter::findBmkEndtags(QString &text, bmkList&fBmks) const {
	FUNCTIONSETUP;
	// Start from the end of the text
	int pos = text.length() - 1, nr=0;
	bool doSearch=true;
	while (pos >= 0/* && doSearch*/) {
#ifdef DEBUG
			DEBUGCONDUIT<<"Current character is \'"<<text[pos].latin1()<<"\'"<<endl;
#endif
		// skip whitespace until we reach a >
		while (text[pos].isSpace() && pos >= 0) {
#ifdef DEBUG
			DEBUGCONDUIT<<"Skipping whitespaces at the end of the file"<<endl;
#endif
			pos--;
		}
		// every other character than a > is assumed to belong to the text, so there are no more bookmarks.
		// TODO: Does this break only jump out of the if or the whole while (which is what I actually want)
		if (pos < 0 || text[pos] != '>') {
#ifdef DEBUG
			DEBUGCONDUIT<<"Current character \'"<<text[pos].latin1()<<"\' at position "<<pos<<" is not and ending >. Finish searching for bookmarks."<<endl;
#endif
		
			pos=-1;
			break;
		} else {
			int endpos = pos;
			doSearch=true;
#ifdef DEBUG
			DEBUGCONDUIT<<"Found the ending >, now looking for the opening <"<<endl;
#endif
			
			
			// Search for the opening <. There must not be a newline in the bookmark text.
			while (doSearch && pos > 0) {
#ifdef DEBUG
			DEBUGCONDUIT<<"pos="<<pos<<", char="<<text[pos].latin1()<<endl;
#endif
				pos--;
				if (text[pos] == '\n') {
#ifdef DEBUG
			DEBUGCONDUIT<<"Found carriage return at position "<<pos<<" inside the bookmark text, assuming this is not a bookmark, and the text ends in a >"<<endl;
#endif
					doSearch = false;
					pos = -1;
					break;
				}
				if (text[pos] == '<') {
					fBmks.append(new docMatchBookmark(text.mid(pos + 1, endpos - pos - 1)));
					nr++;
#ifdef DEBUG
			DEBUGCONDUIT<<"Found opening < at position "<<pos<<", bookmarktext ="<<text.mid(pos+1, endpos-pos-1)<<endl;
#endif
					text.remove(pos, text.length());
					pos--;
					doSearch = false;
				}
			}
		}
#ifdef DEBUG
		DEBUGCONDUIT<<"Finished processing the next bookmark, current position: "<<pos<<endl;
#endif
		
	}
	return nr;
}
		
int DOCConverter::findBmkInline(QString &text, bmkList &fBmks) const {
	FUNCTIONSETUP;
//	bmkList res;
	int nr=0;
	QRegExp rx("<\\*(.*)\\*>");

	rx.setMinimal(TRUE);
	int pos = 0;
	while (pos >= 0) {
		pos = rx.search(text, pos);
		if (pos >= 0) {
			fBmks.append(new docBookmark(rx.cap(1), pos+1));
			nr++;
			text = text.remove(pos, rx.matchedLength());
		}
	}
	return nr;
}

int DOCConverter::findBmkFile(QString &text, bmkList &fBmks) const {
	FUNCTIONSETUP;
//	bmkList res;
	int nr=0;
	// TODO: Read in regexps from a file...
	text.length();
	fBmks.count();
	return nr;
}

bool DOCConverter::convertDOCtoPDB() {
	FUNCTIONSETUP;
	
	if (!docdb) {
		emit logError(i18n("Unable to open Database for writing"));
		return false;
	}
	
	QString text = readText();

// #ifdef DEBUG
// 	DEBUGCONDUIT << endl << endl << endl <<
// 		"*****************************************************************"
// 		<< endl << endl << endl
// 		<< text
// 		<< endl << endl << endl <<
// 		"*****************************************************************"
// 		<< endl << endl << endl;
// #endif

	if (fBmkTypes && eBmkEndtags) {
		findBmkEndtags(text, fBookmarks);
	}	// end: EndTag Bookmarks

	
	// Search for all tags <* Bookmark text *> in the text. We have to delete them immediately, otherwise the later bookmarks will be off.
	if (fBmkTypes && eBmkInline) {
		findBmkInline(text, fBookmarks);
	}	 // end: Inline Bookmarks


	// Read in regular expressions and positions from an external file (doc-filename with extension .bmk)
	if (fBmkTypes & eBmkFile)
	{
		findBmkFile(text, fBookmarks);
	}

	// Process the bookmarks: find the occurences of the regexps, and sort them if requested:
	bmkSortedList pdbBookmarks;
	pdbBookmarks.setAutoDelete(TRUE);
	docBookmark*bmk;
	for (bmk = fBookmarks.first(); bmk; bmk = fBookmarks.next())
	{
		bmk->findMatches(text, pdbBookmarks);
	}
	
	switch (eSortBookmarks)
	{
		case eSortName:
			docBookmark::compare_pos=false;
//			qHeapSort(pdbBookmarks);
			pdbBookmarks.sort();
			break;
		case eSortPos:
			docBookmark::compare_pos=true;
			pdbBookmarks.sort();
			break;
		case eSortNone:
		default:
			break;
	}

#ifdef DEBUG
	DEBUGCONDUIT << "Bookmarks: "<<endl;
	for (bmk = pdbBookmarks.first(); bmk; bmk = pdbBookmarks.next())
	{
		DEBUGCONDUIT<<bmk->bmkName<<" at position "<<bmk->position<<endl;
	}
#endif
	
	if (!docdb->isDBOpen()) {
		emit logError(i18n("Unable to open palm doc database %1").arg(docdb->dbPathName()) );
		return false;
	}
	
	// Clean the whole database, otherwise the records would be just appended!
	docdb->deleteRecord(0, true);

	// Header record for the doc file format
	PilotDOCHead docHead;
	docHead.position=0;
	docHead.recordSize=4096;
	docHead.spare=0;
	docHead.storyLen=text.length();
	docHead.version=compress?2:1;
	docHead.numRecords=(int)( (text.length()-1)/docHead.recordSize)+1;
	PilotRecord*rec=docHead.pack();
	docdb->writeRecord(rec);
	KPILOT_DELETE(rec);
#ifdef DEBUG
	DEBUGCONDUIT << "Write header record: length="<<text.length()<<", compress="<<compress<<endl;
#endif

	// First compress the text, then write out the bookmarks and - if existing - also the annotations
	int len=text.length();
	int start=0,reclen=0;
	int recnum=0;
	while (start<len)
	{
		reclen=min(len-start, PilotDOCEntry::TEXT_SIZE);
#ifdef DEBUG
	DEBUGCONDUIT << "Record #"<<recnum<<", reclen="<<reclen<<", compress="<<compress<<endl;
#endif
		PilotDOCEntry recText;
//		recText.setText(text.mid(start, reclen), reclen);
		recText.setText(text.mid(start, reclen));
//		if (compress) 
		recText.setCompress(compress);
		PilotRecord*textRec=recText.pack();
		docdb->writeRecord(textRec);
		recnum++;
		start+=reclen;
		KPILOT_DELETE(textRec);
	}
	
	recnum=0;
	// Finally, write out the bookmarks
	for (bmk = pdbBookmarks.first(); bmk; bmk = pdbBookmarks.next())
//	for (bmkList::const_iterator it=pdbBookmarks.begin(); it!=pdbBookmarks.end(); it++)
	{
		recnum++;
#ifdef DEBUG
		DEBUGCONDUIT << "Bookmark #"<<recnum<<", Name="<<bmk->bmkName.latin1()<<", Position="<<bmk->position<<endl;
#endif
		PilotDOCBookmark bmkEntry;
		bmkEntry.pos=bmk->position;
		strncpy(&bmkEntry.bookmarkName[0], bmk->bmkName.latin1(), 16);
		PilotRecord*bmkRecord=bmkEntry.pack();
		docdb->writeRecord(bmkRecord);
		KPILOT_DELETE(bmkRecord);
	}

	pdbBookmarks.clear();
	fBookmarks.clear();

	return true;
}



bool DOCConverter::convertPDBtoDOC()
{
	FUNCTIONSETUP;
	if (docfilename.isEmpty()) {
		emit logError(i18n("No filename set for the conversion"));
		return false;
	}
	
	if (!docdb) {
		emit logError(i18n("Unable to open Database for reading"));
		return false;
	}

	// The first record of the db is the document header containing information about the doc db
	PilotRecord*headerRec = docdb->readRecordByIndex(0);
	if (!headerRec)
	{
		emit logError(i18n("Unable to read database header for database %1.").arg(docdb->dbPathName()));
		KPILOT_DELETE(docdb);
		return false;
	}
	PilotDOCHead header(headerRec);
	KPILOT_DELETE(headerRec);
#ifdef DEBUG
	DEBUGCONDUIT<<"Database "<<docdb->dbPathName()<<" has "<<header.numRecords<<" text records, "<<endl
		<<" total number of records: "<<docdb->recordCount()<<endl
		<<" position="<<header.position<<endl
		<<" recordSize="<<header.recordSize<<endl
		<<" spare="<<header.spare<<endl
		<<" storyLen="<<header.storyLen<<endl
//		<<" textRecordSize="<<header.textRecordSize<<endl
		<<" version="<<header.version<<endl;
#endif

	// next come the header.numRecords real document records (might be compressed, see the version flag in the header)
	QFile docfile(docfilename);
	if (!docfile.open(IO_WriteOnly))
	{
		emit logError(i18n("Unable to open output file %1.").arg(docfilename));
		KPILOT_DELETE(docdb);
		return false;
	}
	QTextStream docstream(&docfile);
	for (int i=1; i<header.numRecords+1; i++)
	{
		PilotRecord*rec=docdb->readRecordByIndex(i);
		if (rec)
		{
				// TODO: use a symbolic tag instead of the 2 for compressed
			PilotDOCEntry recText(rec, header.version==2);
			docstream<<recText.getText();
#ifdef DEBUG
			DEBUGCONDUIT<<"Record "<<i<<endl;
#endif
			KPILOT_DELETE(rec);
		} else {
			emit logMessage(i18n("Could not read text record #%1 from Database %2").arg(i).arg(docdb->dbPathName()));
		}
	}

	// After the document records possibly come a few bookmark records, so read them in and put them in a separate bookmark file.
	// TODO: for the ztxt conduit there might be annotations after the bookmarks, so the upper bound needs to be adapted.
	int upperBmkRec=docdb->recordCount();
	QString bmkfilename = docfile.name();
	if (bmkfilename.endsWith(".txt")){
		bmkfilename.remove(bmkfilename.length()-4, 4);
	}
	bmkfilename+=".bmk";
	QFile bmkfile(bmkfilename);
	if (!bmkfile.open(IO_WriteOnly))
	{
		emit logError(i18n("Unable to open output file %1 for the bookmarks of %2.").arg(bmkfilename).arg(docdb ->dbPathName()));
	}
	else
	{
#ifdef DEBUG
	DEBUGCONDUIT<<"Writing "<<upperBmkRec-header.numRecords<<"("<<upperBmkRec<<") bookmarks to file "<<bmkfilename<<endl;
#endif
		QTextStream bmkstream(&bmkfile);
		for (int i=header.numRecords+1; i<upperBmkRec; i++)
		{
			PilotRecord*rec=docdb->readRecordByIndex(i);
			if (rec)
			{
				PilotDOCBookmark bmk(rec);
				bmkstream<<bmk.pos<<", "<<bmk.bookmarkName<<endl;
				KPILOT_DELETE(rec);
			} else {
				emit logMessage(i18n("Could not read bookmark record #%1 from Database %2").arg(i).arg(docdb->dbPathName()));
			}
		}
//		bmkstream.close();
		bmkfile.close();
	}

//	docstream.close();
	docfile.close();
	docdb->cleanup();
	// reset all records to unchanged. I don't know if this is really such a wise idea?
	docdb->resetSyncFlags();
	return true;
}




// $Log$
