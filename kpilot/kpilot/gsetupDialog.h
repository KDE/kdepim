// gsetupDialog.h
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// This is the KDE2 version of gsetupDialog.h, intended for KPilot 4.
// It should still compile under KDE1, though. Major changes have been
// made in the types of functions (const char * -> const QString &) as
// well as the implementation of some functions.
//
// $Id$





// Includes reduced to a minimum
//
//
#include <qstring.h>
#include <qtabdlg.h>
#include <qlist.h>


class setupDialog;
class KConfig;

class setupDialogPage : public QWidget
{
	Q_OBJECT

public:
	/**
	* Make a page for a setup dialog. Use the
	* given tab name, which should already be
	* translated by i18n(). The KConfig * can
	* be used so that the page can read its 
	* configuration from the file. The group of
	* the configuration file is set by the dialog,
	* and pages should not change it.
	*/
#if (QT_VERSION > 199)
	setupDialogPage(const QString &tabname,
		setupDialog *parent,
		KConfig *c=0L);
#else
	setupDialogPage(const char *tabname,
		setupDialog *parent,
		KConfig *c=0L);
#endif

	/**
	* If the user clicks "OK" in a setup dialog,
	* this function is called so that the page can
	* save whatever settings are configured by it
	* to the configuration file. Note that the
	* group of the configuration file should
	* not be changed by the tab page itself -- 
	* the group is a property of the dialog,
	* not of the page.
	*/
	virtual int commitChanges(KConfig *);
	/**
	* If the user cancels a setup dialog, this
	* function is called for all the pages. 
	* This is to "back out" any changes made by
	* the page -- just in case you have a page that
	* makes changes to something before the user OKs it.
	*/
	virtual int cancelChanges(KConfig *);

	const QString &tabName() { return fTabName; } ;

protected:
	/**
	* Remember who the parent is so we can make use
	* of parent services (for example, changing tab
	* settings in different tabs)
	*/
	setupDialog *parentSetup;

private:
	/**
	* This is the string that should appear on "my" tab
	* in the tab dialog. This string should be translated
	* already.
	*/
	QString fTabName;
} ;


/** 
* All KPilot setup dialogs should have an info page;
* this class provides a consistent look for them.
* Give the constructor a title for the info --
* usually programe name+version, a list of authors,
* and perhaps a comment.
*/
class setupInfoPage : public setupDialogPage
{
	Q_OBJECT

public:
#if (QT_VERSION > 199)
	setupInfoPage(setupDialog *parent,
		const QString &title,
		const QString &authors,
		const QString &comment=QString(),
		const char *progname=0L);
#else
	setupInfoPage(setupDialog *parent,
		const char *title,
		const char *authors,
		const char *comment=0L,
		const char *progname=0L);
#endif
} ;


class setupDialog : public QTabDialog
{
	Q_OBJECT

public:
	/**
	* make a setup dialog for interacting with
	* the user. Note that subclasses still
	* need to call setupDialog::setupWidget
	* to get all the right behavior.
	*
	* If caption is NULL, use the name of
	* the dialog. You can also make modal
	* setup dialogs like this.
	*
	* @see setupWidget
	* @see quitOnClose
	*/
#if (QT_VERSION > 199)
	setupDialog(QWidget *parent,
		const QString &name,
		const QString &caption=QString::null,
		bool modal=false);
#else
	setupDialog(QWidget *parent,
		const char *name,
		const char *caption=0L,
		bool modal=false);
#endif

	/**
	* Assuming each setup dialog only
	* applies to a single group of settings
	* is a good idea -- it keeps the complexity
	* of each single dialog down.
	* This function returns the name of
	* the group to use in the config file.
	*/
	const QString &groupName() { return fGroupName; } ;

public slots:
	/**
	* commitChanges is called when the user
	* clicks on the OK button; it does any
	* global sanity checking necessary (currently none)
	* and calls the commitChanges member function
	* for all the pages in the dialog.
	* Those should do local sanity checking and
	* save the changes made.
	* @see addPage
	* @see pages
	*/
	void commitChanges();

	/**
	* cancelChanges tells each page to
	* cancel its changes --- usually
	* this means to do nothing.
	* @see commitChanges
	*/
	void cancelChanges();

public:
	typedef enum { 
		QueryFileYes=1, 
		QueryFileNo=2, 
		QueryFileExists=0 } QueryFileResult;

	/**
	* queryFile is used in sanity checking, mostly.
	* It checks that the given filename, described as
	* filelabel, actually exists. If not, the user
	* is asked whether he or she actually wants to
	* use this nonexistent file. (The filename itself
	* is obviously not a unicode string, so this has
	* been changed back to a const char *)
	*
	* This function should probably be static and
	* take an additional QWidget parameter for the 
	* parent of the question box.
	*
	* @return	0 for file exists, otherwise the return of
	*		KMsgBox / KMessageBox yes-no queries.
	* @see commitChanges
	*/
        int queryFile(
		const QString& filelabel,
		const char *filename) 
		{ return queryFile(this,filelabel,filename); };

	static int queryFile(QWidget *parent,
		const QString& filelabel,
		const char *filename);


protected:
	/**
	* setupWidget does final layout stuff,
	* sizes the dialog window the right size,
	* etc. It needs to be called by subclasses
	* after they have added all the pages
	* they want.
	* @see addPage
	*/
	void setupWidget();

	/**
	* addPage adds (surprise!) a page to the dialog,
	* and keeps track of it to make sure that we call
	* commitChanges() before closing the dialog.
	*/
	int addPage(setupDialogPage *);

private:
	/**
	* This is the actual name of the group the 
	* dialog applies to. Note that the group name
	* must NOT be translated, since that would
	* effectively hide configuration settings when
	* changing locales.
	*/
	QString fGroupName;

	/**
	* pageCount keeps track of how many pages
	* have been added to the dialog thus far.
	*
	* @see addPage
	* @see pages
	*/
	int pageCount;

	/**
	* This keeps pointers to all the pages
	* added to the dialog.
	* @see addPage
	* @see commitChanges
	*/
	QList<setupDialogPage> pages;


	/**
	* Modal dialogs shouldn't kill the
	* app when done; on the other hand for
	* conduits where this is the only (non modal)
	* window, the app should exit on close.
	* Remember the modal setting from the constructor.
	*/
	bool quitOnClose;
} ;


// $Log$
// Revision 1.4  2000/07/16 12:17:16  adridg
// Moved partway to KDE2
//
