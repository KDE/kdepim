#ifndef KornMailDlg_h
#define KornMailDlg_h

#include<kdialogbase.h>
#include <keditcl.h>

class KornMailSubject;
class KMailDrop;

class QProgressDialog;
class QString;

/**
 * KornMailDlg shows the header and (if available) the body of a mail. 
 * If the mails body is not available a button allows the user to load it.
 */
class KornMailDlg : public KDialogBase
{
	Q_OBJECT

	/**
	 * Edit control showing the mail (read only)
	 */
	KEdit * _editCtrl;

	/**
	 * The mail to show
	 */
	KornMailSubject * _mailSubject;

	/**
	 * The mailbox which can load the mail fully
	 */
	KMailDrop * _mailDrop;
	
	/**
	 * Flag used during the load process. Set to true if the user clicks the cancel button.
	 */
	bool _loadMailCanceled;
	
	/**
	 * Progress bar
	 */
	QProgressDialog *_progress;
public:
	/**
	 * KornMailDlg Constructor
	 * @param parent parent widget
	 */
	KornMailDlg( QWidget *parent=0 );

	/**
	 * Set the mail details to show. The mails body is transfered to the edit control
	 * and the "Full Message" button is enabled, if the mail body is not available and
	 * if the mailbox can load the mail fully.
	 * @param mailDrop maibox which can load the mesage fully
	 * @param mailSubject mail to show
	 */
	void setMailSubject( KornMailSubject * mailSubject);

	/**
	 * KornMailDlg Destructor
	 */
	virtual ~KornMailDlg();

private:
	void deleteProgress();
private slots:
	/**
	 * Slot triggered if the user presses the "Full Message" button
	 */
	void showFullMessage();

	/**
	 * Slot triggered if the user canceles the message loading process
	 */
	void loadMailCanceled();
	
	void readMailReady( QString* );
};

#endif
