#include "maildlg.h"
#include <tqlayout.h>
#include<kdebug.h>
#include<klocale.h>
#include<tqapplication.h>
#include "mailsubject.h"
#include <tqprogressdialog.h>
#include "maildrop.h"

KornMailDlg::KornMailDlg( TQWidget *parent )
   : KDialogBase( parent, "maildialog", true, i18n("Mail Details"), User1|Close, Close, true, KGuiItem(i18n("&Full Message"))),
   _progress( 0 )
{
	TQWidget * page = new TQWidget( this );
	setMainWidget(page);
	TQVBoxLayout * topLayout = new TQVBoxLayout( page, 0, spacingHint() );
	_editCtrl = new KEdit(page);
	topLayout->addWidget(_editCtrl, 10);
	_editCtrl->setReadOnly(true);
	connect(this, TQT_SIGNAL(user1Clicked()), this, TQT_SLOT(showFullMessage()));
	setInitialSize(TQSize(TQApplication::desktop()->width()*9/10, TQApplication::desktop()->height()/2));
}

KornMailDlg::~KornMailDlg()
{
}

void KornMailDlg::loadMailCanceled()
{
	_loadMailCanceled = true;
}


void KornMailDlg::showFullMessage()
{
	_loadMailCanceled = false;
	
	// create progress dialog
	_progress = new TQProgressDialog(this, "bla", TRUE);
	_progress->setMinimumDuration(0);
	_progress->setLabelText(i18n("Loading full mail. Please wait..."));

	// this should show it even if the mailbox does not support progress bars
	_progress->setTotalSteps(1000);
	_progress->setProgress(1);
	qApp->processEvents();

	// connect the mailbox with the progress dialog in case it supports progress bars
	connect(_mailDrop, TQT_SIGNAL(readMailTotalSteps(int)), _progress, TQT_SLOT(setTotalSteps(int)));
	connect(_mailDrop, TQT_SIGNAL(readMailProgress(int)), _progress, TQT_SLOT(setProgress(int)));
	qApp->processEvents();

	// connect the mailbox's cancel button
	connect(_progress, TQT_SIGNAL(canceled()), this, TQT_SLOT(loadMailCanceled()));
	
	connect(_mailDrop, TQT_SIGNAL(readMailReady(TQString*)), this, TQT_SLOT(readMailReady(TQString*)));

	// now load the mail fully
	if( _mailDrop->synchrone() )
	{
		TQString mail = _mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);
		readMailReady( &mail );
	}
	else
		_mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);
}

void KornMailDlg::setMailSubject( KornMailSubject * mailSubject )
{
	_mailSubject = mailSubject;
	_mailDrop = mailSubject->getMailDrop();

	// show mail
	_editCtrl->setText(_mailSubject->getHeader());

	// disable "Full Message" button if mail is already loaded fully
	enableButton(User1, !_mailSubject->isHeaderFullMessage() && _mailDrop->canReadMail());
}

void KornMailDlg::readMailReady( TQString* mail )
{
	deleteProgress();

	// if loading was not canceled and did not fail
	if ( mail->length() > 0)
	{
		// store full mail in KornMailSubject instance (so that it has not to be loaded again next time)
		_mailSubject->setHeader(*mail, true);

		// show fully loaded mail
		_editCtrl->setText(*mail);

		// disable "Full Message" button
		enableButton(User1, false);
	}
}

void KornMailDlg::deleteProgress()
{
	_progress->setProgress(_progress->totalSteps());
	_progress->hide();
	
	disconnect( _mailDrop, TQT_SIGNAL(readMailReady(TQString*)), this, TQT_SLOT(readMailReady(TQString*)));
	
	delete _progress;
	_progress = 0;
}

#include "maildlg.moc"
