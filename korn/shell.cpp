/*
* shell.cpp -- Implementation of class KornShell.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sun May  3 10:30:24 EST 1998
*/
#include "korncfgimpl.h"


#include<assert.h>
#include<qpopupmenu.h>
#include<qlayout.h>
#include<qcursor.h>

#include<kapplication.h>
#include<kconfig.h>
#include<kwin.h>
#include<kaction.h>
#include<kstdaction.h>
#include<kaboutapplication.h>
#include<kbugreport.h>
#include<kdebug.h>
#include<kdialogbase.h>
#include <kcursor.h>
#include "subjectsdlg.h"

#include"utils.h"
#include"shell.h"

#include"dropman.h"

#include"optdlg.h"
#include"dropdlg.h"

KornShell::KornShell( QWidget *parent )
	: QWidget( parent ),
	_configDirty( false ),
	_toWrite( false ),
	_layout( 0 ),
	_currentMailDrop(0),
	_subjectsDlg(0),
	_optDlg( 0 )
{
        _buttons = new QPtrList<KornButton>;
	_buttons->setAutoDelete( false );
        _menu = initMenu();

        _manager = new KDropManager;

	connect( _manager, SIGNAL(monitorCreated()),
			this, SLOT(configDirty()) );
	connect( _manager, SIGNAL(monitorDeleted()),
			this, SLOT(configDirty()) );
	connect( _manager, SIGNAL(configChanged()),
			this, SLOT(configDirty()) );

        _settings= new KornSettings( kapp->config() );

#if defined(test_headerbutton)
	_headbutton = new HeadButton(this);
#endif
}

KornShell::~KornShell()
{
	if ( _toWrite ) {
		// write config if changed
		_manager->writeConfig( *(kapp->config()), fu("Korn"));
		kapp->config()->sync();
	}

	delete _buttons;
	delete _menu;
	delete _manager;
	delete _settings;
	delete _optDlg;
	delete _layout;
#if defined(test_headerbutton)
	delete _headbutton;
#endif
}

bool KornShell::init()
{
	_settings->readConfig();

	bool ok = _manager->readConfig( *(kapp->config()),
			fu("Korn"));

	if( !ok && firstTimeInit() == false ) {
		qWarning( "KornShell: DropManager configuration not valid." );
		return false;
	}

	createButtons( _settings->layout() );

	return true;
}





/* lays out the monitor buttons.
 * On top is a "korn" string,
 * below the buttons in their layout
 */
void KornShell::createButtons( KornSettings::Layout layout )
{
	// create layout manager if buttons are
	// not to be docked

	if( _layout != 0 ) {
	  kdDebug() << "deleting layout" << endl;
		delete _layout;
		_layout = 0;
	}

	_buttons->setAutoDelete( true );
	_buttons->clear();

#if 0
 	if( layout == KornSettings::Dock
 			&& !KWM::isKWMInitialized() ) {
 		// No KWM, go to fallback style
 		_settings->setLayout( KornSettings::Horizontal );
		layout = _settings->layout();
 	}
#endif

	switch ( layout ) {
		case KornSettings::Vertical:
		  //kdDebug() << "setting vertical layout" << endl;
			_layout = new QBoxLayout( this,
					QBoxLayout::TopToBottom, 2 );
			break;

		case KornSettings::Horizontal:
		  //kdDebug() << "setting horizontal layout" << endl;
			_layout = new QBoxLayout( this,
					QBoxLayout::LeftToRight, 2 );
			break;

		default:
		  //kdDebug() << "setting no layout" << endl;
			_layout = 0;
			break;
	}

	// create buttons

	QPtrListIterator<KMailDrop> list = _manager->monitors();
	QWidget *parent = ( _layout != 0 ) ? this : 0;
	//kdDebug() << "set parent " << parent << endl;

#if defined(test_headerbutton)
	if ( _layout ) {
	  _layout->addWidget(_headerbutton);
	  size++;
	}
	else {
	  KWin::setType( _headbutton->winId(), NET::Dock );
	  KWin::setSystemTrayWindowFor( _headbutton->winId(), 0 );
	  QApplication::syncX();
	  kapp->setMainWidget( 0 );
	}
	_headbutton->show();
#endif

	_buttons->setAutoDelete( (_layout == 0) );
	int size = 0;

	for( ; list.current(); ++list ) {
		KornButton *butt = new KornButton( parent, list.current(), this );

// changed: The right mouse click on a button is handled by the button now!
		connect( butt, SIGNAL (rightClick()), butt, SLOT(popupMenu()));
//		connect( butt, SIGNAL (dying(KornButton *)),
//			this, SLOT(disconnectButton(KornButton *)) );

		_buttons->append( butt );

		if ( _layout ) {
			_layout->addWidget( butt );
			size++;
			kapp->setMainWidget( this );
		}
		else {
			KWin::setType( butt->winId(), NET::Dock );
			KWin::setSystemTrayWindowFor( butt->winId(), 0 );
			QApplication::syncX();
			kapp->setMainWidget( 0 );
		}

		butt->show();
	}

	// set size of top level widget

	if ( size ) {
		int vert;
		int horiz;

		if( layout == KornSettings::Vertical ) {
			vert = size * 25;
			horiz = 25;
		}
		else {
			vert = 25;
			horiz = size * 25;
		}
		resize( horiz, vert );
	}

  if (0 == _buttons->count()) {
    resize(30,30);
  }
}

void KornShell::show()
{
	// If docked, there's no real toplevel widget
	if( ( _settings->layout() != KornSettings::Dock ) ) {
		QWidget::show();
	}
}

