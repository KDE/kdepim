/*
 * (C) 1999, 2000 Sirtaj Singh Kang <taj@kde.org>
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#ifndef KornOptDlg_h
#define KornOptDlg_h

//class KMailDrop;
#include "maildrop.h"
#include<qintdict.h>
#include<qtabdialog.h>
#include<qptrdict.h>
#include<kdialogbase.h>

#include "kornset.h"
#include "dropdlg.h"

class QRadioButton;
class KDropManager;
class QListBox;

class KornOptDlg: public KDialogBase
{
	Q_OBJECT

private:

	/**
	* List of @ref KDropCfgDialog objects, the keys are pointers to
	* @ref KMailDrop objects. This is not created till required
	* and should normally be deferenced using the @ref ::dlgList
	* accessor.
	* @see ::dlgList
	*/
	QPtrDict<KDropCfgDialog> _dropDlgList;

	/**
	* Returns the dictionary of open drop configuration dialogs.
	* If the list has not already been created, it is created.
	*
	* @see ::_dropDlgList
	*/
	QPtrDict<KDropCfgDialog> _dlgList;

	/**
	* Returns an associated and initialized drop dialog for the
	* monitor, or return an existing one if it already exists.
	*
	* @param drop	The monitor for which the dialog will be returned.
	* @param checkExists	If true, the check for an existing
	*			dialog will be performed.
	*/
	KDropCfgDialog *dropDialog( KMailDrop *drop, bool checkExists=true );

	QRadioButton	*_horiz;
	QRadioButton	*_vert;
	QRadioButton	*_dock;

	QListBox	*_listBox;

	QIntDict<KMailDrop>	_monitorList;

	KDropManager	*_manager;

	/** 
	 * Fills the list box from the monitor list.
	 */
	void fillListBox();

	/** 
	 * Find the monitor using the name from the current
	 * item in @ref ::_listBox
	 */
	KMailDrop *getMonitor() const;

public:
	KornOptDlg( KDropManager *manager, QWidget *parent=0 );
	virtual ~KornOptDlg();

	KornSettings::Layout kornLayout() const;
	void setKornLayout( KornSettings::Layout );

protected slots:

	void newBox();
	void deleteBox();
	void cloneBox();
	void modifyBox();

signals:
	void listUpdated( bool hasItems );

private slots:
	/**
	* This should be connected to the closed signal of all
	* created drop config dialogs. The implementation deletes
	* the dialog.
	*/
	void dlgClosed( KDropCfgDialog * );

	void updateList();

	void enableButtons();
};

#endif
