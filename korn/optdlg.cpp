/*
 * (C) 1999, 2000 Sirtaj Singh Kang <taj@kde.org>
 * (C) 2000 Rik Hemsley <rik@kde.org>
 */

#include <assert.h>

#include <kbuttonbox.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvbox.h>

#include "utils.h"
#include "maildrop.h"
#include "dropman.h"
#include "optdlg.h"
#include "typedlg.h"


KornOptDlg::KornOptDlg( KDropManager *man, QWidget *parent )
	:
	KDialogBase(
	  Tabbed,
	  i18n("Configure"),
	  Close,
	  Close,
	  parent,
	  "KornOptDlg",
	  true,
	  true
	),
	_manager(man)
{
	assert(_manager);

	_dlgList.setAutoDelete(true);
	_monitorList.setAutoDelete( false );

	connect( _manager, SIGNAL(monitorCreated()), this,
		SLOT(updateList()));

	connect( _manager, SIGNAL(monitorDeleted()), this,
		SLOT(updateList()));

	connect( _manager, SIGNAL(configChanged()), this,
		SLOT(updateList()));

	QHBox * boxes  = addHBoxPage(i18n("&Boxes"));
	QVBox * display = addVBoxPage(i18n("&Display"));

	// Boxes

	// initialize monitor list
	// list is filled at the end so that all signals
	// are connected properly.

	_listBox = new QListBox(boxes);

	connect( _listBox, SIGNAL(selectionChanged()), 
			this, SLOT(enableButtons()));
	connect( _listBox, SIGNAL( doubleClicked(QListBoxItem*)), 
			this, SLOT(modifyBox()));

	// up/down buttons

	// initialize buttons

	KButtonBox * buttonBox = new KButtonBox(boxes, Vertical);
	connect( this, SIGNAL(listUpdated(bool)), buttonBox,
		SLOT(setEnabled(bool)) );

	buttonBox->addButton(i18n("&Modify..."),this, SLOT(modifyBox()));
	buttonBox->addButton(i18n("&New..."),	this, SLOT(newBox()));
	buttonBox->addButton(i18n("&Remove"),	this, SLOT(deleteBox()));
	buttonBox->addButton(i18n("Co&py"),	this, SLOT(cloneBox()));

	// Display

	QButtonGroup *aGroup = new QButtonGroup(display);
	aGroup->hide();

	_horiz	= new QRadioButton(i18n("&Horizontal"), display);
	_vert	= new QRadioButton(i18n("&Vertical"), 	display);
	_dock	= new QRadioButton(i18n("&Docked"),    	display);

	aGroup->insert(_horiz);
	aGroup->insert(_vert);
	aGroup->insert(_dock);

	connect(this, SIGNAL(closeClicked()), SIGNAL(finished()));

	// Init !

	fillListBox();
}

KornOptDlg::~KornOptDlg()
{
}

void KornOptDlg::fillListBox()
{
	_listBox->clear();

	_monitorList.clear();

	QPtrListIterator<KMailDrop> iter = _manager->monitors();

	if( !iter.current() ) {
		// empty list
//		emit listUpdated( false );
		return;
	}

	int index = 0;

	for( ; iter.current(); ++iter ) {
		if (iter.current()->caption().isEmpty()) {
			qWarning( "maildrop of type %s has no caption",
				iter.current()->type().ascii() );
			_listBox->insertItem(i18n("<not specified>"),
					     index );
		}
		else {
			_listBox->insertItem( 
				iter.current()->caption(), index );
		}
		_monitorList.insert( index, iter.current() );
		index++;
	}

	// make sure one item is always selected
	if( _listBox->currentItem() < 0 ) {
		_listBox->setSelected(0, true);
		emit listUpdated( true );
	}
}

void KornOptDlg::setKornLayout( KornSettings::Layout layout )
{
	switch( layout ) {
		case KornSettings::Horizontal:	
			_horiz->setChecked( true );	
			break;
		case KornSettings::Vertical:	
			_vert->setChecked( true );	
			break;
		case KornSettings::Dock:	
			_dock->setChecked( true );	
			break;
		default:
			break;
	}
}