void KornShell::popup(KornButton *button)
{
	// store the mail box of the calling button. As the popup menu and
	// all windows opend by it lock the KornButton's, the mail box
	// can reside in an instance variable of KornShell and its contents
	// is valid until the next right mouse button click
	_currentMailDrop = button->getMailDrop();

	// enabe "Read Subjects" menu item, if the mail box can read the
	// mail subjects
	_checkMailAction->setEnabled(_currentMailDrop->canReadSubjects());

	// open the popup menu
	popupMenu();
}

void KornShell::popupMenu()
{
	_menu->popup( QCursor::pos() );
}

void KornShell::reCheck()
{
	// _currentMailDrop must point to the mailbox belonging to
	// the button the user clicked on.
	if (!_currentMailDrop)
		return; // this should never happen!
	QApplication::setOverrideCursor( KCursor::waitCursor() );
	if (_currentMailDrop->running())
	{
		// if the mail box is polled: stop the polling and restart it
		// this resets the poll timer, prevents an interference of
		// the polling with the re-check and re-checks (startMonitor())
		// checks for new mails).
		_currentMailDrop->stopMonitor();
		_currentMailDrop->startMonitor(); // reset timer as well!
	}
	else
	{
		// if the mail box is not polled, simply re-check.
		_currentMailDrop->recheck();
	}
	QApplication::restoreOverrideCursor();
}

void KornShell::readSubjects()
{
	// _currentMailDrop must point to the mailbox belonging to
	// the button the user clicked on.
	if (!_currentMailDrop)
		return;

	// create subjects dialog, if it does not exist so far.
	if (!_subjectsDlg)
		_subjectsDlg = new KornSubjectsDlg(this);

	// show dialog
	_subjectsDlg->showSubjectsDlg(_currentMailDrop);
}

void KornShell::optionDlg()
{
	if( _optDlg != 0 ) {
		_optDlg->show();
		return;
	}

	//_optDlg = new KornOptDlg( _manager, 0 );
	_optDlg = new KDialogBase( 0, "Configuration Dialog", false, i18n( "Korn Configuration" ),
					KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply, KDialogBase::Ok, true );
	_optDlg->setMainWidget( new KornCfgImpl( _optDlg, "Configuration widget" ) );

	//_optDlg->setKornLayout( _settings->layout() );

	connect( _optDlg, SIGNAL(finished()), this, SLOT(dlgClosed()) );

	_optDlg->show();
}

void KornShell::dlgClosed()
{
	if( _optDlg == 0 ) {
		qWarning( "KornShell:: dlgClosed() called without a dialog." );
		return;
	}

	bool needsCreate = false;

	// ok, read and update settings

	//if ( _settings->layout() != _optDlg->kornLayout() ) {

//		_settings->setLayout( _optDlg->kornLayout() );
//		_settings->writeConfig();
//
//		needsCreate = true;
//	}

	if ( _configDirty ) {
		needsCreate = true;
		_configDirty = false;
	}

	if ( needsCreate ) {
		// change only if not the same layout
		hide();
		createButtons( _settings->layout() );
		show();
	}

	_optDlg->delayedDestruct();
	_optDlg = 0;
}

void KornShell::help()
{
	kapp->invokeHelp();
}

void KornShell::reportBug()
{
  KBugReport br(this);
  br.exec();
}

void KornShell::about()
{
  KAboutApplication about(this);
  about.exec();
}

QPopupMenu *KornShell::initMenu()
{
  QPopupMenu *menu = new QPopupMenu;
  KActionCollection* actions = new KActionCollection(this);

  KStdAction::preferences(this, SLOT(optionDlg()), actions)->plug(menu);
  menu->insertSeparator();
  (new KAction(i18n("R&echeck"), KShortcut('e'), this, SLOT(reCheck()), actions, "re_check"))->plug(menu);
  _checkMailAction = new KAction(i18n("Read &Subjects"), KShortcut('s'), this, SLOT(readSubjects()), actions, "read_subjects");
  _checkMailAction->plug(menu);
  _checkMailAction->setEnabled(false);
  menu->insertSeparator();
  KStdAction::help(this, SLOT(help()), actions)->plug(menu);
  KStdAction::reportBug(this, SLOT(reportBug()), actions)->plug(menu);
  KStdAction::aboutApp(this, SLOT(about()), actions)->plug(menu);
  menu->insertSeparator();
  KStdAction::quit(qApp, SLOT(quit()), actions)->plug(menu);

  return menu;
}

bool KornShell::firstTimeInit()
{
	// ask user whether a sample config is wanted

  // XXX Do we really need to ask ?

#if 0
	int status = KMessageBox::warningYesNo(0,
		i18n( "You do not appear to have used KOrn before.\n"
		"Would you like a basic configuration created for you?" ),
		i18n("Welcome to KOrn"),
		KStdGuiItem::yes(), i18n( "No, Exit" ));

	if( status != 0 ) {
		return false;
	}
#endif

	// get manager to create default config

	return _manager->createBasicConfig();
}

void KornShell::disconnectButton( KornButton *button )
{
	assert( button != 0 );

	if( _buttons->remove( button )  ) {
		// button was in list

		if( !_buttons->autoDelete() ) {
			delete button;
		}
	}
}

void KornShell::configDirty()
{
	_configDirty = true;
	_toWrite = true;
}

void KornShell::saveSession()
{
	// No Session management saving required... should
	// just restore itself with last config.
}
#include "shell.moc"
