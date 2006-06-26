#ifndef KornSubjectsDlg_h
#define KornSubjectsDlg_h

#include <kdialog.h>
#include <k3listview.h>
#include <QVector>
#include <kpushbutton.h>

class KMailDrop;
class KornMailSubject;
class KornMailId;
class K3ListView;
class KornMailDlg;
class QProgressDialog;
class DoubleProgressDialog;

template< class T > class QList;
class QVariant;

/**
 * KornSubjectsDlg loads all mail subjects and shows them in a list control.
 * Buttons allow the user to select several mails, delete them or to show one
 * of the mails.
 */
class KornSubjectsDlg: public KDialog
{
	Q_OBJECT

	/**
	* SubjectListViewItem is a helper class representing one line in the list view.
	* It stores the mail subject the line represents and controls the sorting.
	*/
	class SubjectListViewItem : public K3ListViewItem
	{
		KornMailSubject * _mailSubject;
	public:
		/**
		* SubjectListViewItem Constructor
		* @param parent list view
		* @param item KornMailSubject this item should represent. It is NOT deleted
		* if SubjectListViewItem is deleted.
		*/
		SubjectListViewItem( Q3ListView *parent, KornMailSubject * item);

		/**
		 * SubjectListViewItem Destructor
		 */
		~SubjectListViewItem();

		/**
		* Compare to list view item. Called if the sort header are clicked.
		* @param item item to compare this with
		* @param column column to compare
		* @param ascending search order
		*/
		int compare( Q3ListViewItem* item, int column, bool ascending ) const;

		/**
		* Return the mail subject.
		* @return the mail subject
		*/
		KornMailSubject * getMailSubject() const {return _mailSubject;}
	};

	QList< KMailDrop* >	*_mailDrop;
	struct SubjectsData
	{
		int maildrop_index;
		QVector< KornMailSubject > *subjects;
		DoubleProgressDialog *progress;
		bool atRechecking;
	} *_subjects;

	struct DeleteData
	{
		QList< KornMailSubject* > *messages;
		QList< QVariant > *ids;
		QProgressDialog *progress;
		KMailDrop *drop;
		int totalNumberOfMessages;
	} *_delete;

	K3ListView * _list;
	KPushButton * invertSelButton;
	KPushButton * clearSelButton;
	KPushButton * deleteButton;
	KPushButton * showButton;
	KornMailDlg * mailDlg;

	bool _loadSubjectsCanceled, _deleteMailsCanceled;
	bool _canDeleteMaildrop;

	/**
	 * Load the mails subjects and refresh the list view.
	 * @return false if the load process was cancled (close the dialog!), true otherwise
	 */
	//bool reload();

	/**
	 * Show a message in a separate dialog
	 * @param item message to show
	 */
	void showMessage(Q3ListViewItem * item);
public:
	/**
	 * KornSubjectsDlg Constructor
	 * @param parent parent widget
	 */
	KornSubjectsDlg( QWidget *parent=0 );

	/**
	 * This functions clears all available KMailDrop's.
	 */
	void clear();

	/**
	 * This function adds a maildrop to the list.
	 * @param mailDrop The maildrop which have to be added.
	 */
	void addMailBox(KMailDrop* mailDrop);

	/**
	 * This method loads the messages and shows the dialog.
	 */
	void loadMessages();

	/**
	 * Show the KornSubjectsDlg as a modal dialog.
	 * @param name The name of the box
	 */
	void showSubjectsDlg( const QString& name );

	/**
	 * KornSubjectsDlg Destructor
	 */
	virtual ~KornSubjectsDlg();

private slots:

	/**
	 * called if the cancel button was clicked while loadind the subjects
	 */
	void loadSubjectsCanceled();

	/**
	 * called if the selection of the list view was changed
	 */
	void listSelectionChanged();

	/**
	 * called if the "Invert Selection" button was clicked
	 */
	void invertSelection();

	/**
	 * called if the "Remove Selection" button was clicked
	 */
	void removeSelection();

	/**
	 * called if the "Show" button was clicked
	 */
	void showMessage();

	/**
	 * called if a list view item was double clicked
	 */
	void doubleClicked ( Q3ListViewItem *item );

	void closeDialog();

	//Functions for the subjects
public slots:
	/**
	 * This function reloads the subjects
	 */
	void reloadSubjects();
private:
	void prepareStep1Subjects( KMailDrop* );
	void removeStep1Subjects( KMailDrop* );
	void prepareStep2Subjects( KMailDrop* );
	void removeStep2Subjects( KMailDrop* );
	bool makeSubjectsStruct();
	void deleteSubjectsStruct();
private slots:
	void slotReloadRechecked();
	void slotSubjectsCanceled();
	void subjectAvailable( KornMailSubject* );
	void subjectsReady( bool );

	//Functions neccesairy for delete
	//TODO: public, when fillDeleteMessageList is private?
public slots:
	/**
	 * This function ask to delete a number of messages and delete the message if the answer was yes.
	 * Which files should be deleted can be set in the function fillDeleteMessageList()
	 */
	void deleteMessage();
private:
	void makeDeleteStruct();
	void deleteDeleteStruct();
	void fillDeleteMessageList();
	void fillDeleteIdList( KMailDrop *drop );
	void deleteNextMessage();
private slots:
	void deleteMailsReady( bool );
	void slotDeleteCanceled();

};

#endif
