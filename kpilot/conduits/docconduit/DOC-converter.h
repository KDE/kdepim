#ifndef _DOC_CONVERTER_H
#define _DOC_CONVERTER_H
// doc-converter.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$
//

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#define DOC_UNCOMPRESSED 1
#define DOC_COMPRESSED 2


#define BMK_SUFFIX ".bmk"
#define PDBBMK_SUFFIX ".bm"

#include <qptrlist.h>
#include <qobject.h>


class PilotDatabase;


/****************************************************************************************************
 *  various bookmark classes. Most important is the bmkList  findMatches(QString, bmkList &) function, 
 *  which needs to return a list of all bookmarks found for the given bookmark expression.
 *  A bookmark usually consists of a bookmark text and an offset into the text document.
 ****************************************************************************************************/

class docBookmark;
#define bmkList QPtrList<docBookmark>
#define bmkSortedList QSortedList<docBookmark>

class docBookmark {
public:
	static bool compare_pos;
	docBookmark():bmkName(), position(0) { };
	docBookmark(QString name, long int pos):bmkName(name), position(pos) { };
	docBookmark(const docBookmark &bmk):bmkName(bmk.bmkName),position(bmk.position){};
	virtual ~ docBookmark() { };
	virtual int findMatches(QString, bmkList &fBookmarks) {
		FUNCTIONSETUP;
		fBookmarks.append(new docBookmark(*this));
		return 1;
	};

	QString bmkName;
	long int position;
};

class docMatchBookmark:public docBookmark {
 public:
	docMatchBookmark():docBookmark() { from=0; to=100;};
	docMatchBookmark(QString pattrn, int options=0):docBookmark(),
		pattern(pattrn), opts(options) { from=0; to=100; };
	docMatchBookmark(QString pattrn, QString bmkname,
		int options=0):docBookmark(bmkname, 0), pattern(pattrn), 
		opts(options) { from=0; to=100; };
	virtual ~ docMatchBookmark() { };

	virtual int findMatches(QString, bmkList &fBookmarks);
	QString pattern;
	int opts;
	int from, to;
};

class docRegExpBookmark:public docMatchBookmark {
 public:
	docRegExpBookmark():docMatchBookmark() { capSubexpression=-1;};
	docRegExpBookmark(QString regexp, int cap=0,
		int options=0):docMatchBookmark(regexp, options) {capSubexpression=cap; };;
	docRegExpBookmark(QString pattrn, QString bmkname,
		int options=0):docMatchBookmark(pattrn, bmkname, options) { capSubexpression=-1; };
	virtual ~ docRegExpBookmark() { };

	virtual int findMatches(QString, bmkList &fBookmarks);
	int capSubexpression;
};


/*************************************************************************************************************
 *  The converter class that does the real work for us.
 *************************************************************************************************************/

class DOCConverter:public QObject {
Q_OBJECT
private:
	PilotDatabase * docdb;
	QString docfilename;
	QString bmkfilename;
	bool compress;
	enum eSortBookmarksEnum
	{
		eSortNone,
		eSortPos,
		eSortName
	} eSortBookmarks;

	bmkList fBookmarks;

public:
	 DOCConverter(QObject *parent=0L, const char *name=0L);
	 virtual ~ DOCConverter();

	QString readText();
	void setDOCpath(QString path, QString file);
	void setDOCpath(QString filename);
	void setPDB(PilotDatabase * dbi);
	QString docFilename() const {return docfilename;}
	QString bmkFilename() const {return bmkfilename;}
	void setBmkFilename(QString bmkf) { bmkfilename=bmkf;}

	bool getCompress() const { return compress; };
	void setCompress(bool newcomp) {compress=newcomp;};

	bool convertDOCtoPDB();
	bool convertPDBtoDOC();

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
	
	int findBmkEndtags(QString &, bmkList&);
	int findBmkInline(QString &, bmkList&);
	int findBmkFile(QString &, bmkList&);
	

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
	void logMessage(const QString &);
	void logError(const QString &);
};

#endif
