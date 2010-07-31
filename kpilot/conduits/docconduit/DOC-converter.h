#ifndef _DOC_CONVERTER_H
#define _DOC_CONVERTER_H
/* DOC-converter.h                           KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#define DOC_UNCOMPRESSED 1
#define DOC_COMPRESSED 2


#define BMK_SUFFIX ".bmk"
#define PDBBMK_SUFFIX ".bm"

#include <tqptrlist.h>
#include <tqobject.h>

class PilotDatabase;


/****************************************************************************************************
 *  various bookmark classes. Most important is the bmkList  findMatches(TQString, bmkList &) function,
 *  which needs to return a list of all bookmarks found for the given bookmark expression.
 *  A bookmark usually consists of a bookmark text and an offset into the text document.
 ****************************************************************************************************/

class docBookmark;
#define bmkList TQPtrList<docBookmark>
#define bmkSortedList TQSortedList<docBookmark>

class docBookmark {
public:
	static bool compare_pos;
	docBookmark():bmkName(), position(0) { };
	docBookmark(TQString name, long int pos):bmkName(name), position(pos) { };
	docBookmark(const docBookmark &bmk):bmkName(bmk.bmkName),position(bmk.position){};
	virtual ~ docBookmark() { };
	virtual int findMatches(TQString, bmkList &fBookmarks) {
		FUNCTIONSETUP;
		fBookmarks.append(new docBookmark(*this));
		return 1;
	};

	TQString bmkName;
	long int position;
};

class docMatchBookmark:public docBookmark {
 public:
	docMatchBookmark():docBookmark() { from=0; to=100;};
	docMatchBookmark(TQString pattrn, int options=0):docBookmark(),
		pattern(pattrn), opts(options) { from=0; to=100; };
	docMatchBookmark(TQString pattrn, TQString bmkname,
		int options=0):docBookmark(bmkname, 0), pattern(pattrn),
		opts(options) { from=0; to=100; };
	virtual ~ docMatchBookmark() { };

	virtual int findMatches(TQString, bmkList &fBookmarks);
	TQString pattern;
	int opts;
	int from, to;
};

class docRegExpBookmark:public docMatchBookmark {
 public:
	docRegExpBookmark():docMatchBookmark() { capSubexpression=-1;};
	docRegExpBookmark(TQString regexp, int cap=0,
		int options=0):docMatchBookmark(regexp, options) {capSubexpression=cap; };
	docRegExpBookmark(TQString pattrn, TQString bmkname,
		int options=0):docMatchBookmark(pattrn, bmkname, options) { capSubexpression=-1; };
	virtual ~ docRegExpBookmark() { };

	virtual int findMatches(TQString, bmkList &fBookmarks);
	int capSubexpression;
};


/*************************************************************************************************************
 *  The converter class that does the real work for us.
 *************************************************************************************************************/

class DOCConverter:public TQObject {
Q_OBJECT
private:
	PilotDatabase * docdb;
	TQString txtfilename;
	TQString bmkfilename;
	bool compress;

	bmkList fBookmarks;
public:
	enum eSortBookmarksEnum
	{
		eSortNone,
		eSortPos,
		eSortName
	} eSortBookmarks;

public:
	 DOCConverter(TQObject *parent=0L, const char *name=0L);
	 virtual ~ DOCConverter();

	TQString readText();
	void setTXTpath(TQString path, TQString file);
	void setTXTpath(TQString filename);
	void setPDB(PilotDatabase * dbi);
	TQString txtFilename() const {return txtfilename;}
	TQString bmkFilename() const {return bmkfilename;}
	void setBmkFilename(TQString bmkf) { bmkfilename=bmkf;}

	bool getCompress() const { return compress; };
	void setCompress(bool newcomp) {compress=newcomp;};

	bool convertTXTtoPDB();
	bool convertPDBtoTXT();

	int setBookmarks(bmkList bookmarks) {
		fBookmarks = bookmarks;
		return fBookmarks.count();
	};
	int clearBookmarks() {
		fBookmarks.clear();
		return fBookmarks.count();
	};
	int addBookmark(docBookmark*bookmark) {
		fBookmarks.append(bookmark);
		return fBookmarks.count();
	};

	int findBmkEndtags(TQString &, bmkList&);
	int findBmkInline(TQString &, bmkList&);
	int findBmkFile(TQString &, bmkList&);


	void setSort(enum eSortBookmarksEnum sort) {eSortBookmarks=sort;}
	enum eSortBookmarksEnum getSort() {return eSortBookmarks;}

	enum eBmkTypesEnum {
		eBmkNone = 0,
		eBmkFile = 1,
		eBmkInline = 2,
		eBmkEndtags = 4,
		eBmkDefaultBmkFile = 8
	} fBmkTypes;
	void setBookmarkTypes(int types) {
		fBmkTypes = (eBmkTypesEnum) types;
	};

protected:
	int findBookmarks();

private:
	void readConfig();
signals:
	void logMessage(const TQString &);
	void logError(const TQString &);
};

#endif
