#include "subjectsdlg.h"
#include "maildrop.h"
#include <qapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qdatetime.h>
#include "mailsubject.h"
#include <klocale.h>
#include <qprogressdialog.h>
#include <kmessagebox.h>
#include "maildlg.h"
#include "progress_dialog.h"


KornSubjectsDlg::SubjectListViewItem::SubjectListViewItem( QListView *parent, KornMailSubject * item)
	// set the column strings except column 2 (date)
	: KListViewItem(parent, item->getSender(), item->getSubject(), "", KGlobal::locale()->formatNumber(item->getSize(), 0))
	, _mailSubject(item)
{
	// convert the date according to the user settings and show it in column 2
	QDateTime date;
	date.setTime_t(_mailSubject->getDate());
	setText(2, KGlobal::locale()->formatDateTime(date, true, true));
}

int KornSubjectsDlg::SubjectListViewItem::compare( QListViewItem* item, int column, bool ascending ) const
{
	if ( column == 2 )
	{
		// if column 2 was clicked, compare the dates.
		QDateTime d, e;
		d.setTime_t(_mailSubject->getDate());
		e.setTime_t(((SubjectListViewItem *)item)->_mailSubject->getDate());
		return e.secsTo( d );
	}
	else if ( column == 3 )
	{
		// if column 3 was clicked, compare the sizes.
		int i1 = _mailSubject->getSize();
		int i2 = ((SubjectListViewItem *)item)->_mailSubject->getSize();
		return i1 - i2;
	}
	else
	{
		// otherwise call default handling (i.e. string compare)
		return KListViewItem::compare( item, column, ascending );
	}
}

KornSubjectsDlg::KornSubjectsDlg( QWidget *parent )
   : KDialogBase( parent, "urldialog", true, "test", Close, Close, true), _mailDrop( new QPtrList< KMailDrop > ), 
 	_subjects(0), _delete(0), mailDlg(0), _canDeleteMaildrop( true )
{
	_loadSubjectsCanceled = false;
	setModal( true );

	// The dialog contains a list view and several buttons.
	// Two box layouts hol dthem.
	QWidget * page = new QWidget( this );
	setMainWidget(page);
	invertSelButton = new KPushButton(i18n("&Invert Selection"), page);
	clearSelButton = new KPushButton(i18n("&Remove Selection"), page);
	deleteButton = new KPushButton(i18n("&Delete"), page);
	showButton = new KPushButton(i18n("&Show"), page);
	deleteButton->setEnabled(false);
	showButton->setEnabled(false);
	QVBoxLayout * topLayout = new QVBoxLayout( page, 0, spacingHint() );
	QHBoxLayout * buttons = new QHBoxLayout();
	_list = new KListView(page);
	topLayout->addWidget(_list, 10);
	topLayout->addLayout(buttons, 0);
	buttons->addWidget(invertSelButton, 0);
	buttons->addWidget(clearSelButton, 0);
	buttons->addWidget(deleteButton, 0);
	buttons->addWidget(showButton, 0);
	buttons->addStretch(10);

	// feed the list view with its colums
	_list->setSelectionMode(QListView::Multi);
	_list->addColumn(i18n("From"));
	_list->addColumn(i18n("Subject"));
	_list->addColumn(i18n("Date"));
	_list->addColumn(i18n("Size (Bytes)"));

	// column 3 contains a number (change alignment)
	_list->setColumnAlignment(3, Qt::AlignRight);
	_list->setItemMargin(3);

	// connect the selection changed and double click events of the list view
	connect(_list, SIGNAL(selectionChanged()), this, SLOT(listSelectionChanged()));
	connect(_list, SIGNAL(executed(QListViewItem *)), this, SLOT(doubleClicked(QListViewItem *)));

	// connect the buttons
	connect(invertSelButton, SIGNAL(clicked()), this, SLOT(invertSelection()));
	connect(clearSelButton, SIGNAL(clicked()), this, SLOT(removeSelection()));
	connect(showButton, SIGNAL(clicked()), this, SLOT(showMessage()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteMessage()));
	setInitialSize(QSize(QApplication::desktop()->width(), QApplication::desktop()->height()));
}

void KornSubjectsDlg::clear()
{
	_mailDrop->clear();
}

void KornSubjectsDlg::addMailBox(KMailDrop* mailDrop)
{
	_mailDrop->append( mailDrop );
}
	