KornSettings::Layout KornOptDlg::kornLayout() const
{
	// there ought to be a nicer
	// way than this..

	if( _horiz->isChecked() ) {
		return KornSettings::Horizontal;
	}
	else if( _vert->isChecked() ) {
		return KornSettings::Vertical;
	}
	else if( _dock->isChecked() ) {
		return KornSettings::Dock;
	}

	qWarning( "kornLayout: no layout option is selected." );
			
	// shouldn't get here
	return KornSettings::Horizontal;
}


void KornOptDlg::newBox()
{
	TypeDialog *dlg = new TypeDialog( _manager->types(), 
			0, "Type", true );
		
	bool selected = dlg->exec();
	QString selectedType=dlg->type();
	if( selected == true && !selectedType.isEmpty()) {
		// type selected

		// new monitor

		KMailDrop *drop = _manager->newMonitor( selectedType);
		drop->setCaption(i18n("New Monitor"));

		// new config dialog

		KDropCfgDialog *dropcfg = dropDialog( drop, false );

		if ( !dropcfg->exec() ) {
		  // delete associated config dialog
		  _dropDlgList.remove( drop );
		  _manager->deleteMonitor( drop );
		}
		//delete dropcfg;
	}

	delete dlg;
}

void KornOptDlg::deleteBox()
{
	KMailDrop *monitor = getMonitor();

	if( monitor == 0 ) {
		return;
	}

	// delete associated config dialog

	_dropDlgList.remove( monitor );

	_manager->deleteMonitor( monitor );
}

void KornOptDlg::cloneBox()
{
	KMailDrop *monitor = getMonitor();

	if( monitor == 0 ) {
		qWarning( "cloneBox: getMonitor() returned null." );
		return;
	}

	// clone monitor

	monitor = _manager->cloneMonitor( monitor );

	if( monitor == 0 ) {
		qWarning( "cloneBox: cloneMonitor() returned null." );
		return;
	}
	// display monitor

	KDropCfgDialog *dropcfg = dropDialog( monitor, false );

	dropcfg->show();
}

KMailDrop *KornOptDlg::getMonitor() const
{
	// get monitor

	KMailDrop *monitor = _monitorList.find( _listBox->currentItem() );

	if( monitor == 0 ) {
		qWarning( "KornOptDlg::modifyBox: no monitor found for"
				" listbox item '%d'.", 
				_listBox->currentItem() );
	}

	return monitor;
}

void KornOptDlg::modifyBox()
{

	// display config dialog.

	KMailDrop *monitor = getMonitor();

	if( monitor == 0 ) {
		return;
	}

	KDropCfgDialog *dlg = dropDialog( monitor );

	dlg->show();

	return;
}

void KornOptDlg::dlgClosed( KDropCfgDialog *dlg )
{
	assert( dlg );

	QPtrDictIterator<KDropCfgDialog> it(_dropDlgList);

  for (; it.current(); ++it) {

    if (it.current() == dlg) {
      // found! remove it
      _dropDlgList.take(it.currentKey());
      return;
    }
  }
  qWarning( "dlgClosed: Disassociated dialog %s not "
      "found in dialog list.",
      dlg->caption().ascii() );
}

KDropCfgDialog *KornOptDlg::dropDialog( KMailDrop *drop, 
	bool checkExists )
{
	assert( drop );
	KDropCfgDialog *dlg;

	if( checkExists ) {
		// exists check
		dlg  = _dropDlgList.find( drop );	

		if( dlg != 0 ) {
			return dlg;
		}
	}

	dlg =  new KDropCfgDialog( drop->caption() );
	assert( dlg != 0 );

	drop->addConfigPage( dlg );
	_dlgList.insert( drop, dlg );

	connect( dlg, SIGNAL(disassociate(KDropCfgDialog*)),
			this, SLOT(dlgClosed(KDropCfgDialog*)) );

	return dlg;
}

void KornOptDlg::updateList()
{
	fillListBox();
}

void KornOptDlg::enableButtons()
{
	emit listUpdated(true);
}

/* vim: set noexpandtab tabstop=8 softtabstop=0: */
#include "optdlg.moc"
