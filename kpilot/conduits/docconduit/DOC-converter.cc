/* KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** The doc converter synchronizes text files on the PC with DOC databases on the Palm
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"
#include "DOC-converter.moc"

#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qsortedlist.h>

#include <pilotDatabase.h>
#include <pilotLocalDatabase.h>
#include <pilotSerialDatabase.h>

#include "pilotDOCHead.h"
#include "pilotDOCEntry.h"
#include "pilotDOCBookmark.h"



// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *doc_converter_id = "$Id$";

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
}

bool operator== ( const docBookmark &s1, const docBookmark &s2)
{
	return (s1.position==s2.position) && (s1.bmkName==s2.bmkName);
}


int docMatchBookmark::findMatches(QString doctext, bmkList &fBookmarks) {
	FUNCTIONSETUP;
//	bmkList res;
	int pos = 0, nr=0, found=0;
	DEBUGCONDUIT<<"Finding matches of "<<pattern<<endl;

	while (pos >= 0 && found<to) {
		pos = doctext.find(pattern, pos);
		DEBUGCONDUIT<<"Result of search: pos="<<pos<<endl;
		if (pos >= 0)
		{
			++found;
			if (found>=from && found<=to) {
				fBookmarks.append(new docBookmark(pattern, pos));
				++nr;
				
			}
			++pos;
		}
	}
	return nr;
}



int docRegExpBookmark::findMatches(QString doctext, bmkList &fBookmarks) {
//	bmkList res;
	QRegExp rx(pattern);
	int pos = 0, nr=0, found=0;

	while (pos>=0 && found<=to) {
		DEBUGCONDUIT<<"Searching for bookmark "<<pattern<<endl;
		pos=rx.search(doctext, pos);
		if (pos > -1) {
			++found;
			if (found>=from && found<to) {
				if (capSubexpression>=0) {
					fBookmarks.append(new docBookmark(/*bmkName.left(16)*/rx.cap(capSubexpression), pos));
				} else {
					// TODO: use the subexpressions from the regexp for the bmk name ($1..$9) (given as separate regexp)
					QString bmkText(bmkName);
					for (int i=0; i<=rx.numCaptures(); ++i) {
						bmkText.replace(QString("$%1").arg(i), rx.cap(i));
						bmkText.replace(QString("\\%1").arg(i), rx.cap(i));
					}
					fBookmarks.append(new docBookmark(bmkText.left(16), pos));
				}
				++nr;
			}
			++pos;
		}
	}
	return nr;
}








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



void DOCConverter::setTXTpath(QString path, QString file) {
	QDir dr(path);
	QFileInfo pth(dr, file);
	if (!file.isEmpty())
		 txtfilename = pth.absFilePath();
}



void DOCConverter::setTXTpath(QString filename) {
	if (!filename.isEmpty()) txtfilename = filename;
}



void DOCConverter::setPDB(PilotDatabase * dbi) {
	if (dbi) docdb = dbi;
}

 

QString DOCConverter::readText() {
	FUNCTIONSETUP;
	if (txtfilename.isEmpty()) return QString();
	QFile docfile(txtfilename);
	if (!docfile.open(IO_ReadOnly))
	{
		emit logError(i18n("Unable to open text file %1 for reading.").arg(txtfilename));
		return QString();
	}

	QTextStream docstream(&docfile);

	QString doc = docstream.read();
	docfile.close();
	return doc;
}