void KornSubjectsDlg::loadMessages()
{
	reloadSubjects();
	exec();
}

void KornSubjectsDlg::listSelectionChanged()
{
	if (!_mailDrop)
		return;
	int selected = _list->selectedItems().count();

	// eneable the show button if one is selected
	showButton->setEnabled(selected == 1);

	// eneable the delete button if one or more items are selected
	//TODO//deleteButton->setEnabled((selected > 0) && _mailDrop->canDeleteMails());
}

void KornSubjectsDlg::doubleClicked(QListViewItem * item)
{
	// show the message
	showMessage(item);
}

KornSubjectsDlg::~KornSubjectsDlg()
{
	if (_subjects)
		delete _subjects;
	_subjects = 0;
}

void KornSubjectsDlg::loadSubjectsCanceled()
{
	_loadSubjectsCanceled = true;
}

void KornSubjectsDlg::invertSelection()
{
	_list->invertSelection();
}

void KornSubjectsDlg::removeSelection()
{
	_list->clearSelection();
}

void KornSubjectsDlg::showMessage()
{
	if (!_mailDrop)
		return;
	
	// get selcted item
	QPtrList<QListViewItem> messages = _list->selectedItems();
	QListViewItem * item = messages.first();
	
	// and show it
	showMessage(item);
}

void KornSubjectsDlg::showMessage(QListViewItem * item )
{
	if (!item)
		return;

	// Create mail dialog if it has not been created so far.
	if (!mailDlg)
		mailDlg = new KornMailDlg (this);

	// Feed the mail dailog with data and show it (modal dialog)
	mailDlg->setMailSubject( ( ( KornSubjectsDlg::SubjectListViewItem *)item )->getMailSubject() );
	mailDlg->exec();
}

void KornSubjectsDlg::showSubjectsDlg( const QString& name )
{
	setCaption( i18n("Mails in Box: %1").arg( name ) );

	// load the subjects
	reloadSubjects();
	// if the load process was not cancled: show the dialog
	if( this->isVisible() )
		exec();
}

void KornSubjectsDlg::closeDialog( )
{
	disconnect( this, SIGNAL( finished() ), this, SLOT( closeDialog() ) );	
}

//----------------------------
// ALL FUNCTIONS WITH HAVE SOMETHING TO DO WITH FETCHING SUBJECTS
//----------------------------

//The public function
void KornSubjectsDlg::reloadSubjects()
{
	if( _subjects )
		return; //Already fetching

	makeSubjectsStruct();
		
	_subjects->progress->setNumberOfBoxes( _mailDrop->count() );
	_subjects->progress->setProgressOfBoxes( 0 );
	_subjects->progress->setNumberOfSteps( 1 );
	_subjects->progress->setProgress( 0 );
	
	_subjects->it->toFirst();
	
	if( !_subjects->it->current() )
		return; //No maildrops available.
	
	_subjects->progress->show();
	
	kdDebug() << "KornSubjectsDlg::reloadSubjects(): " << _subjects->progress->isVisible() << endl;
	
	prepareStep1Subjects( _subjects->it->current() );
}

//Private help-functions
void KornSubjectsDlg::prepareStep1Subjects( KMailDrop *drop )
{
	_subjects->progress->setText( i18n( "Rechecking box..." ) );
	_subjects->progress->setNumberOfSteps( 1 );
	_subjects->progress->setProgress( 0 );
	_subjects->atRechecking = true;
	
	connect( drop, SIGNAL( rechecked() ), this, SLOT( slotReloadRechecked() ) );
	drop->recheck();
}

void KornSubjectsDlg::removeStep1Subjects( KMailDrop *drop )
{
	disconnect( drop, SIGNAL( rechecked() ), this, SLOT( slotReloadRechecked() ) );
}

void KornSubjectsDlg::prepareStep2Subjects( KMailDrop *drop )
{
	_subjects->progress->setText( i18n( "Fetching messages..." ) );
	_subjects->atRechecking = false;
	
	connect( drop, SIGNAL( readSubject( KornMailSubject* ) ), this, SLOT( subjectAvailable( KornMailSubject* ) ) );
	connect( drop, SIGNAL( readSubjectsReady( bool ) ), this, SLOT( subjectsReady( bool ) ) );
	connect( drop, SIGNAL( readSubjectsTotalSteps( int ) ), _subjects->progress, SLOT( setNumberOfSteps( int ) ) );
	connect( drop, SIGNAL( readSubjectsProgress( int ) ), _subjects->progress, SLOT( setProgress( int ) ) );
	connect( _subjects->progress, SIGNAL( cancelPressed() ), drop, SLOT( readSubjectsCanceled() ) );
	
	_subjects->it->current()->readSubjects( 0 );
}

