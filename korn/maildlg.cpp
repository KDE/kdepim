#include "maildlg.h"
#include <qlayout.h>
#include<kdebug.h>
#include<qapplication.h>
#include "mailsubject.h"
#include <qprogressdialog.h>
#include "maildrop.h"

KornMailDlg::KornMailDlg( QWidget *parent )
   : KDialogBase( parent, "maildialog", true, "Mail Details", User1|Close, Close, true, KGuiItem("&Full Message"))
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
	QProgressDialog progress(this, "bla", TRUE);
	progress.setMinimumDuration(0);
	progress.setLabelText("Loading full mail. Please wait...");

	// this should show it even if the mailbox does not support progress bars
	progress.setTotalSteps(1000);
	progress.setProgress(1);
	qApp->processEvents();

	// connect the mailbox with the progress dialog in case it supports progress bars
	connect(_mailDrop, SIGNAL(readMailTotalSteps(int)), &progress, SLOT(setTotalSteps(int)));
	connect(_mailDrop, SIGNAL(readMailProgress(int)), &progress, SLOT(setProgress(int)));
	qApp->processEvents();

	// connect the mailbox's cancel button
	connect(&progress, SIGNAL(canceled()), this, SLOT(loadMailCanceled()));

	// now load the mail fully
	QString mail = _mailDrop->readMail(_mailSubject->getId(), &_loadMailCanceled);

	// remove progress dialog
	progress.setProgress(progress.totalSteps());
	progress.hide();
	disconnect(_mailDrop, 0, &progress, 0);

	// if loading was not canceled and did not fail
	if (!_loadMailCanceled && mail.length() > 0)
	{
		// store full mail in KornMailSubject instance (so that it has not to be loaded again next time)
		_mailSubject->setHeader(mail, true);

		// show fully loaded mail
		_editCtrl->setText(mail);

		// disable "Full Message" button
		enableButton(User1, false);
	}
}

void KornMailDlg::setMailSubject(KMailDrop * mailDrop, KornMailSubject * mailSubject)
{
	_mailSubject = mailSubject;
	_mailDrop = mailDrop;

	// show mail
	_editCtrl->setText(_mailSubject->getHeader());

	// disable "Full Message" button if mail is already loaded fully
	enableButton(User1, !_mailSubject->isHeaderFullMessage() && _mailDrop->canReadMail());
}


#include "maildlg.moc"