int DOCConverter::findBmkEndtags(QString &text, bmkList&fBmks) {
	FUNCTIONSETUP;
	// Start from the end of the text
	int pos = text.length() - 1, nr=0;
	bool doSearch=true;
	while (pos >= 0/* && doSearch*/) {
		DEBUGCONDUIT<<"Current character is \'"<<text[pos].latin1()<<"\'"<<endl;
		// skip whitespace until we reach a >
		while (text[pos].isSpace() && pos >= 0) {
			DEBUGCONDUIT<<"Skipping whitespaces at the end of the file"<<endl;
			pos--;
		}
		// every other character than a > is assumed to belong to the text, so there are no more bookmarks.
		if (pos < 0 || text[pos] != '>') {
			DEBUGCONDUIT<<"Current character \'"<<text[pos].latin1()<<"\' at position "<<pos<<" is not and ending >. Finish searching for bookmarks."<<endl;
		
			pos=-1;
			break;
		} else {
			int endpos = pos;
			doSearch=true;
			DEBUGCONDUIT<<"Found the ending >, now looking for the opening <"<<endl;
			
			// Search for the opening <. There must not be a newline in the bookmark text.
			while (doSearch && pos > 0) {
//				DEBUGCONDUIT<<"pos="<<pos<<", char="<<text[pos].latin1()<<endl;
				pos--;
				if (text[pos] == '\n') {
					DEBUGCONDUIT<<"Found carriage return at position "<<pos<<" inside the bookmark text, assuming this is not a bookmark, and the text ends in a >"<<endl;
					doSearch = false;
					pos = -1;
					break;
				}
				if (text[pos] == '<') {
					fBmks.append(new docMatchBookmark(text.mid(pos + 1, endpos - pos - 1)));
					++nr;
					DEBUGCONDUIT<<"Found opening < at position "<<pos<<", bookmarktext ="<<text.mid(pos+1, endpos-pos-1)<<endl;
					text.remove(pos, text.length());
					pos--;
					doSearch = false;
				}
			}
		}
		DEBUGCONDUIT<<"Finished processing the next bookmark, current position: "<<pos<<endl;
	}
	return nr;
}
		
int DOCConverter::findBmkInline(QString &text, bmkList &fBmks) {
	FUNCTIONSETUP;
//	bmkList res;
	int nr=0;
	QRegExp rx(CSL1("<\\*(.*)\\*>"));

	rx.setMinimal(TRUE);
	int pos = 0;
	while (pos >= 0) {
		pos = rx.search(text, pos);
		if (pos >= 0) {
			fBmks.append(new docBookmark(rx.cap(1), pos+1));
			++nr;
			text = text.remove(pos, rx.matchedLength());
		}
	}
	return nr;
}

int DOCConverter::findBmkFile(QString &, bmkList &fBmks) {
	FUNCTIONSETUP;
	int nr=0;
	
	QString bmkfilename = txtfilename;
	if (bmkfilename.endsWith(CSL1(".txt"))){
		bmkfilename.remove(bmkfilename.length()-4, 4);
	}
	QString oldbmkfilename=bmkfilename;
	bmkfilename+=CSL1(BMK_SUFFIX);
	QFile bmkfile(bmkfilename);
	if (!bmkfile.open(IO_ReadOnly)) 	{
		bmkfilename=oldbmkfilename+CSL1(PDBBMK_SUFFIX);
		bmkfile.setName(bmkfilename);
		if (!bmkfile.open(IO_ReadOnly)) {
			DEBUGCONDUIT<<"Unable to open bookmarks file "<<bmkfilename<<" for reading the bookmarks of "<<docdb ->dbPathName()<<endl;
			return 0;
		}
	}
	
	DEBUGCONDUIT<<"Bookmark file: "<<bmkfilename<<endl;

	QTextStream bmkstream(&bmkfile);
	QString line;
	while ( !(line=bmkstream.readLine()).isEmpty() ) {
		if (!line.isEmpty() && !line.startsWith(CSL1("#")) ) {
			QStringList bmkinfo=QStringList::split(CSL1(","), line);
			int fieldnr=bmkinfo.count();
			// We use the same syntax for the entries as MakeDocJ bookmark files:
			//   <bookmark>,<string-to-search>,<bookmark-name-string>,<starting-bookmark>,<ending-bookmark>
			// For an explanation see: http://home.kc.rr.com/krzysztow/PalmPilot/MakeDocJ/index.html
			if (fieldnr>0){
				DEBUGCONDUIT<<"Working on bookmark \""<<line<<"\""<<endl;
				docMatchBookmark*bmk=0L;
				QString bookmark=bmkinfo[0];
				bool ok;
				int pos=bookmark.toInt(&ok);
				if (ok) {
					if (fieldnr>1) {
						QString name(bmkinfo[1]);
						DEBUGCONDUIT<<"Bookmark \""<<name<<"\" set at position "<<pos<<endl;
						fBmks.append(new docBookmark(name, pos));
					}
				} else if (bookmark==CSL1("-") || bookmark==CSL1("+")) {
					if (fieldnr>1) {
						QString patt(bmkinfo[1]);
						QString name(patt);
						if (fieldnr>2) {
							int cap=bmkinfo[2].toInt(&ok);
							if (ok) {
								bmk=new docRegExpBookmark(patt, cap);
							} else {
								name=bmkinfo[2];
								bmk=new docRegExpBookmark(patt, name);
							}
						} else{
							bmk=new docRegExpBookmark(patt, name);
						}
						// The third entry in the line (optional) denotes the index of a capture subexpression (if an integer) or the bookmark text as regexp (if a string)
						DEBUGCONDUIT<<"RegExp Bookmark, pattern="<<patt<<", name="<<name<<endl;
						if (bmk) {
							if (bookmark==CSL1("-")) {
								bmk->from=1;
								bmk->to=1;
							} else {
								if (fieldnr>3) {
									bool ok;
									int tmp=bmkinfo[3].toInt(&ok);
									if (ok) bmk->from=tmp;
									if (fieldnr>4) {
										tmp=bmkinfo[4].toInt(&ok);
										if (ok) bmk->to=tmp;
									}
								}
							}
							fBmks.append(bmk);
							bmk=0L;
						} else {
							DEBUGCONDUIT<<"Could not allocate bookmark "<<name<<endl;
						}
					} else {
						DEBUGCONDUIT<<"RegExp bookmark found with no other information (no bookmark pattern nor name)"<<endl;
					}
				} else {
					QString pattern(bookmark);
					if (fieldnr>1) pattern=bmkinfo[1];
					if (fieldnr>2) bookmark=bmkinfo[2];
					DEBUGCONDUIT<<"RegExp Bookmark, pattern="<<pattern<<", name="<<bookmark<<endl;
					bmk=new docRegExpBookmark(pattern, bookmark);
					if (bmk) {
						bmk->from=1;
						bmk->to=1;
						fBmks.append(bmk);
					}
				}
			} // fieldnr>0
		} // !line.isEmpty()
	} // while
	return nr;
}

