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


#ifndef _GSETUPDIALOG_H
#define _GSETUPDIALOG_H

#ifndef __OPTIONS_H
#include "options.h"
#endif


// Includes reduced to a minimum
//
//
#include <qstring.h>
#include <qtabdlg.h>
#include <qlist.h>
#include <kmessagebox.h>

class setupDialog;
class KConfig;

class setupDialogPage : public QWidget
{
	Q_OBJECT

public:
	/**
	* Make a page for a setup dialog. Use the
	* given tab name, which should already be
	* translated by i18n(). 
	*
	* Subclasses will have the KConfig& parameter
	* which can
	* be used so that the page can read its 
	* configuration from the file. The group of
	* the configuration file is set by the dialog,
	* and pages should not change it.
	*/
	setupDialogPage(const QString &tabname,
		setupDialog *parent /* , Add for subclasses
		KConfig& c */ );
		

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
	virtual int commitChanges(KConfig&);

	/**
	* If the user cancels a setup dialog, this
	* function is called for all the pages. 
	* This is to "back out" any changes made by
	* the page -- just in case you have a page that
	* makes changes to something before the user OKs it.
	*/
	virtual int cancelChanges(KConfig&);

	/** Before committing changes, the dialog calls
	* validateChanges() on all the pages to see if the
	* information is consistent (whatever the pages think
	* that is). If there is a page with a non-valid state,
	* the dialog complains and the dialog is *NOT* closed.
	*
	* Returns 0 if all is well.
	* Returns non-0 if the data is inconsistent and the
	*	dialog can't be sensibly closed.
	*/
	virtual int validateChanges(KConfig&);

	/**
	* @return The tab name given to the constructor
	*/
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
* All KPilot setup dialogs should have an info page
* unless they are in an application that has an
* "about" menu item somewhere.
*
* This class provides a consistent look for them.
* The KDE2 version takes al its information from the
* about data provided to KInstance.
*/
class setupInfoPage : public setupDialogPage
{
	Q_OBJECT

public:
	/**
	* Since the about page is too small to include the licence
	* and the complete list of authors and contributors, we
	* put a button on the page that pops up the about box.
	* This is of course only useful (or desireable even) if
	* there is no "standard" way to get the about box.
	*
	* With the guideline above (info page unless about menu)
	* you will almost never use includeabout=false.
	*/
	setupInfoPage(setupDialog *parent,bool includeabout=true);

protected slots:
	/**
	* Displays the KAboutApplication box when the user clicks
	* on the about button.
	*/
	void showAbout();
} ;


class setupDialog : public QTabDialog
{
	Q_OBJECT

public:
	/**
	* Make a setup dialog for interacting with
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
	setupDialog(QWidget *parent,
		const QString &name,
		const QString &caption=QString::null,
		bool modal=false);

	/**
	* Assuming each setup dialog only
	* applies to a single group of settings
	* is a good idea -- it keeps the complexity
	* of each single dialog down.
	* This function returns the name of
	* the group to use in the config file.
	*/
	const QString &groupName() { return fGroupName; } ;

	/**
	* For a particular dialog, return the configuration
	* version -- a number that indicates what revision of
	* the settings are to be written. The number should be
	* incremented when considerable changes occur to the setup.
	*/
	int getConfigurationVersion() const { return fConfigVersion; } ;
	
	/**
	* A convenience function; returns the stored configuration
	* number from a config file.
	*/
	static int getConfigurationVersion(KConfig& c,
		const QString &group=QString::null);

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
		QueryFileYes=KMessageBox::Yes, 
		QueryFileNo=KMessageBox::No, 
		QueryFileExists=0,
		QueryNull=3 } QueryFileResult;

	/**
	* queryFile is used in sanity checking, mostly.
	* It checks that the given filename, described as
	* filelabel, actually exists. If not, the user
	* is asked whether he or she actually wants to
	* use this nonexistent file. 
	*
	* KDE2: The filelabel passed to queryFile should be
	* already translated and may contain %1 as a
	* marker where the filename should be inserted.
	* queryFile appends "Really use this file?" to
	* the label to form a question.
	*
	* KDE1: The filelabel should be translated already.
	* The filename is appended to it along with "Really
	* use this file?".
	*
	* 
	* @return	0 for file exists, otherwise the return of
	*		KMsgBox / KMessageBox yes-no queries.
	*		Return value QueryNull (=3) for null strings.
	* @see commitChanges
	*/
        int queryFile(
		const QString& filelabel,
		const QString &filename) 
		{ return queryFile(this,filelabel,filename); };

	static int queryFile(QWidget *parent,
		const QString& filelabel,
		const QString &filename);


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


	void setConfigurationVersion(int r) { fConfigVersion=r; } ;

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

	int fConfigVersion;
} ;

// $Log$
// Revision 1.4  2000/11/10 08:32:33  adridg
// Fixed spurious config new() and delete()
//
// Revision 1.3  2000/07/30 10:01:55  adridg
// Completed KDE2 layout
//
// Revision 1.2  2000/07/24 04:10:00  pilone
// 	First round of KDE 2.0 changes...almost there..
//
// Revision 1.4  2000/07/16 12:17:16  adridg
// Moved partway to KDE2
//


#endif /* _GSETUPDIALOG_H */