void KornSubjectsDlg::removeStep2Subjects( KMailDrop *drop )
{
	disconnect( drop, SIGNAL( readSubject( KornMailSubject* ) ), this, SLOT( subjectAvailable( KornMailSubject* ) ) );
	disconnect( drop, SIGNAL( readSubjectsReady( bool ) ), this, SLOT( subjectsReady( bool ) ) );
	disconnect( drop, SIGNAL( readSubjectsTotalSteps( int ) ), _subjects->progress, SLOT( setNumberOfSteps( int ) ) );
	disconnect( drop, SIGNAL( readSubjectsProgress( int ) ), _subjects->progress, SLOT( setProgress( int ) ) );
	disconnect( _subjects->progress, SIGNAL( cancelPressed() ), drop, SLOT( readSubjectsCanceled() ) );
}

bool KornSubjectsDlg::makeSubjectsStruct()
{
	if( _subjects ) //Subjects are already being checked
		return false;
	
	_subjects = new SubjectsData;
	_subjects->it = new QPtrListIterator< KMailDrop >( *_mailDrop );
	_subjects->subjects = new QValueVector< KornMailSubject >;
	_subjects->progress = new DoubleProgressDialog( this, "progress" );
	_subjects->atRechecking = true;
	
	connect( _subjects->progress, SIGNAL( cancelPressed() ), this, SLOT( slotSubjectsCanceled() ) );
	
	return true;
}

void KornSubjectsDlg::deleteSubjectsStruct()
{
	disconnect( _subjects->progress, SIGNAL( cancelPressed() ), this, SLOT( slotSubjectsCanceled() ) );
	
	delete _subjects->progress;
	delete _subjects->subjects;
	delete _subjects->it;
	delete _subjects; _subjects = 0;
}

//Slots
void KornSubjectsDlg::slotReloadRechecked()
{
	_subjects->progress->setText( i18n( "Downloading subjects..." ) ); //Progress message when fetching messages
	
	removeStep1Subjects( _subjects->it->current() );
	_subjects->subjects->reserve( _subjects->it->current()->count() ); //enlarge QValueVector to speed adding up.
	prepareStep2Subjects( _subjects->it->current() );
}

void KornSubjectsDlg::slotSubjectsCanceled()
{
	if( !_subjects )
		return; //Nothing to do
	
	if( _subjects->atRechecking )
		removeStep1Subjects( _subjects->it->current() );
	else
		removeStep2Subjects( _subjects->it->current() );
	
	deleteSubjectsStruct();
}

void KornSubjectsDlg::subjectAvailable( KornMailSubject * subject )
{
	if( _subjects )
		_subjects->subjects->push_back( *subject );
	
	delete subject;
}

void KornSubjectsDlg::subjectsReady( bool success )
{
	static int progress;
	
	if( !_subjects )
		return;
	
	if( _subjects->it->atFirst() )
		progress = 0;
	
	removeStep2Subjects( _subjects->it->current() );
	
	//Goto next drop
	++(*_subjects->it);
	++progress;
	
	_subjects->progress->setProgressOfBoxes( progress );
	
	if( _subjects->it->current() )
	{
		prepareStep1Subjects( _subjects->it->current() );
	} else {
		//All subjects downloaded
		for( QValueVector<KornMailSubject>::iterator it = _subjects->subjects->begin(); it != _subjects->subjects->end();
				   ++it )
		{ //Draw entry's
			new SubjectListViewItem(_list, &(*it));
		}
		
		if( _subjects->subjects->capacity() != _subjects->subjects->count() )
			_subjects->subjects->resize( _subjects->subjects->count() );
	
		deleteSubjectsStruct();
		//If windows isn't visible already, shows it.
		if( !isVisible() && success )
			show();
	}
}

//---------------------------------
//Here comes all functions with have to do something with deleting messages
//---------------------------------


