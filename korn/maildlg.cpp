#include "maildlg.h"
#include <qlayout.h>
#include<kdebug.h>
#include<klocale.h>
#include<qapplication.h>
#include "mailsubject.h"
#include <qprogressdialog.h>
#include "maildrop.h"

KornMailDlg::KornMailDlg( QWidget *parent )
   : KDialogBase( parent, "maildialog", true, i18n("Mail Details"), User1|Close, Close, true, KGuiItem(i18n("&Full Message"))),
   _progress( 0 )
{
	QWidget * page = new QWidget( this );
	setMainWidget(page);
	QVBoxLayout * topLayout = new QVBoxLayout( page, 0, spacingHint() );
	_editCtrl = new KEdit(page);
	topLayout->addWidget(_editCtrl, 10);
	_editCtrl->setReadOnly(true);
	connect(this, SIGNAL(user1Clicked()), this, SLOT(showFullMessage()));
	setInitialSize(QSize(QApplication::desktop()->width()*9/10, QApplication::desktop()->height()/2));
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
	_progress = new QProgressDialog(this, "bla", TRUE);
	_progress->setMinimumDuration(0);
	_progress->setLabelText(i18n("Loading full mail. Please wait..."));

	// this should show it even if the mailbox does not support progress bars
	_progress->setTotalSteps(1000);
	_progress->setProgress(1);
	qApp->processEvents();

	// connect the mailbox with the progress dialog in case it supports progress bars
	connect(_mailDrop, SIGNAL(readMailTotalSteps(int)), _progress, SLOT(setTotalSteps(int)));
	connect(_mailDrop, SIGNAL(readMailProgress(int)), _progress, SLOT(setProgress(int)));
	qApp->processEvents();

	// connect the mailbox's cancel button
	connect(_progress, SIGNAL(canceled()), this, SLOT(loadMailCanceled()));
	
	connect(_mailDrop, SIGNAL(readMailReady(QString*)), this, SLOT(readMailReady(QString*)));

	// now load the mail fully
	if( _mailDrop->synchrone() )
	{
		QString mail = _mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);
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

void KornMailDlg::readMailReady( QString* mail )
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
	
	disconnect( _mailDrop, SIGNAL(readMailReady(QString*)), this, SLOT(readMailReady(QString*)));
	
	delete _progress;
	_progress = 0;
}

#include "maildlg.moc"