bool DOCConverter::convertTXTtoPDB() {
	FUNCTIONSETUP;
	
	if (!docdb) {
		emit logError(i18n("Unable to open Database for writing"));
		return false;
	}
	
	QString text = readText();

	if (fBmkTypes & eBmkEndtags) {
		findBmkEndtags(text, fBookmarks);
	}	// end: EndTag Bookmarks


	// Search for all tags <* Bookmark text *> in the text. We have to delete them immediately, otherwise the later bookmarks will be off.
	if (fBmkTypes & eBmkInline) {
		findBmkInline(text, fBookmarks);
	}	 // end: Inline Bookmarks


	// Read in regular expressions and positions from an external file (doc-filename with extension .bmk)
	if (fBmkTypes & eBmkFile)
	{
		findBmkFile(text, fBookmarks);
	}

	// Process the bookmarks: find the occurrences of the regexps, and sort them if requested:
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
		DEBUGCONDUIT<<bmk->bmkName.left(20)<<" at position "<<bmk->position<<endl;
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
	docHead.version=compress?DOC_COMPRESSED:DOC_UNCOMPRESSED;
	docHead.numRecords=(int)( (text.length()-1)/docHead.recordSize)+1;
	PilotRecord*rec=docHead.pack();
	docdb->writeRecord(rec);
	KPILOT_DELETE(rec);
	
	DEBUGCONDUIT << "Write header record: length="<<text.length()<<", compress="<<compress<<endl;

	// First compress the text, then write out the bookmarks and - if existing - also the annotations
	int len=text.length();
	int start=0,reclen=0;
	int recnum=0;
	while (start<len)
	{
		reclen=min(len-start, PilotDOCEntry::TEXT_SIZE);
		DEBUGCONDUIT << "Record #"<<recnum<<", reclen="<<reclen<<", compress="<<compress<<endl;
		
		PilotDOCEntry recText;
//		recText.setText(text.mid(start, reclen), reclen);
		recText.setText(text.mid(start, reclen));
//		if (compress) 
		recText.setCompress(compress);
		PilotRecord*textRec=recText.pack();
		docdb->writeRecord(textRec);
		++recnum;
		start+=reclen;
		KPILOT_DELETE(textRec);
	}
	
	recnum=0;
	// Finally, write out the bookmarks
	for (bmk = pdbBookmarks.first(); bmk; bmk = pdbBookmarks.next())
//	for (bmkList::const_iterator it=pdbBookmarks.begin(); it!=pdbBookmarks.end(); ++it)
	{
		++recnum;
		DEBUGCONDUIT << "Bookmark #"<<recnum<<", Name="<<bmk->bmkName.left(20)<<", Position="<<bmk->position<<endl;
		
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



bool DOCConverter::convertPDBtoTXT()
{
	FUNCTIONSETUP;
	if (txtfilename.isEmpty()) {
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
	
	DEBUGCONDUIT<<"Database "<<docdb->dbPathName()<<" has "<<header.numRecords<<" text records, "<<endl
		<<" total number of records: "<<docdb->recordCount()<<endl
		<<" position="<<header.position<<endl
		<<" recordSize="<<header.recordSize<<endl
		<<" spare="<<header.spare<<endl
		<<" storyLen="<<header.storyLen<<endl
//		<<" textRecordSize="<<header.textRecordSize<<endl
		<<" version="<<header.version<<endl;

	// next come the header.numRecords real document records (might be compressed, see the version flag in the header)
	QFile docfile(txtfilename);
	if (!docfile.open(IO_WriteOnly))
	{
		emit logError(i18n("Unable to open output file %1.").arg(txtfilename));
		KPILOT_DELETE(docdb);
		return false;
	}
	QString doctext;
	for (int i=1; i<header.numRecords+1; ++i)
	{
		PilotRecord*rec=docdb->readRecordByIndex(i);
		if (rec)
		{
			PilotDOCEntry recText(rec, header.version==DOC_COMPRESSED);
			doctext.append(recText.getText());
			DEBUGCONDUIT<<"Record "<<i<<endl;
			KPILOT_DELETE(rec);
		} else {
			emit logMessage(i18n("Could not read text record #%1 from Database %2").arg(i).arg(docdb->dbPathName()));
		}
	}

	// After the document records possibly come a few bookmark records, so read them in and put them in a separate bookmark file.
	// for the ztxt conduit there might be annotations after the bookmarks, so the upper bound needs to be adapted.
	int upperBmkRec=docdb->recordCount();
	bmkSortedList bmks;
	bmks.setAutoDelete(TRUE);
	for (int i=header.numRecords+1; i<upperBmkRec; ++i)
	{
		PilotRecord*rec=docdb->readRecordByIndex(i);
		if (rec)
		{
			PilotDOCBookmark bookie(rec);
			docBookmark*bmk=new docBookmark(bookie.bookmarkName, bookie.pos);
			bmks.append(bmk);
			KPILOT_DELETE(rec);
		} else {
			emit logMessage(i18n("Could not read bookmark record #%1 from Database %2").arg(i).arg(docdb->dbPathName()));
		}
	}
	// TODO: Sort the list of bookmarks according to their position
	docBookmark::compare_pos=true;
	bmks.sort();

	if ((fBmkTypes & eBmkFile) && (bmks.count()>0))
	{
		QString bmkfilename = docfile.name();
		if (bmkfilename.endsWith(CSL1(".txt"))){
			bmkfilename.remove(bmkfilename.length()-4, 4);
		}
		bmkfilename+=CSL1(PDBBMK_SUFFIX);
		QFile bmkfile(bmkfilename);
		if (!bmkfile.open(IO_WriteOnly))
		{
			emit logError(i18n("Unable to open file %1 for the bookmarks of %2.")
				.arg(bmkfilename).arg(docdb ->dbPathName()));
		}
		else
		{
			DEBUGCONDUIT<<"Writing "<<upperBmkRec-header.numRecords<<
				"("<<upperBmkRec<<") bookmarks to file "<<bmkfilename<<endl;
			QTextStream bmkstream(&bmkfile);
			for (docBookmark*bmk=bmks.first(); bmk; bmk=bmks.next())
			{
				bmkstream<<bmk->position<<", "<<bmk->bmkName<<endl;
			}
			//bmkstream.close();
			bmkfile.close();
		}
	}
	if (fBmkTypes & eBmkInline)
	{
		for (docBookmark*bmk=bmks.last(); bmk; bmk=bmks.prev())
		{
			doctext.insert(bmk->position, QString(CSL1("<*") +
				bmk->bmkName +
				CSL1("*>")));
		}
	}

	// Finally, write the actual text out to the file.
	QTextStream docstream(&docfile);
	docstream<<doctext;
	//docstream.close();
	docfile.close();
	docdb->cleanup();
	// reset all records to unchanged. I don't know if this is really such a wise idea?
	docdb->resetSyncFlags();
	return true;
}


