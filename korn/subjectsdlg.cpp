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
   : KDialogBase( parent, "urldialog", true, "test", Close, Close, true), _mailDrop(0), _subjects(0), mailDlg(0)
{
	_loadSubjectsCanceled = false;

	// The dialog contains a list view and several buttons.
	// Two box layouts hol dthem.
	QWidget * page = new QWidget( this );
	setMainWidget(page);
	invertSelButton = new KPushButton("&Invert Selection", page);
	clearSelButton = new KPushButton("&Remove Selection", page);
	deleteButton = new KPushButton("&Delete", page);
	showButton = new KPushButton("&Show", page);
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
	_list->addColumn("From");
	_list->addColumn("Subject");
	_list->addColumn("Date");
	_list->addColumn("Size (Bytes)");

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

void KornSubjectsDlg::listSelectionChanged()
{
	if (!_mailDrop)
		return;
	int selected = _list->selectedItems().count();

	// eneable the show button if one is selected
	showButton->setEnabled(selected == 1);

	// eneable the delete button if one or more items are selected
	deleteButton->setEnabled((selected > 0) && _mailDrop->canDeleteMails());
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
	_subjects = NULL;
}

void KornSubjectsDlg::loadSubjectsCanceled()
{
	_loadSubjectsCanceled = true;
}

void KornSubjectsDlg::deleteMailsCanceled()
{
	_deleteMailsCanceled = true;
}

void KornSubjectsDlg::invertSelection()
{
	_list->invertSelection();
}

void KornSubjectsDlg::removeSelection()
{
	_list->clearSelection();
}

void KornSubjectsDlg::deleteMessage()
{
	if (!_mailDrop)
		return;
	_deleteMailsCanceled = false;
	QPtrList<QListViewItem> messages = _list->selectedItems();
	if (!messages.count())
		return;
	QString confirmation = "Do you really want to delete one message?";
	if (messages.count() > 1)
		confirmation = "Do you really want to delete " + QString::number(messages.count()) + " messages?";
	if (KMessageBox::questionYesNo(this, confirmation, "Confirmation") != KMessageBox::Yes)
		return;

	// Collect ids of teh messages to delete
	QPtrList<const KornMailId> ids;
	for ( QListViewItem * item = messages.first(); item; item = messages.next() )
	{
		ids.append(((KornSubjectsDlg::SubjectListViewItem *)item)->getMailSubject()->getId());
	}
	bool refresh = false; // true: reload subjects after the delete
	{ // this curled brace ensures, that the progress dialog is deleted before reload is called below
		// Create progress dialog
		QProgressDialog progress(this, "bla", TRUE);
		progress.setMinimumDuration(0);
		progress.setLabelText("Deleting Mails. Please wait...");

		// Initially show it
		progress.setTotalSteps(1000);
		progress.setProgress(1);

		// Connect the progress bar signals of the mail box
		connect(_mailDrop, SIGNAL(deleteMailsTotalSteps(int)), &progress, SLOT(setTotalSteps(int)));
		connect(_mailDrop, SIGNAL(deleteMailsProgress(int)), &progress, SLOT(setProgress(int)));
		qApp->processEvents();

		// connect the cancel button of the progress bar
		connect(&progress, SIGNAL(canceled()), this, SLOT(deleteMailsCanceled()));

		// delete the mails
		refresh = _mailDrop->deleteMails(& ids, &_deleteMailsCanceled);

		// remove progress bar
		progress.setProgress(progress.totalSteps());
		progress.hide();
		disconnect(_mailDrop, 0, &progress, 0);
	}

	// delete canceld: reload subjects
	if (_deleteMailsCanceled)
		refresh = true;
	if (refresh)
	{
		// reload the subjects
		if (!reload())
			// reload canceled? close dialog
			close();
	}
	else
	{
// TODO: delete items from list view
		kdError() << "remove list view items not implemented! See void KornSubjectsDlg::deleteMessage()" << endl;
	}
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

void KornSubjectsDlg::showMessage(QListViewItem * item)
{
	if (!item)
		return;

	// Create mail dialog if it has not been created so far.
	if (!mailDlg)
		mailDlg = new KornMailDlg (this);

	// Feed the mail dailog with data and show it (modal dialog)
	mailDlg->setMailSubject(_mailDrop, ((KornSubjectsDlg::SubjectListViewItem *)item)->getMailSubject());
	mailDlg->exec();
}

bool KornSubjectsDlg::reload()
{
	_loadSubjectsCanceled = false;

	// clear list view
	_list->clear();
	showButton->setEnabled(false);
	deleteButton->setEnabled(false);
	{ // this curled brace ensures, that the progress dialog is deleted before the list view is filled
		// Create progress dialog
		QProgressDialog progress(this, "bla", TRUE);
		progress.setMinimumDuration(0);
		progress.setLabelText("Loading subjects. Please wait...");

		// Initially show it
		progress.setTotalSteps(1000);
		progress.setProgress(1);

		// Connect the progress bar signals of the mail box
		connect(_mailDrop, SIGNAL(readSubjectsTotalSteps(int)), &progress, SLOT(setTotalSteps(int)));
		connect(_mailDrop, SIGNAL(readSubjectsProgress(int)), &progress, SLOT(setProgress(int)));
		qApp->processEvents();

		// connect the cancel button of the progress bar
		connect(&progress, SIGNAL(canceled()), this, SLOT(loadSubjectsCanceled()));
		if (_subjects)
			delete _subjects;
		_subjects = 0;

		// load the subjects
		_subjects = _mailDrop->readSubjects(&_loadSubjectsCanceled);

		// remove progress bar
		progress.setProgress(progress.totalSteps());
		qApp->processEvents();
		disconnect(_mailDrop, 0, &progress, 0);
	}

	// show them, if not canceled
	if (!_loadSubjectsCanceled)
	{
		for( QValueVector<KornMailSubject>::iterator it = _subjects->begin(); it != _subjects->end(); ++it )
		{
			new SubjectListViewItem(_list, &(*it));
		}
		return true;
	}
	else
		// load process canceled. Close dialog!
		return false;
}

void KornSubjectsDlg::showSubjectsDlg(KMailDrop *mailDrop)
{
	// store the mail box during the lifetime of the dialog
	_mailDrop = mailDrop;
	setCaption("Mails in Box: " + _mailDrop->caption());

	// load the subjects
	if (reload())
		// if the load process was not cancled: show the dialog
		exec();

	// the dialog has been closed, delete the pointer to the mailbox
	_mailDrop = 0;
}

#include "subjectsdlg.moc"
