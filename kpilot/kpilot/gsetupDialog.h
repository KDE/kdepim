// kpilotSetupDialog.h
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
// $Revision$





#include <qtabdlg.h>

#define setupDialog_MAX_OPTIONS_PAGES 	(16)

class setupDialog;
class KConfig;

class setupDialogPage : public QWidget
{
	Q_OBJECT

public:
	setupDialogPage(setupDialog *parent,KConfig *c=0L);

	virtual int commitChanges(KConfig *);
	virtual int cancelChanges(KConfig *);

	virtual const char *tabName()=0;

protected:
	setupDialog *parentSetup;

} ;


class setupInfoPage : public setupDialogPage
{
	Q_OBJECT

public:
	setupInfoPage(setupDialog *parent,
		const char *title,
		const char *authors,
		const char *comment=0L,
		const char *progname=0L);

	virtual const char *tabName();
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
	setupDialog(QWidget *parent,
		const char *name,
		const char *caption=0L,
		bool modal=false);

	/**
	* Assuming each setup dialog only
	* applies to a single group of settings
	* is a good idea -- it keeps the complexity
	* of each single dialog down.
	* This function returns the name of
	* the group to use in the config file.
	*/
	virtual const char *groupName();

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
	/**
	* queryFile is used in sanity checking, mostly.
	* It checks that the given filename, described as
	* filelabel, actually exists. If not, the user
	* is asked whether he or she actually wants to
	* use this nonexistent filename.
	*
	* @return	1 for yes 2 for no 0 for file exists
	* @see commitChanges
	*/
        int queryFile(const QString& filelabel,const QString& filename);


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
	setupDialogPage *pages[setupDialog_MAX_OPTIONS_PAGES];


	/**
	* Modal dialogs shouldn't kill the
	* app when done; on the other hand for
	* conduits where this is the only (non modal)
	* window, the app should exit on close.
	* Remember the modal setting from the constructor.
	*/
	bool quitOnClose;
} ;