//Main function
void KornSubjectsDlg::deleteMessage()
{
	if ( !_delete )
		return; //A delete action is already pending
	
	makeDeleteStruct();
	
	fillDeleteMessageList();
	
	if ( _delete->messages->count() == 0 )
	{
		deleteDeleteStruct();
		return; //No messages to delete
	} else {
		_delete->totalNumberOfMessages = _delete->messages->count();
	}
	
	QString confirmation = i18n(	"Do you really want to delete %n message?",
					"Do you really want to delete %n messages?", _delete->messages->count() );
	
	if( KMessageBox::questionYesNo( this, confirmation, i18n( "Confirmation" ) ) != KMessageBox::Yes )
	{
		deleteDeleteStruct();
		return; //Not excepted
	}
	
	_delete->progress->setLabelText( i18n( "Deleting mail; please wait...." ) );
	_delete->progress->setTotalSteps( _delete->totalNumberOfMessages );
	_delete->progress->setProgress( 0 );
	_delete->progress->show();

	deleteNextMessage();
}

//Private help functions
void KornSubjectsDlg::makeDeleteStruct()
{
	_delete = new DeleteData;
	_delete->messages = new QPtrList< KornMailSubject >;
	_delete->ids = new QPtrList< const KornMailId >;
	_delete->progress = new QProgressDialog( this, "progress" );
	_delete->totalNumberOfMessages = 0;
	
	connect( _delete->progress, SIGNAL( canceled() ), this, SLOT( slotDeleteCanceled() ) );
}

void KornSubjectsDlg::deleteDeleteStruct()
{
	disconnect( _delete->progress, SIGNAL( canceled() ), this, SLOT( slotDeleteCanceled() ) );
	
	delete _delete->messages;
	delete _delete->ids;
	delete _delete->progress;
	delete _delete; _delete = 0;
}

void KornSubjectsDlg::fillDeleteMessageList()
{
	QListViewItem *current;
	for( current = _list->selectedItems().first(); current; current = _list->selectedItems().next() )
		_delete->messages->append( ( ( KornSubjectsDlg::SubjectListViewItem * ) current )->getMailSubject() );
}

void KornSubjectsDlg::fillDeleteIdList( KMailDrop *drop )
{
	_delete->ids->clear();
	KornMailSubject *current;
	for( current = _delete->messages->first(); current; current = _delete->messages->next() )
		if( current->getMailDrop() == drop )
			_delete->ids->append( current->getId() );
}

void KornSubjectsDlg::deleteNextMessage()
{
	if( _delete->messages->count() == 0 ) //No more messages to delete
	{
		deleteDeleteStruct();
		reloadSubjects(); //Reload all subjects again
		return;
	}
	
	_delete->ids = new QPtrList< const KornMailId >;
	_delete->drop = _delete->messages->getFirst()->getMailDrop();
	
	fillDeleteIdList( _delete->drop );
	
	// Connect the progress bar signals of the mail box
	connect( _delete->drop, SIGNAL( deleteMailsTotalSteps( int ) ), _delete->progress, SLOT( setTotalSteps( int ) ) );
	connect( _delete->drop, SIGNAL( deleteMailsProgress( int ) ), _delete->progress, SLOT( setProgress( int ) ) );
	connect( _delete->drop, SIGNAL( deleteMailsReady( bool ) ), this, SLOT( deleteMailsReady( bool ) ) );

	// connect the cancel button of the progress bar
	connect( _delete->progress, SIGNAL( canceled() ), _delete->drop, SLOT( deleteMailsCanceled() ) );

	// delete the mails
	_delete->drop->deleteMails( _delete->ids, 0 );
}

void KornSubjectsDlg::deleteMailsReady( bool /*success*/ )
{
	if( !_delete )
		return;
	
	disconnect( _delete->drop, SIGNAL( deleteMailsTotalSteps( int ) ), _delete->progress, SLOT( setTotalSteps( int ) ) );
	disconnect( _delete->drop, SIGNAL( deleteMailsProgress( int ) ), _delete->progress, SLOT( setProgress( int ) ) );
	disconnect( _delete->drop, SIGNAL( deleteMailsReady( bool ) ), this, SLOT( deleteMailsReady( bool ) ) );

	// disconnect the cancel button of the progress bar
	disconnect( _delete->progress, SIGNAL( canceled() ), _delete->drop, SLOT( deleteMailsCanceled() ) );
	
	deleteNextMessage();
}

void KornSubjectsDlg::slotDeleteCanceled()
{
	deleteDeleteStruct();
	reloadSubjects();
}

#include "subjectsdlg.moc"
